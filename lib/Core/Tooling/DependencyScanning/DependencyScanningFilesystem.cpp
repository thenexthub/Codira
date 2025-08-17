//===- DependencyScanningFilesystem.cpp - clang-scan-deps fs --------------===//
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

#include "language/Core/Tooling/DependencyScanning/DependencyScanningFilesystem.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Threading.h"
#include <optional>

using namespace language::Core;
using namespace tooling;
using namespace dependencies;

toolchain::ErrorOr<DependencyScanningWorkerFilesystem::TentativeEntry>
DependencyScanningWorkerFilesystem::readFile(StringRef Filename) {
  // Load the file and its content from the file system.
  auto MaybeFile = getUnderlyingFS().openFileForRead(Filename);
  if (!MaybeFile)
    return MaybeFile.getError();
  auto File = std::move(*MaybeFile);

  auto MaybeStat = File->status();
  if (!MaybeStat)
    return MaybeStat.getError();
  auto Stat = std::move(*MaybeStat);

  auto MaybeBuffer = File->getBuffer(Stat.getName());
  if (!MaybeBuffer)
    return MaybeBuffer.getError();
  auto Buffer = std::move(*MaybeBuffer);

  // If the file size changed between read and stat, pretend it didn't.
  if (Stat.getSize() != Buffer->getBufferSize())
    Stat = toolchain::vfs::Status::copyWithNewSize(Stat, Buffer->getBufferSize());

  return TentativeEntry(Stat, std::move(Buffer));
}

bool DependencyScanningWorkerFilesystem::ensureDirectiveTokensArePopulated(
    EntryRef Ref) {
  auto &Entry = Ref.Entry;

  if (Entry.isError() || Entry.isDirectory())
    return false;

  CachedFileContents *Contents = Entry.getCachedContents();
  assert(Contents && "contents not initialized");

  // Double-checked locking.
  if (Contents->DepDirectives.load())
    return true;

  std::lock_guard<std::mutex> GuardLock(Contents->ValueLock);

  // Double-checked locking.
  if (Contents->DepDirectives.load())
    return true;

  SmallVector<dependency_directives_scan::Directive, 64> Directives;
  // Scan the file for preprocessor directives that might affect the
  // dependencies.
  if (scanSourceForDependencyDirectives(Contents->Original->getBuffer(),
                                        Contents->DepDirectiveTokens,
                                        Directives)) {
    Contents->DepDirectiveTokens.clear();
    // FIXME: Propagate the diagnostic if desired by the client.
    Contents->DepDirectives.store(new std::optional<DependencyDirectivesTy>());
    return false;
  }

  // This function performed double-checked locking using `DepDirectives`.
  // Assigning it must be the last thing this function does, otherwise other
  // threads may skip the critical section (`DepDirectives != nullptr`), leading
  // to a data race.
  Contents->DepDirectives.store(
      new std::optional<DependencyDirectivesTy>(std::move(Directives)));
  return true;
}

DependencyScanningFilesystemSharedCache::
    DependencyScanningFilesystemSharedCache() {
  // This heuristic was chosen using a empirical testing on a
  // reasonably high core machine (iMacPro 18 cores / 36 threads). The cache
  // sharding gives a performance edge by reducing the lock contention.
  // FIXME: A better heuristic might also consider the OS to account for
  // the different cost of lock contention on different OSes.
  NumShards =
      std::max(2u, toolchain::hardware_concurrency().compute_thread_count() / 4);
  CacheShards = std::make_unique<CacheShard[]>(NumShards);
}

DependencyScanningFilesystemSharedCache::CacheShard &
DependencyScanningFilesystemSharedCache::getShardForFilename(
    StringRef Filename) const {
  assert(toolchain::sys::path::is_absolute_gnu(Filename));
  return CacheShards[toolchain::hash_value(Filename) % NumShards];
}

DependencyScanningFilesystemSharedCache::CacheShard &
DependencyScanningFilesystemSharedCache::getShardForUID(
    toolchain::sys::fs::UniqueID UID) const {
  auto Hash = toolchain::hash_combine(UID.getDevice(), UID.getFile());
  return CacheShards[Hash % NumShards];
}

