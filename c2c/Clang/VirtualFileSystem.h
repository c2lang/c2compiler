//===- VirtualFileSystem.h - Virtual File System Layer ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the virtual file system interface vfs::FileSystem.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_VIRTUALFILESYSTEM_H
#define LLVM_CLANG_BASIC_VIRTUALFILESYSTEM_H

#include "Clang/LLVM.h"
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/None.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Chrono.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/SourceMgr.h>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <memory>
#include <stack>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace llvm {

class MemoryBuffer;

} // namespace llvm

namespace c2lang {
namespace vfs {

/// The result of a \p status operation.
class Status {
  std::string Name;
  llvm::sys::fs::UniqueID UID;
  llvm::sys::TimePoint<> MTime;
  uint32_t User;
  uint32_t Group;
  uint64_t Size;
  llvm::sys::fs::file_type Type = llvm::sys::fs::file_type::status_error;
  llvm::sys::fs::perms Perms;

public:
   // FIXME: remove when files support multiple names
  bool IsVFSMapped = false;

  Status() = default;
  Status(const llvm::sys::fs::file_status &Status);
  Status(StringRef Name, llvm::sys::fs::UniqueID UID,
         llvm::sys::TimePoint<> MTime, uint32_t User, uint32_t Group,
         uint64_t Size, llvm::sys::fs::file_type Type,
         llvm::sys::fs::perms Perms);

  /// Get a copy of a Status with a different name.
  static Status copyWithNewName(const Status &In, StringRef NewName);
  static Status copyWithNewName(const llvm::sys::fs::file_status &In,
                                StringRef NewName);

  /// Returns the name that should be used for this file or directory.
  StringRef getName() const { return Name; }

  /// @name Status interface from llvm::sys::fs
  /// @{
  llvm::sys::fs::file_type getType() const { return Type; }
  llvm::sys::fs::perms getPermissions() const { return Perms; }
  llvm::sys::TimePoint<> getLastModificationTime() const { return MTime; }
  llvm::sys::fs::UniqueID getUniqueID() const { return UID; }
  uint32_t getUser() const { return User; }
  uint32_t getGroup() const { return Group; }
  uint64_t getSize() const { return Size; }
  /// @}
  /// @name Status queries
  /// These are static queries in llvm::sys::fs.
  /// @{
  bool equivalent(const Status &Other) const;
  bool isDirectory() const;
  bool isRegularFile() const;
  bool isOther() const;
  bool isSymlink() const;
  bool isStatusKnown() const;
  bool exists() const;
  /// @}
};

/// Represents an open file.
class File {
public:
  /// Destroy the file after closing it (if open).
  /// Sub-classes should generally call close() inside their destructors.  We
  /// cannot do that from the base class, since close is virtual.
  virtual ~File();

  /// Get the status of the file.
  virtual llvm::ErrorOr<Status> status() = 0;

  /// Get the name of the file
  virtual llvm::ErrorOr<std::string> getName() {
    if (auto Status = status())
      return Status->getName().str();
    else
      return Status.getError();
  }

  /// Get the contents of the file as a \p MemoryBuffer.
  virtual llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(const Twine &Name, int64_t FileSize = -1,
            bool RequiresNullTerminator = true, bool IsVolatile = false) = 0;

  /// Closes the file.
  virtual std::error_code close() = 0;
};

namespace detail {

/// An interface for virtual file systems to provide an iterator over the
/// (non-recursive) contents of a directory.
struct DirIterImpl {
  virtual ~DirIterImpl();

  /// Sets \c CurrentEntry to the next entry in the directory on success,
  /// or returns a system-defined \c error_code.
  virtual std::error_code increment() = 0;

  Status CurrentEntry;
};

} // namespace detail

/// An input iterator over the entries in a virtual path, similar to
/// llvm::sys::fs::directory_iterator.
class directory_iterator {
  std::shared_ptr<detail::DirIterImpl> Impl; // Input iterator semantics on copy

public:
  directory_iterator(std::shared_ptr<detail::DirIterImpl> I)
      : Impl(std::move(I)) {
    assert(Impl.get() != nullptr && "requires non-null implementation");
    if (!Impl->CurrentEntry.isStatusKnown())
      Impl.reset(); // Normalize the end iterator to Impl == nullptr.
  }

