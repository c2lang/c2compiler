//===--- FileManager.h - File System Probing and Caching --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the c2lang::FileManager interface and associated types.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_FILEMANAGER_H
#define LLVM_CLANG_BASIC_FILEMANAGER_H

#include "Clang/FileSystemOptions.h"
#include "Clang/VirtualFileSystem.h"
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <ctime>
#include <memory>
#include <map>
#include <string>

namespace llvm {

class MemoryBuffer;

} // end namespace llvm

namespace c2lang {

class FileSystemStatCache;

/// Cached information about one directory (either on disk or in
/// the virtual file system).
class DirectoryEntry {
  friend class FileManager;

  StringRef Name; // Name of the directory.

public:
  StringRef getName() const { return Name; }
};

/// Cached information about one file (either on disk
/// or in the virtual file system).
///
/// If the 'File' member is valid, then this FileEntry has an open file
/// descriptor for the file.
class FileEntry {
  friend class FileManager;

  StringRef Name;             // Name of the file.
  std::string RealPathName;   // Real path to the file; could be empty.
  off_t Size;                 // File size in bytes.
  time_t ModTime;             // Modification time of file.
  const DirectoryEntry *Dir;  // Directory file lives in.
  unsigned UID;               // A unique (small) ID for the file.
  llvm::sys::fs::UniqueID UniqueID;
  bool IsNamedPipe;
  bool InPCH;
  bool IsValid;               // Is this \c FileEntry initialized and valid?

  /// The open file, if it is owned by the \p FileEntry.
  mutable std::unique_ptr<vfs::File> File;

public:
  FileEntry()
      : UniqueID(0, 0), IsNamedPipe(false), InPCH(false), IsValid(false)
  {}

  FileEntry(const FileEntry &) = delete;
  FileEntry &operator=(const FileEntry &) = delete;

  StringRef getName() const { return Name; }
  StringRef tryGetRealPathName() const { return RealPathName; }
  bool isValid() const { return IsValid; }
  off_t getSize() const { return Size; }
  unsigned getUID() const { return UID; }
  const llvm::sys::fs::UniqueID &getUniqueID() const { return UniqueID; }
  bool isInPCH() const { return InPCH; }
  time_t getModificationTime() const { return ModTime; }

  /// Return the directory the file lives in.
  const DirectoryEntry *getDir() const { return Dir; }

  bool operator<(const FileEntry &RHS) const { return UniqueID < RHS.UniqueID; }

  /// Check whether the file is a named pipe (and thus can't be opened by
  /// the native FileManager methods).
  bool isNamedPipe() const { return IsNamedPipe; }

  void closeFile() const {
    File.reset(); // rely on destructor to close File
  }
};

struct FileData;

/// Implements support for file system lookup, file system caching,
/// and directory search management.
///
/// This also handles more advanced properties, such as uniquing files based
/// on "inode", so that a file with two names (e.g. symlinked) will be treated
/// as a single file.
///
class FileManager : public RefCountedBase<FileManager> {
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  FileSystemOptions FileSystemOpts;

  /// Cache for existing real directories.
  std::map<llvm::sys::fs::UniqueID, DirectoryEntry> UniqueRealDirs;

  /// Cache for existing real files.
  std::map<llvm::sys::fs::UniqueID, FileEntry> UniqueRealFiles;

  /// The virtual directories that we have allocated.
  ///
  /// For each virtual file (e.g. foo/bar/baz.cpp), we add all of its parent
  /// directories (foo/ and foo/bar/) here.
  SmallVector<std::unique_ptr<DirectoryEntry>, 4> VirtualDirectoryEntries;
  /// The virtual files that we have allocated.
  SmallVector<std::unique_ptr<FileEntry>, 4> VirtualFileEntries;

  /// A cache that maps paths to directory entries (either real or
  /// virtual) we have looked up
  ///
  /// The actual Entries for real directories/files are
  /// owned by UniqueRealDirs/UniqueRealFiles above, while the Entries
  /// for virtual directories/files are owned by
  /// VirtualDirectoryEntries/VirtualFileEntries above.
  ///
  llvm::StringMap<DirectoryEntry*, llvm::BumpPtrAllocator> SeenDirEntries;

  /// A cache that maps paths to file entries (either real or
  /// virtual) we have looked up.
  ///
  /// \see SeenDirEntries
  llvm::StringMap<FileEntry*, llvm::BumpPtrAllocator> SeenFileEntries;

  /// The canonical names of directories.
  llvm::DenseMap<const DirectoryEntry *, llvm::StringRef> CanonicalDirNames;

  /// Storage for canonical names that we have computed.
  llvm::BumpPtrAllocator CanonicalNameStorage;

  /// Each FileEntry we create is assigned a unique ID #.
  ///
  unsigned NextFileUID;

  // Statistics.
  unsigned NumDirLookups, NumFileLookups;
  unsigned NumDirCacheMisses, NumFileCacheMisses;

  // Caching.
  std::unique_ptr<FileSystemStatCache> StatCache;

  bool getStatValue(StringRef Path, FileData &Data, bool isFile,
                    std::unique_ptr<vfs::File> *F);

  /// Add all ancestors of the given path (pointing to either a file
  /// or a directory) as virtual directories.
  void addAncestorsAsVirtualDirs(StringRef Path);

public:
  FileManager(const FileSystemOptions &FileSystemOpts);
  ~FileManager();

    /// Lookup, cache, and verify the specified directory (real or
  /// virtual).
  ///
  /// This returns NULL if the directory doesn't exist.
  ///
  /// \param CacheFailure If true and the file does not exist, we'll cache
  /// the failure to find this file.
  const DirectoryEntry *getDirectory(StringRef DirName,
                                     bool CacheFailure = true);

  /// Lookup, cache, and verify the specified file (real or
  /// virtual).
  ///
  /// This returns NULL if the file doesn't exist.
  ///
  /// \param OpenFile if true and the file exists, it will be opened.
  ///
  /// \param CacheFailure If true and the file does not exist, we'll cache
  /// the failure to find this file.
  const FileEntry *getFile(StringRef Filename, bool OpenFile = false,
                           bool CacheFailure = true);


    /// Open the specified file as a MemoryBuffer, returning a new
  /// MemoryBuffer if successful, otherwise returning null.
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBufferForFile(const FileEntry *Entry, bool isVolatile = false,
                   bool ShouldCloseOpenFile = true);

    /// If path is not absolute and FileSystemOptions set the working
  /// directory, the path is modified to be relative to the given
  /// working directory.
  /// \returns true if \c path changed.
  bool FixupRelativePath(SmallVectorImpl<char> &path) const;

    /// Retrieve the canonical name for a given directory.
  ///
  /// This is a very expensive operation, despite its results being cached,
  /// and should only be used when the physical layout of the file system is
  /// required, which is (almost) never.
  StringRef getCanonicalName(const DirectoryEntry *Dir);

  void PrintStats() const;
};

} // end namespace c2lang

#endif // LLVM_CLANG_BASIC_FILEMANAGER_H