std::vector<DependencyScanningFilesystemSharedCache::OutOfDateEntry>
DependencyScanningFilesystemSharedCache::getOutOfDateEntries(
    toolchain::vfs::FileSystem &UnderlyingFS) const {
  // Iterate through all shards and look for cached stat errors.
  std::vector<OutOfDateEntry> InvalidDiagInfo;
  for (unsigned i = 0; i < NumShards; i++) {
    const CacheShard &Shard = CacheShards[i];
    std::lock_guard<std::mutex> LockGuard(Shard.CacheLock);
    for (const auto &[Path, CachedPair] : Shard.CacheByFilename) {
      const CachedFileSystemEntry *Entry = CachedPair.first;
      toolchain::ErrorOr<toolchain::vfs::Status> Status = UnderlyingFS.status(Path);
      if (Status) {
        if (Entry->getError()) {
          // This is the case where we have cached the non-existence
          // of the file at Path first, and a file at the path is created
          // later. The cache entry is not invalidated (as we have no good
          // way to do it now), which may lead to missing file build errors.
          InvalidDiagInfo.emplace_back(Path.data());
        } else {
          toolchain::vfs::Status CachedStatus = Entry->getStatus();
          if (Status->getType() == toolchain::sys::fs::file_type::regular_file &&
              Status->getType() == CachedStatus.getType()) {
            // We only check regular files. Directory files sizes could change
            // due to content changes, and reporting directory size changes can
            // lead to false positives.
            // TODO: At the moment, we do not detect symlinks to files whose
            // size may change. We need to decide if we want to detect cached
            // symlink size changes. We can also expand this to detect file
            // type changes.
            uint64_t CachedSize = CachedStatus.getSize();
            uint64_t ActualSize = Status->getSize();
            if (CachedSize != ActualSize) {
              // This is the case where the cached file has a different size
              // from the actual file that comes from the underlying FS.
              InvalidDiagInfo.emplace_back(Path.data(), CachedSize, ActualSize);
            }
          }
        }
      }
    }
  }
  return InvalidDiagInfo;
}

const CachedFileSystemEntry *
DependencyScanningFilesystemSharedCache::CacheShard::findEntryByFilename(
    StringRef Filename) const {
  assert(toolchain::sys::path::is_absolute_gnu(Filename));
  std::lock_guard<std::mutex> LockGuard(CacheLock);
  auto It = CacheByFilename.find(Filename);
  return It == CacheByFilename.end() ? nullptr : It->getValue().first;
}

const CachedFileSystemEntry *
DependencyScanningFilesystemSharedCache::CacheShard::findEntryByUID(
    toolchain::sys::fs::UniqueID UID) const {
  std::lock_guard<std::mutex> LockGuard(CacheLock);
  auto It = EntriesByUID.find(UID);
  return It == EntriesByUID.end() ? nullptr : It->getSecond();
}

const CachedFileSystemEntry &
DependencyScanningFilesystemSharedCache::CacheShard::
    getOrEmplaceEntryForFilename(StringRef Filename,
                                 toolchain::ErrorOr<toolchain::vfs::Status> Stat) {
  std::lock_guard<std::mutex> LockGuard(CacheLock);
  auto [It, Inserted] = CacheByFilename.insert({Filename, {nullptr, nullptr}});
  auto &[CachedEntry, CachedRealPath] = It->getValue();
  if (!CachedEntry) {
    // The entry is not present in the shared cache. Either the cache doesn't
    // know about the file at all, or it only knows about its real path.
    assert((Inserted || CachedRealPath) && "existing file with empty pair");
    CachedEntry =
        new (EntryStorage.Allocate()) CachedFileSystemEntry(std::move(Stat));
  }
  return *CachedEntry;
}

const CachedFileSystemEntry &
DependencyScanningFilesystemSharedCache::CacheShard::getOrEmplaceEntryForUID(
    toolchain::sys::fs::UniqueID UID, toolchain::vfs::Status Stat,
    std::unique_ptr<toolchain::MemoryBuffer> Contents) {
  std::lock_guard<std::mutex> LockGuard(CacheLock);
  auto [It, Inserted] = EntriesByUID.try_emplace(UID);
  auto &CachedEntry = It->getSecond();
  if (Inserted) {
    CachedFileContents *StoredContents = nullptr;
    if (Contents)
      StoredContents = new (ContentsStorage.Allocate())
          CachedFileContents(std::move(Contents));
    CachedEntry = new (EntryStorage.Allocate())
        CachedFileSystemEntry(std::move(Stat), StoredContents);
  }
  return *CachedEntry;
}

