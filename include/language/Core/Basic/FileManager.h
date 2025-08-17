//===--- FileManager.h - File System Probing and Caching --------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the language::Core::FileManager interface and associated types.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_FILEMANAGER_H
#define LANGUAGE_CORE_BASIC_FILEMANAGER_H

#include "language/Core/Basic/DirectoryEntry.h"
#include "language/Core/Basic/FileEntry.h"
#include "language/Core/Basic/FileSystemOptions.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/ErrorOr.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include <ctime>
#include <map>
#include <memory>
#include <string>

namespace toolchain {

class MemoryBuffer;

} // end namespace toolchain

namespace language::Core {

class FileSystemStatCache;

/// Implements support for file system lookup, file system caching,
/// and directory search management.
///
/// This also handles more advanced properties, such as uniquing files based
/// on "inode", so that a file with two names (e.g. symlinked) will be treated
/// as a single file.
///
class FileManager : public RefCountedBase<FileManager> {
  IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS;
  FileSystemOptions FileSystemOpts;
  toolchain::SpecificBumpPtrAllocator<FileEntry> FilesAlloc;
  toolchain::SpecificBumpPtrAllocator<DirectoryEntry> DirsAlloc;

  /// Cache for existing real directories.
  toolchain::DenseMap<toolchain::sys::fs::UniqueID, DirectoryEntry *> UniqueRealDirs;

  /// Cache for existing real files.
  toolchain::DenseMap<toolchain::sys::fs::UniqueID, FileEntry *> UniqueRealFiles;

  /// The virtual directories that we have allocated.
  ///
  /// For each virtual file (e.g. foo/bar/baz.cpp), we add all of its parent
  /// directories (foo/ and foo/bar/) here.
  SmallVector<DirectoryEntry *, 4> VirtualDirectoryEntries;
  /// The virtual files that we have allocated.
  SmallVector<FileEntry *, 4> VirtualFileEntries;

  /// A set of files that bypass the maps and uniquing.  They can have
  /// conflicting filenames.
  SmallVector<FileEntry *, 0> BypassFileEntries;

  /// A cache that maps paths to directory entries (either real or
  /// virtual) we have looked up, or an error that occurred when we looked up
  /// the directory.
  ///
  /// The actual Entries for real directories/files are
  /// owned by UniqueRealDirs/UniqueRealFiles above, while the Entries
  /// for virtual directories/files are owned by
  /// VirtualDirectoryEntries/VirtualFileEntries above.
  ///
  toolchain::StringMap<toolchain::ErrorOr<DirectoryEntry &>, toolchain::BumpPtrAllocator>
      SeenDirEntries;

  /// A cache that maps paths to file entries (either real or
  /// virtual) we have looked up, or an error that occurred when we looked up
  /// the file.
  ///
  /// \see SeenDirEntries
  toolchain::StringMap<toolchain::ErrorOr<FileEntryRef::MapValue>, toolchain::BumpPtrAllocator>
      SeenFileEntries;

  /// A mirror of SeenFileEntries to give fake answers for getBypassFile().
  ///
  /// Don't bother hooking up a BumpPtrAllocator. This should be rarely used,
  /// and only on error paths.
  std::unique_ptr<toolchain::StringMap<toolchain::ErrorOr<FileEntryRef::MapValue>>>
      SeenBypassFileEntries;

  /// The file entry for stdin, if it has been accessed through the FileManager.
  OptionalFileEntryRef STDIN;

  /// The canonical names of files and directories .
  toolchain::DenseMap<const void *, toolchain::StringRef> CanonicalNames;

  /// Storage for canonical names that we have computed.
  toolchain::BumpPtrAllocator CanonicalNameStorage;

  /// Each FileEntry we create is assigned a unique ID #.
  ///
  unsigned NextFileUID;

  /// Statistics gathered during the lifetime of the FileManager.
  unsigned NumDirLookups = 0;
  unsigned NumFileLookups = 0;
  unsigned NumDirCacheMisses = 0;
  unsigned NumFileCacheMisses = 0;

  // Caching.
  std::unique_ptr<FileSystemStatCache> StatCache;

