//===--- Path.cpp ---------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#include <IndexStoreDB_Support/Path.h>
#include <IndexStoreDB_LLVMSupport/llvm_ADT_StringMap.h>
#include <IndexStoreDB_LLVMSupport/llvm_Support_Allocator.h>
#include <IndexStoreDB_LLVMSupport/llvm_Support_Path.h>
#include <IndexStoreDB_LLVMSupport/llvm_Support_FileSystem.h>
#include <IndexStoreDB_LLVMSupport/llvm_Support_Mutex.h>
#if defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <limits.h>
#include <stdlib.h>

#if defined(_WIN32)
#define PATH_MAX MAX_PATH
#endif

using namespace IndexStoreDB;

namespace {
class CanonicalPathCacheImpl {
  llvm::StringMap<CanonicalFilePathRef, llvm::BumpPtrAllocator> CanonPaths;
  mutable llvm::sys::Mutex StateMtx;

public:
  CanonicalFilePath getCanonicalPath(StringRef Path,
                                     StringRef WorkingDir = StringRef());
};
}

CanonicalFilePath
CanonicalPathCacheImpl::getCanonicalPath(StringRef Path, StringRef WorkingDir) {
  if (Path.empty())
    return CanonicalFilePath();

  SmallString<256> AbsPath;
  if (llvm::sys::path::is_absolute(Path)) {
    AbsPath = Path;
  } else {
    assert(!WorkingDir.empty() && "passed relative path without working-dir");
    AbsPath = WorkingDir;
    AbsPath += '/';
    AbsPath += Path;
  }

  {
    llvm::sys::ScopedLock L(StateMtx);
    auto It = CanonPaths.find(AbsPath);
    if (It != CanonPaths.end())
      return It->second;
  }

  llvm::SmallString<PATH_MAX> Buffer;
  if (llvm::sys::fs::real_path(AbsPath.c_str(), Buffer, false)) {
    return CanonicalFilePathRef::getAsCanonicalPath(AbsPath);
  }
  StringRef CanonPath = Buffer;

  {
    llvm::sys::ScopedLock L(StateMtx);
    auto Pair = CanonPaths.insert(std::make_pair(AbsPath.str(), CanonicalFilePathRef()));
    auto &It = Pair.first;
    bool WasInserted = Pair.second;
    if (!WasInserted)
      return It->second;

    CanonicalFilePathRef CanonPathRef;
    if (CanonPath == It->first()) {
      CanonPathRef = CanonicalFilePathRef::getAsCanonicalPath(It->first());
    } else {
      auto &Alloc = CanonPaths.getAllocator();
      char *CopyPtr = Alloc.Allocate<char>(CanonPath.size());
      std::uninitialized_copy(CanonPath.begin(), CanonPath.end(), CopyPtr);
      StringRef CopyCanonPath(CopyPtr, CanonPath.size());
      CanonPathRef = CanonicalFilePathRef::getAsCanonicalPath(CopyCanonPath);
    }
    It->second = CanonPathRef;
    return CanonPathRef;
  }
}


CanonicalPathCache::CanonicalPathCache() {
  Impl = new CanonicalPathCacheImpl();
}
CanonicalPathCache::~CanonicalPathCache() {
  delete static_cast<CanonicalPathCacheImpl*>(Impl);
}

CanonicalFilePath
CanonicalPathCache::getCanonicalPath(StringRef Path, StringRef WorkingDir) {
  return static_cast<CanonicalPathCacheImpl*>(Impl)->getCanonicalPath(Path, WorkingDir);
}
