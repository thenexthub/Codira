//===--- Path.h -------------------------------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_SUPPORT_PATH_H
#define INDEXSTOREDB_SUPPORT_PATH_H

#include <IndexStoreDB_Support/LLVM.h>
#include <IndexStoreDB_Support/Visibility.h>
#include <IndexStoreDB_LLVMSupport/llvm_ADT_SmallString.h>
#include <IndexStoreDB_LLVMSupport/llvm_Support_Path.h>

namespace IndexStoreDB {
  class CanonicalFilePathRef;

class CanonicalFilePath {
  std::string Path;

public:
  CanonicalFilePath() = default;
  inline CanonicalFilePath(CanonicalFilePathRef CanonPath);

  const std::string &getPath() const { return Path; }
  bool empty() const { return Path.empty(); }

  friend bool operator==(CanonicalFilePath LHS, CanonicalFilePath RHS) {
    return LHS.Path == RHS.Path;
  }
  friend bool operator!=(CanonicalFilePath LHS, CanonicalFilePath RHS) {
    return !(LHS == RHS);
  }
  friend bool operator<(CanonicalFilePath LHS, CanonicalFilePath RHS) {
    return LHS.Path < RHS.Path;
  }
};

class CanonicalFilePathRef {
  StringRef Path;

public:
  CanonicalFilePathRef() = default;
  CanonicalFilePathRef(const CanonicalFilePath &CanonPath)
    : Path(CanonPath.getPath()) {}

  static CanonicalFilePathRef getAsCanonicalPath(StringRef Path) {
    CanonicalFilePathRef CanonPath;
    CanonPath.Path = Path;
    return CanonPath;
  }

  StringRef getPath() const { return Path; }
  bool empty() const { return Path.empty(); }
  size_t size() const { return Path.size(); }

  friend bool operator==(CanonicalFilePathRef LHS, CanonicalFilePathRef RHS) {
    return LHS.Path == RHS.Path;
  }
  friend bool operator!=(CanonicalFilePathRef LHS, CanonicalFilePathRef RHS) {
    return !(LHS == RHS);
  }

  bool contains(CanonicalFilePathRef other) {
    if (empty() || !other.Path.startswith(Path))
      return false;
    auto rest = other.Path.drop_front(size());
    return !rest.empty() && llvm::sys::path::is_separator(rest.front());
  }
};

inline CanonicalFilePath::CanonicalFilePath(CanonicalFilePathRef CanonPath)
  : Path(CanonPath.getPath()) {}

class INDEXSTOREDB_EXPORT CanonicalPathCache {
  void *Impl;

public:
  CanonicalPathCache();
  ~CanonicalPathCache();

  CanonicalFilePath getCanonicalPath(StringRef Path,
                                     StringRef WorkingDir = StringRef());
};

} // namespace IndexStoreDB

#endif