  std::error_code getStatValue(StringRef Path, toolchain::vfs::Status &Status,
                               bool isFile, std::unique_ptr<toolchain::vfs::File> *F,
                               bool IsText = true);

  /// Add all ancestors of the given path (pointing to either a file
  /// or a directory) as virtual directories.
  void addAncestorsAsVirtualDirs(StringRef Path);

  /// Fills the RealPathName in file entry.
  void fillRealPathName(FileEntry *UFE, toolchain::StringRef FileName);

public:
  /// Construct a file manager, optionally with a custom VFS.
  ///
  /// \param FS if non-null, the VFS to use.  Otherwise uses
  /// toolchain::vfs::getRealFileSystem().
  FileManager(const FileSystemOptions &FileSystemOpts,
              IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS = nullptr);
  ~FileManager();

  /// Installs the provided FileSystemStatCache object within
  /// the FileManager.
  ///
  /// Ownership of this object is transferred to the FileManager.
  ///
  /// \param statCache the new stat cache to install. Ownership of this
  /// object is transferred to the FileManager.
  void setStatCache(std::unique_ptr<FileSystemStatCache> statCache);

  /// Removes the FileSystemStatCache object from the manager.
  void clearStatCache();

  /// Returns the number of unique real file entries cached by the file manager.
  size_t getNumUniqueRealFiles() const { return UniqueRealFiles.size(); }

  /// Lookup, cache, and verify the specified directory (real or
  /// virtual).
  ///
  /// This returns a \c std::error_code if there was an error reading the
  /// directory. On success, returns the reference to the directory entry
  /// together with the exact path that was used to access a file by a
  /// particular call to getDirectoryRef.
  ///
  /// \param CacheFailure If true and the file does not exist, we'll cache
  /// the failure to find this file.
  toolchain::Expected<DirectoryEntryRef> getDirectoryRef(StringRef DirName,
                                                    bool CacheFailure = true);

  /// Get a \c DirectoryEntryRef if it exists, without doing anything on error.
  OptionalDirectoryEntryRef getOptionalDirectoryRef(StringRef DirName,
                                                    bool CacheFailure = true) {
    return toolchain::expectedToOptional(getDirectoryRef(DirName, CacheFailure));
  }

  /// Lookup, cache, and verify the specified file (real or virtual). Return the
  /// reference to the file entry together with the exact path that was used to
  /// access a file by a particular call to getFileRef. If the underlying VFS is
  /// a redirecting VFS that uses external file names, the returned FileEntryRef
  /// will use the external name instead of the filename that was passed to this
  /// method.
  ///
  /// This returns a \c std::error_code if there was an error loading the file,
  /// or a \c FileEntryRef otherwise.
  ///
  /// \param OpenFile if true and the file exists, it will be opened.
  ///
  /// \param CacheFailure If true and the file does not exist, we'll cache
  /// the failure to find this file.
  toolchain::Expected<FileEntryRef> getFileRef(StringRef Filename,
                                          bool OpenFile = false,
                                          bool CacheFailure = true,
                                          bool IsText = true);

  /// Get the FileEntryRef for stdin, returning an error if stdin cannot be
  /// read.
  ///
  /// This reads and caches stdin before returning. Subsequent calls return the
  /// same file entry, and a reference to the cached input is returned by calls
  /// to getBufferForFile.
  toolchain::Expected<FileEntryRef> getSTDIN();

  /// Get a FileEntryRef if it exists, without doing anything on error.
  OptionalFileEntryRef getOptionalFileRef(StringRef Filename,
                                          bool OpenFile = false,
                                          bool CacheFailure = true) {
    return toolchain::expectedToOptional(
        getFileRef(Filename, OpenFile, CacheFailure));
  }

  /// Returns the current file system options
  FileSystemOptions &getFileSystemOpts() { return FileSystemOpts; }
  const FileSystemOptions &getFileSystemOpts() const { return FileSystemOpts; }

  toolchain::vfs::FileSystem &getVirtualFileSystem() const { return *FS; }
  toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem>
  getVirtualFileSystemPtr() const {
    return FS;
  }

  /// Enable or disable tracking of VFS usage. Used to not track full header
  /// search and implicit modulemap lookup.
  void trackVFSUsage(bool Active);

