//===- VirtualFileSystem.cpp - Virtual File System Layer ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the VirtualFileSystem interface.
//
//===----------------------------------------------------------------------===//

#include "Clang/VirtualFileSystem.h"
#include "Clang/LLVM.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/ADT/Twine.h>
#include <llvm/ADT/iterator_range.h>
#include <llvm/Config/llvm-config.h>
#include <llvm/Support/Compiler.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Chrono.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/Errc.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/SMLoc.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/YAMLParser.h>
#include <llvm/Support/raw_ostream.h>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace c2lang;
using namespace vfs;
using namespace llvm;

using llvm::sys::fs::file_status;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;
using llvm::sys::fs::UniqueID;

Status::Status(const file_status &Status)
    : UID(Status.getUniqueID()), MTime(Status.getLastModificationTime()),
      User(Status.getUser()), Group(Status.getGroup()), Size(Status.getSize()),
      Type(Status.type()), Perms(Status.permissions())  {}

Status::Status(StringRef Name, UniqueID UID, sys::TimePoint<> MTime,
               uint32_t User, uint32_t Group, uint64_t Size, file_type Type,
               perms Perms)
    : Name(Name), UID(UID), MTime(MTime), User(User), Group(Group), Size(Size),
      Type(Type), Perms(Perms) {}

Status Status::copyWithNewName(const Status &In, StringRef NewName) {
  return Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.getType(),
                In.getPermissions());
}

Status Status::copyWithNewName(const file_status &In, StringRef NewName) {
  return Status(NewName, In.getUniqueID(), In.getLastModificationTime(),
                In.getUser(), In.getGroup(), In.getSize(), In.type(),
                In.permissions());
}

bool Status::equivalent(const Status &Other) const {
  assert(isStatusKnown() && Other.isStatusKnown());
  return getUniqueID() == Other.getUniqueID();
}

bool Status::isDirectory() const {
  return Type == file_type::directory_file;
}

bool Status::isRegularFile() const {
  return Type == file_type::regular_file;
}