  /// Construct an 'end' iterator.
  directory_iterator() = default;

  /// Equivalent to operator++, with an error code.
  directory_iterator &increment(std::error_code &EC) {
    assert(Impl && "attempting to increment past end");
    EC = Impl->increment();
    if (!Impl->CurrentEntry.isStatusKnown())
      Impl.reset(); // Normalize the end iterator to Impl == nullptr.
    return *this;
  }

  const Status &operator*() const { return Impl->CurrentEntry; }
  const Status *operator->() const { return &Impl->CurrentEntry; }

  bool operator==(const directory_iterator &RHS) const {
    if (Impl && RHS.Impl)
      return Impl->CurrentEntry.equivalent(RHS.Impl->CurrentEntry);
    return !Impl && !RHS.Impl;
  }
  bool operator!=(const directory_iterator &RHS) const {
    return !(*this == RHS);
  }
};

class FileSystem;

/// An input iterator over the recursive contents of a virtual path,
/// similar to llvm::sys::fs::recursive_directory_iterator.
class recursive_directory_iterator {
  using IterState =
      std::stack<directory_iterator, std::vector<directory_iterator>>;

  FileSystem *FS;
  std::shared_ptr<IterState> State; // Input iterator semantics on copy.

public:

    /// Construct an 'end' iterator.
  recursive_directory_iterator() = default;

  /// Equivalent to operator++, with an error code.
  recursive_directory_iterator &increment(std::error_code &EC);

  const Status &operator*() const { return *State->top(); }
  const Status *operator->() const { return &*State->top(); }

  bool operator==(const recursive_directory_iterator &Other) const {
    return State == Other.State; // identity
  }
  bool operator!=(const recursive_directory_iterator &RHS) const {
    return !(*this == RHS);
  }

  /// Gets the current level. Starting path is at level 0.
  int level() const {
    assert(!State->empty() && "Cannot get level without any iteration state");
    return State->size()-1;
  }
};

/// The virtual file system interface.
class FileSystem : public llvm::ThreadSafeRefCountedBase<FileSystem> {
public:
  virtual ~FileSystem();

  /// Get the status of the entry at \p Path, if one exists.
  virtual llvm::ErrorOr<Status> status(const Twine &Path) = 0;

  /// Get a \p File object for the file at \p Path, if one exists.
  virtual llvm::ErrorOr<std::unique_ptr<File>>
  openFileForRead(const Twine &Path) = 0;

  /// This is a convenience method that opens a file, gets its content and then
  /// closes the file.
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBufferForFile(const Twine &Name, int64_t FileSize = -1,
                   bool RequiresNullTerminator = true, bool IsVolatile = false);

  /// Get a directory_iterator for \p Dir.
  /// \note The 'end' iterator is directory_iterator().
  virtual directory_iterator dir_begin(const Twine &Dir,
                                       std::error_code &EC) = 0;

  /// Set the working directory. This will affect all following operations on
  /// this file system and may propagate down for nested file systems.
  virtual std::error_code setCurrentWorkingDirectory(const Twine &Path) = 0;

  /// Get the working directory of this file system.
  virtual llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const = 0;

  /// Gets real path of \p Path e.g. collapse all . and .. patterns, resolve
  /// symlinks. For real file system, this uses `llvm::sys::fs::real_path`.
  /// This returns errc::operation_not_permitted if not implemented by subclass.
  virtual std::error_code getRealPath(const Twine &Path,
                                      SmallVectorImpl<char> &Output) const;

  /// Check whether a file exists. Provided for convenience.
  bool exists(const Twine &Path);

};

/// Gets an \p vfs::FileSystem for the 'real' file system, as seen by
/// the operating system.
IntrusiveRefCntPtr<FileSystem> getRealFileSystem();


} // namespace vfs
} // namespace c2lang

#endif // LLVM_CLANG_BASIC_VIRTUALFILESYSTEM_H
