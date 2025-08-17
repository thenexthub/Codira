//=== SourceMgrAdapter.h - SourceMgr to SourceManager Adapter ---*- C++ -*-===//
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
//
// This file provides an adapter that maps diagnostics from toolchain::SourceMgr
// to Clang's SourceManager.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SOURCEMGRADAPTER_H
#define LANGUAGE_CORE_SOURCEMGRADAPTER_H

#include "language/Core/Basic/SourceManager.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/SourceMgr.h"
#include <string>
#include <utility>

namespace language::Core {

class DiagnosticsEngine;
class FileEntry;

/// An adapter that can be used to translate diagnostics from one or more
/// toolchain::SourceMgr instances to a ,
class SourceMgrAdapter {
  /// Clang source manager.
  SourceManager &SrcMgr;

  /// Clang diagnostics engine.
  DiagnosticsEngine &Diagnostics;

  /// Diagnostic IDs for errors, warnings, and notes.
  unsigned ErrorDiagID, WarningDiagID, NoteDiagID;

  /// The default file to use when mapping buffers.
  OptionalFileEntryRef DefaultFile;

  /// A mapping from (LLVM source manager, buffer ID) pairs to the
  /// corresponding file ID within the Clang source manager.
  toolchain::DenseMap<std::pair<const toolchain::SourceMgr *, unsigned>, FileID>
      FileIDMapping;

  /// Diagnostic handler.
  static void handleDiag(const toolchain::SMDiagnostic &Diag, void *Context);

public:
  /// Create a new \c SourceMgr adaptor that maps to the given source
  /// manager and diagnostics engine.
  SourceMgrAdapter(SourceManager &SM, DiagnosticsEngine &Diagnostics,
                   unsigned ErrorDiagID, unsigned WarningDiagID,
                   unsigned NoteDiagID,
                   OptionalFileEntryRef DefaultFile = std::nullopt);

  ~SourceMgrAdapter();

  /// Map a source location in the given LLVM source manager to its
  /// corresponding location in the Clang source manager.
  SourceLocation mapLocation(const toolchain::SourceMgr &LLVMSrcMgr,
                             toolchain::SMLoc Loc);

  /// Map a source range in the given LLVM source manager to its corresponding
  /// range in the Clang source manager.
  SourceRange mapRange(const toolchain::SourceMgr &LLVMSrcMgr, toolchain::SMRange Range);

  /// Handle the given diagnostic from an LLVM source manager.
  void handleDiag(const toolchain::SMDiagnostic &Diag);

  /// Retrieve the diagnostic handler to use with the underlying SourceMgr.
  toolchain::SourceMgr::DiagHandlerTy getDiagHandler() {
    return &SourceMgrAdapter::handleDiag;
  }

  /// Retrieve the context to use with the diagnostic handler produced by
  /// \c getDiagHandler().
  void *getDiagContext() { return this; }
};

} // end namespace language::Core

#endif