bool Status::isOther() const {
  return exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

bool Status::isSymlink() const {
  return Type == file_type::symlink_file;
}

bool Status::isStatusKnown() const {
  return Type != file_type::status_error;
}

bool Status::exists() const {
  return isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() = default;

FileSystem::~FileSystem() = default;

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(const llvm::Twine &Name, int64_t FileSize,
                             bool RequiresNullTerminator, bool IsVolatile) {
  auto F = openFileForRead(Name);
  if (!F)
    return F.getError();

  return (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}


std::error_code FileSystem::getRealPath(const Twine &Path,
                                        SmallVectorImpl<char> &Output) const {
  return errc::operation_not_permitted;
}

bool FileSystem::exists(const Twine &Path) {
  auto Status = status(Path);
  return Status && Status->exists();
}

#ifndef NDEBUG
static bool isTraversalComponent(StringRef Component) {
  return Component.equals("..") || Component.equals(".");
}

static bool pathHasTraversal(StringRef Path) {
  using namespace llvm::sys;

  for (StringRef Comp : llvm::make_range(path::begin(Path), path::end(Path)))
    if (isTraversalComponent(Comp))
      return true;
  return false;
}
#endif

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

namespace {

/// Wrapper around a raw file descriptor.
class RealFile : public File {
  friend class RealFileSystem;

  int FD;
  Status S;
  std::string RealName;

  RealFile(int FD, StringRef NewName, StringRef NewRealPathName)
      : FD(FD), S(NewName, {}, {}, {}, {}, {},
                  llvm::sys::fs::file_type::status_error, {}),
        RealName(NewRealPathName.str()) {
    assert(FD >= 0 && "Invalid or inactive file descriptor");
  }

public:
  ~RealFile() override;

  ErrorOr<Status> status() override;
  ErrorOr<std::string> getName() override;
  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(const Twine &Name,
                                                   int64_t FileSize,
                                                   bool RequiresNullTerminator,
                                                   bool IsVolatile) override;
  std::error_code close() override;
};

} // namespace

RealFile::~RealFile() { close(); }

ErrorOr<Status> RealFile::status() {
  assert(FD != -1 && "cannot stat closed file");
  if (!S.isStatusKnown()) {
    file_status RealStatus;
    if (std::error_code EC = sys::fs::status(FD, RealStatus))
      return EC;
    S = Status::copyWithNewName(RealStatus, S.getName());
  }
  return S;
}

ErrorOr<std::string> RealFile::getName() {
  return RealName.empty() ? S.getName().str() : RealName;
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(const Twine &Name, int64_t FileSize,
                    bool RequiresNullTerminator, bool IsVolatile) {
  assert(FD != -1 && "cannot get buffer for closed file");
  return MemoryBuffer::getOpenFile(FD, Name, FileSize, RequiresNullTerminator,
                                   IsVolatile);
}

std::error_code RealFile::close() {
  std::error_code EC = sys::Process::SafelyCloseFileDescriptor(FD);
  FD = -1;
  return EC;
}

namespace {

/// The file system according to your operating system.
class RealFileSystem : public FileSystem {
public:
  ErrorOr<Status> status(const Twine &Path) override;
  ErrorOr<std::unique_ptr<File>> openFileForRead(const Twine &Path) override;
  directory_iterator dir_begin(const Twine &Dir, std::error_code &EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override;
  std::error_code setCurrentWorkingDirectory(const Twine &Path) override;
  std::error_code getRealPath(const Twine &Path,
                              SmallVectorImpl<char> &Output) const override;
};

} // namespace

ErrorOr<Status> RealFileSystem::status(const Twine &Path) {
  sys::fs::file_status RealStatus;
  if (std::error_code EC = sys::fs::status(Path, RealStatus))
    return EC;
  return Status::copyWithNewName(RealStatus, Path.str());
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(const Twine &Name) {
  int FD;
  SmallString<256> RealName;
  if (std::error_code EC =
          sys::fs::openFileForRead(Name, FD, sys::fs::OF_None, &RealName))
    return EC;
  return std::unique_ptr<File>(new RealFile(FD, Name.str(), RealName.str()));
}

llvm::ErrorOr<std::string> RealFileSystem::getCurrentWorkingDirectory() const {
  SmallString<256> Dir;
  if (std::error_code EC = llvm::sys::fs::current_path(Dir))
    return EC;
  return Dir.str().str();
}

std::error_code RealFileSystem::setCurrentWorkingDirectory(const Twine &Path) {
  // FIXME: chdir is thread hostile; on the other hand, creating the same
  // behavior as chdir is complex: chdir resolves the path once, thus
  // guaranteeing that all subsequent relative path operations work
  // on the same path the original chdir resulted in. This makes a
  // difference for example on network filesystems, where symlinks might be
  // switched during runtime of the tool. Fixing this depends on having a
  // file system abstraction that allows openat() style interactions.
  return llvm::sys::fs::set_current_path(Path);
}

std::error_code
RealFileSystem::getRealPath(const Twine &Path,
                            SmallVectorImpl<char> &Output) const {
  return llvm::sys::fs::real_path(Path, Output);
}

IntrusiveRefCntPtr<FileSystem> vfs::getRealFileSystem() {
  static IntrusiveRefCntPtr<FileSystem> FS = new RealFileSystem();
  return FS;
}

namespace {

class RealFSDirIter : public c2lang::vfs::detail::DirIterImpl {
  llvm::sys::fs::directory_iterator Iter;

public:
  RealFSDirIter(const Twine &Path, std::error_code &EC) : Iter(Path, EC) {
    if (Iter != llvm::sys::fs::directory_iterator()) {
      llvm::sys::fs::file_status S;
      std::error_code ErrorCode = llvm::sys::fs::status(Iter->path(), S, true);
      CurrentEntry = Status::copyWithNewName(S, Iter->path());
      if (!EC)
        EC = ErrorCode;
    }
  }

  std::error_code increment() override {
    std::error_code EC;
    Iter.increment(EC);
    if (Iter == llvm::sys::fs::directory_iterator()) {
      CurrentEntry = Status();
    } else {
      llvm::sys::fs::file_status S;
      std::error_code ErrorCode = llvm::sys::fs::status(Iter->path(), S, true);
      CurrentEntry = Status::copyWithNewName(S, Iter->path());
      if (!EC)
        EC = ErrorCode;
    }
    return EC;
  }
};

} // namespace

directory_iterator RealFileSystem::dir_begin(const Twine &Dir,
                                             std::error_code &EC) {
  return directory_iterator(std::make_shared<RealFSDirIter>(Dir, EC));
}


c2lang::vfs::detail::DirIterImpl::~DirIterImpl() = default;

namespace {


} // namespace


namespace c2lang {
namespace vfs {



} // namespace vfs
} // namespace c2lang

