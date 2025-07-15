//===--- ClangSourceBufferImporter.h - Map Clang buffers over ---*- C++ -*-===//
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

#ifndef LANGUAGE_CLANGIMPORTER_CLANGSOURCEBUFFERIMPORTER_H
#define LANGUAGE_CLANGIMPORTER_CLANGSOURCEBUFFERIMPORTER_H

#include "language/Basic/Toolchain.h"
#include "language/Basic/SourceLoc.h"
#include "clang/Basic/SourceLocation.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/SmallVector.h"

namespace toolchain {
class MemoryBuffer;
}

namespace clang {
class SourceManager;
}

namespace language {
class SourceManager;

namespace importer {

/// A helper class used to keep alive the Clang source managers where
/// diagnostics have been reported.
///
/// This is a bit of a hack, but LLVM's source manager (and by extension
/// Codira's) does not support buffers going away, so if we want to report
/// diagnostics in them we have to do it this way.
class ClangSourceBufferImporter {
  // This is not using SmallPtrSet or similar because we need the
  // IntrusiveRefCntPtr to stay a ref-counting pointer.
  SmallVector<toolchain::IntrusiveRefCntPtr<const clang::SourceManager>, 4>
    sourceManagersWithDiagnostics;
  toolchain::DenseMap<const char *, unsigned> mirroredBuffers;
  SourceManager &languageSourceManager;

public:
  explicit ClangSourceBufferImporter(SourceManager &sourceMgr)
    : languageSourceManager(sourceMgr) {}

  /// Returns a Codira source location that points into a Clang buffer.
  ///
  /// This will keep the Clang buffer alive as long as this object.
  SourceLoc resolveSourceLocation(const clang::SourceManager &clangSrcMgr,
                                  clang::SourceLocation clangLoc);
};

} // end namespace importer
} // end namespace language

#endif