const CachedFileSystemEntry &
DependencyScanningFilesystemSharedCache::CacheShard::
    getOrInsertEntryForFilename(StringRef Filename,
                                const CachedFileSystemEntry &Entry) {
  std::lock_guard<std::mutex> LockGuard(CacheLock);
  auto [It, Inserted] = CacheByFilename.insert({Filename, {&Entry, nullptr}});
  auto &[CachedEntry, CachedRealPath] = It->getValue();
  if (!Inserted || !CachedEntry)
    CachedEntry = &Entry;
  return *CachedEntry;
}

const CachedRealPath *
DependencyScanningFilesystemSharedCache::CacheShard::findRealPathByFilename(
    StringRef Filename) const {
  assert(toolchain::sys::path::is_absolute_gnu(Filename));
  std::lock_guard<std::mutex> LockGuard(CacheLock);
  auto It = CacheByFilename.find(Filename);
  return It == CacheByFilename.end() ? nullptr : It->getValue().second;
}

const CachedRealPath &DependencyScanningFilesystemSharedCache::CacheShard::
    getOrEmplaceRealPathForFilename(StringRef Filename,
                                    toolchain::ErrorOr<toolchain::StringRef> RealPath) {
  std::lock_guard<std::mutex> LockGuard(CacheLock);

  const CachedRealPath *&StoredRealPath = CacheByFilename[Filename].second;
  if (!StoredRealPath) {
    auto OwnedRealPath = [&]() -> CachedRealPath {
      if (!RealPath)
        return RealPath.getError();
      return RealPath->str();
    }();

    StoredRealPath = new (RealPathStorage.Allocate())
        CachedRealPath(std::move(OwnedRealPath));
  }

  return *StoredRealPath;
}

bool DependencyScanningWorkerFilesystem::shouldBypass(StringRef Path) const {
  return BypassedPathPrefix && Path.starts_with(*BypassedPathPrefix);
}

DependencyScanningWorkerFilesystem::DependencyScanningWorkerFilesystem(
    DependencyScanningFilesystemSharedCache &SharedCache,
    IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS)
    : toolchain::RTTIExtends<DependencyScanningWorkerFilesystem,
                        toolchain::vfs::ProxyFileSystem>(std::move(FS)),
      SharedCache(SharedCache),
      WorkingDirForCacheLookup(toolchain::errc::invalid_argument) {
  updateWorkingDirForCacheLookup();
}

const CachedFileSystemEntry &
DependencyScanningWorkerFilesystem::getOrEmplaceSharedEntryForUID(
    TentativeEntry TEntry) {
  auto &Shard = SharedCache.getShardForUID(TEntry.Status.getUniqueID());
  return Shard.getOrEmplaceEntryForUID(TEntry.Status.getUniqueID(),
                                       std::move(TEntry.Status),
                                       std::move(TEntry.Contents));
}

const CachedFileSystemEntry *
DependencyScanningWorkerFilesystem::findEntryByFilenameWithWriteThrough(
    StringRef Filename) {
  if (const auto *Entry = LocalCache.findEntryByFilename(Filename))
    return Entry;
  auto &Shard = SharedCache.getShardForFilename(Filename);
  if (const auto *Entry = Shard.findEntryByFilename(Filename))
    return &LocalCache.insertEntryForFilename(Filename, *Entry);
  return nullptr;
}

toolchain::ErrorOr<const CachedFileSystemEntry &>
DependencyScanningWorkerFilesystem::computeAndStoreResult(
    StringRef OriginalFilename, StringRef FilenameForLookup) {
  toolchain::ErrorOr<toolchain::vfs::Status> Stat =
      getUnderlyingFS().status(OriginalFilename);
  if (!Stat) {
    const auto &Entry =
        getOrEmplaceSharedEntryForFilename(FilenameForLookup, Stat.getError());
    return insertLocalEntryForFilename(FilenameForLookup, Entry);
  }

  if (const auto *Entry = findSharedEntryByUID(*Stat))
    return insertLocalEntryForFilename(FilenameForLookup, *Entry);

  auto TEntry =
      Stat->isDirectory() ? TentativeEntry(*Stat) : readFile(OriginalFilename);

  const CachedFileSystemEntry *SharedEntry = [&]() {
    if (TEntry) {
      const auto &UIDEntry = getOrEmplaceSharedEntryForUID(std::move(*TEntry));
      return &getOrInsertSharedEntryForFilename(FilenameForLookup, UIDEntry);
    }
    return &getOrEmplaceSharedEntryForFilename(FilenameForLookup,
                                               TEntry.getError());
  }();

  return insertLocalEntryForFilename(FilenameForLookup, *SharedEntry);
}