  void setVirtualFileSystem(IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS) {
    this->FS = std::move(FS);
  }

  /// Retrieve a file entry for a "virtual" file that acts as
  /// if there were a file with the given name on disk.
  ///
  /// The file itself is not accessed.
  FileEntryRef getVirtualFileRef(StringRef Filename, off_t Size,
                                 time_t ModificationTime);

  /// Retrieve a FileEntry that bypasses VFE, which is expected to be a virtual
  /// file entry, to access the real file.  The returned FileEntry will have
  /// the same filename as FE but a different identity and its own stat.
  ///
  /// This should be used only for rare error recovery paths because it
  /// bypasses all mapping and uniquing, blindly creating a new FileEntry.
  /// There is no attempt to deduplicate these; if you bypass the same file
  /// twice, you get two new file entries.
  OptionalFileEntryRef getBypassFile(FileEntryRef VFE);

  /// Open the specified file as a MemoryBuffer, returning a new
  /// MemoryBuffer if successful, otherwise returning null.
  /// The IsText parameter controls whether the file should be opened as a text
  /// or binary file, and should be set to false if the file contents should be
  /// treated as binary.
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>>
  getBufferForFile(FileEntryRef Entry, bool isVolatile = false,
                   bool RequiresNullTerminator = true,
                   std::optional<int64_t> MaybeLimit = std::nullopt,
                   bool IsText = true);
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>>
  getBufferForFile(StringRef Filename, bool isVolatile = false,
                   bool RequiresNullTerminator = true,
                   std::optional<int64_t> MaybeLimit = std::nullopt,
                   bool IsText = true) const {
    return getBufferForFileImpl(Filename,
                                /*FileSize=*/MaybeLimit.value_or(-1),
                                isVolatile, RequiresNullTerminator, IsText);
  }

private:
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>>
  getBufferForFileImpl(StringRef Filename, int64_t FileSize, bool isVolatile,
                       bool RequiresNullTerminator, bool IsText) const;

  DirectoryEntry *&getRealDirEntry(const toolchain::vfs::Status &Status);

public:
  /// Get the 'stat' information for the given \p Path.
  ///
  /// If the path is relative, it will be resolved against the WorkingDir of the
  /// FileManager's FileSystemOptions.
  ///
  /// \returns a \c std::error_code describing an error, if there was one
  std::error_code getNoncachedStatValue(StringRef Path,
                                        toolchain::vfs::Status &Result);

  /// If path is not absolute and FileSystemOptions set the working
  /// directory, the path is modified to be relative to the given
  /// working directory.
  /// \returns true if \c path changed.
  bool FixupRelativePath(SmallVectorImpl<char> &path) const;

  /// Makes \c Path absolute taking into account FileSystemOptions and the
  /// working directory option.
  /// \returns true if \c Path changed to absolute.
  bool makeAbsolutePath(SmallVectorImpl<char> &Path) const;

  /// Produce an array mapping from the unique IDs assigned to each
  /// file to the corresponding FileEntryRef.
  void
  GetUniqueIDMapping(SmallVectorImpl<OptionalFileEntryRef> &UIDToFiles) const;

  /// Retrieve the canonical name for a given directory.
  ///
  /// This is a very expensive operation, despite its results being cached,
  /// and should only be used when the physical layout of the file system is
  /// required, which is (almost) never.
  StringRef getCanonicalName(DirectoryEntryRef Dir);

  /// Retrieve the canonical name for a given file.
  ///
  /// This is a very expensive operation, despite its results being cached,
  /// and should only be used when the physical layout of the file system is
  /// required, which is (almost) never.
  StringRef getCanonicalName(FileEntryRef File);

private:
  /// Retrieve the canonical name for a given file or directory.
  ///
  /// The first param is a key in the CanonicalNames array.
  StringRef getCanonicalName(const void *Entry, StringRef Name);

public:
  void PrintStats() const;

  /// Import statistics from a child FileManager and add them to this current
  /// FileManager.
  void AddStats(const FileManager &Other);
};

} // end namespace language::Core

#endif // LANGUAGE_CORE_BASIC_FILEMANAGER_H