toolchain::ErrorOr<EntryRef>
DependencyScanningWorkerFilesystem::getOrCreateFileSystemEntry(
    StringRef OriginalFilename) {
  SmallString<256> PathBuf;
  auto FilenameForLookup = tryGetFilenameForLookup(OriginalFilename, PathBuf);
  if (!FilenameForLookup)
    return FilenameForLookup.getError();

  if (const auto *Entry =
          findEntryByFilenameWithWriteThrough(*FilenameForLookup))
    return EntryRef(OriginalFilename, *Entry).unwrapError();
  auto MaybeEntry = computeAndStoreResult(OriginalFilename, *FilenameForLookup);
  if (!MaybeEntry)
    return MaybeEntry.getError();
  return EntryRef(OriginalFilename, *MaybeEntry).unwrapError();
}

toolchain::ErrorOr<toolchain::vfs::Status>
DependencyScanningWorkerFilesystem::status(const Twine &Path) {
  SmallString<256> OwnedFilename;
  StringRef Filename = Path.toStringRef(OwnedFilename);

  if (shouldBypass(Filename))
    return getUnderlyingFS().status(Path);

  toolchain::ErrorOr<EntryRef> Result = getOrCreateFileSystemEntry(Filename);
  if (!Result)
    return Result.getError();
  return Result->getStatus();
}

bool DependencyScanningWorkerFilesystem::exists(const Twine &Path) {
  // While some VFS overlay filesystems may implement more-efficient
  // mechanisms for `exists` queries, `DependencyScanningWorkerFilesystem`
  // typically wraps `RealFileSystem` which does not specialize `exists`,
  // so it is not likely to benefit from such optimizations. Instead,
  // it is more-valuable to have this query go through the
  // cached-`status` code-path of the `DependencyScanningWorkerFilesystem`.
  toolchain::ErrorOr<toolchain::vfs::Status> Status = status(Path);
  return Status && Status->exists();
}

namespace {

/// The VFS that is used by clang consumes the \c CachedFileSystemEntry using
/// this subclass.
class DepScanFile final : public toolchain::vfs::File {
public:
  DepScanFile(std::unique_ptr<toolchain::MemoryBuffer> Buffer,
              toolchain::vfs::Status Stat)
      : Buffer(std::move(Buffer)), Stat(std::move(Stat)) {}

  static toolchain::ErrorOr<std::unique_ptr<toolchain::vfs::File>> create(EntryRef Entry);

  toolchain::ErrorOr<toolchain::vfs::Status> status() override { return Stat; }

  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>>
  getBuffer(const Twine &Name, int64_t FileSize, bool RequiresNullTerminator,
            bool IsVolatile) override {
    return std::move(Buffer);
  }

  std::error_code close() override { return {}; }

private:
  std::unique_ptr<toolchain::MemoryBuffer> Buffer;
  toolchain::vfs::Status Stat;
};

} // end anonymous namespace

toolchain::ErrorOr<std::unique_ptr<toolchain::vfs::File>>
DepScanFile::create(EntryRef Entry) {
  assert(!Entry.isError() && "error");

  if (Entry.isDirectory())
    return std::make_error_code(std::errc::is_a_directory);

  auto Result = std::make_unique<DepScanFile>(
      toolchain::MemoryBuffer::getMemBuffer(Entry.getContents(),
                                       Entry.getStatus().getName(),
                                       /*RequiresNullTerminator=*/false),
      Entry.getStatus());

  return toolchain::ErrorOr<std::unique_ptr<toolchain::vfs::File>>(
      std::unique_ptr<toolchain::vfs::File>(std::move(Result)));
}

toolchain::ErrorOr<std::unique_ptr<toolchain::vfs::File>>
DependencyScanningWorkerFilesystem::openFileForRead(const Twine &Path) {
  SmallString<256> OwnedFilename;
  StringRef Filename = Path.toStringRef(OwnedFilename);

  if (shouldBypass(Filename))
    return getUnderlyingFS().openFileForRead(Path);

  toolchain::ErrorOr<EntryRef> Result = getOrCreateFileSystemEntry(Filename);
  if (!Result)
    return Result.getError();
  return DepScanFile::create(Result.get());
}

std::error_code
DependencyScanningWorkerFilesystem::getRealPath(const Twine &Path,
                                                SmallVectorImpl<char> &Output) {
  SmallString<256> OwnedFilename;
  StringRef OriginalFilename = Path.toStringRef(OwnedFilename);

  if (shouldBypass(OriginalFilename))
    return getUnderlyingFS().getRealPath(Path, Output);

  SmallString<256> PathBuf;
  auto FilenameForLookup = tryGetFilenameForLookup(OriginalFilename, PathBuf);
  if (!FilenameForLookup)
    return FilenameForLookup.getError();

  auto HandleCachedRealPath =
      [&Output](const CachedRealPath &RealPath) -> std::error_code {
    if (!RealPath)
      return RealPath.getError();
    Output.assign(RealPath->begin(), RealPath->end());
    return {};
  };

  // If we already have the result in local cache, no work required.
  if (const auto *RealPath =
          LocalCache.findRealPathByFilename(*FilenameForLookup))
    return HandleCachedRealPath(*RealPath);

  // If we have the result in the shared cache, cache it locally.
  auto &Shard = SharedCache.getShardForFilename(*FilenameForLookup);
  if (const auto *ShardRealPath =
          Shard.findRealPathByFilename(*FilenameForLookup)) {
    const auto &RealPath = LocalCache.insertRealPathForFilename(
        *FilenameForLookup, *ShardRealPath);
    return HandleCachedRealPath(RealPath);
  }

  // If we don't know the real path, compute it...
  std::error_code EC = getUnderlyingFS().getRealPath(OriginalFilename, Output);
  toolchain::ErrorOr<toolchain::StringRef> ComputedRealPath = EC;
  if (!EC)
    ComputedRealPath = StringRef{Output.data(), Output.size()};

  // ...and try to write it into the shared cache. In case some other thread won
  // this race and already wrote its own result there, just adopt it. Write
  // whatever is in the shared cache into the local one.
  const auto &RealPath = Shard.getOrEmplaceRealPathForFilename(
      *FilenameForLookup, ComputedRealPath);
  return HandleCachedRealPath(
      LocalCache.insertRealPathForFilename(*FilenameForLookup, RealPath));
}

std::error_code DependencyScanningWorkerFilesystem::setCurrentWorkingDirectory(
    const Twine &Path) {
  std::error_code EC = ProxyFileSystem::setCurrentWorkingDirectory(Path);
  updateWorkingDirForCacheLookup();
  return EC;
}

void DependencyScanningWorkerFilesystem::updateWorkingDirForCacheLookup() {
  toolchain::ErrorOr<std::string> CWD =
      getUnderlyingFS().getCurrentWorkingDirectory();
  if (!CWD) {
    WorkingDirForCacheLookup = CWD.getError();
  } else if (!toolchain::sys::path::is_absolute_gnu(*CWD)) {
    WorkingDirForCacheLookup = toolchain::errc::invalid_argument;
  } else {
    WorkingDirForCacheLookup = *CWD;
  }
  assert(!WorkingDirForCacheLookup ||
         toolchain::sys::path::is_absolute_gnu(*WorkingDirForCacheLookup));
}

toolchain::ErrorOr<StringRef>
DependencyScanningWorkerFilesystem::tryGetFilenameForLookup(
    StringRef OriginalFilename, toolchain::SmallVectorImpl<char> &PathBuf) const {
  StringRef FilenameForLookup;
  if (toolchain::sys::path::is_absolute_gnu(OriginalFilename)) {
    FilenameForLookup = OriginalFilename;
  } else if (!WorkingDirForCacheLookup) {
    return WorkingDirForCacheLookup.getError();
  } else {
    StringRef RelFilename = OriginalFilename;
    RelFilename.consume_front("./");
    PathBuf.assign(WorkingDirForCacheLookup->begin(),
                   WorkingDirForCacheLookup->end());
    toolchain::sys::path::append(PathBuf, RelFilename);
    FilenameForLookup = StringRef{PathBuf.begin(), PathBuf.size()};
  }
  assert(toolchain::sys::path::is_absolute_gnu(FilenameForLookup));
  return FilenameForLookup;
}

const char DependencyScanningWorkerFilesystem::ID = 0;
