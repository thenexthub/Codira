//===--- SwiftEditorDiagConsumer.h - ----------------------------*- C++ -*-===//
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

#ifndef LLVM_SOURCEKIT_LIB_SWIFTLANG_SWIFTEDITORDIAGCONSUMER_H
#define LLVM_SOURCEKIT_LIB_SWIFTLANG_SWIFTEDITORDIAGCONSUMER_H

#include "SourceKit/Core/LangSupport.h"
#include "language/AST/DiagnosticConsumer.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"

namespace SourceKit {

class EditorDiagConsumer : public swift::DiagnosticConsumer {
  typedef std::vector<DiagnosticEntryInfo> DiagnosticsTy;
  /// Maps from a BufferID to the diagnostics that were emitted inside that
  /// buffer.
  llvm::DenseMap<unsigned, DiagnosticsTy> BufferDiagnostics;
  DiagnosticsTy InvalidLocDiagnostics;

  llvm::StringMap<BufferInfoSharedPtr> BufferInfos;

  int LastDiagBufferID = -1;
  unsigned LastDiagIndex = 0;

  bool haveLastDiag() {
    return LastDiagBufferID >= 0;
  }
  void clearLastDiag() {
    LastDiagBufferID = -1;
  }
  DiagnosticEntryInfo &getLastDiag() {
    assert(haveLastDiag());
    return BufferDiagnostics[LastDiagBufferID][LastDiagIndex];
  }

  bool HadAnyError = false;

  BufferInfoSharedPtr getBufferInfo(StringRef FileName,
                                    std::optional<unsigned> BufferID,
                                    swift::SourceManager &SM);

public:
  /// The diagnostics are returned in source-order.
  ArrayRef<DiagnosticEntryInfo> getDiagnosticsForBuffer(unsigned BufferID) const {
    ArrayRef<DiagnosticEntryInfo> Diags;
    auto DiagFound = BufferDiagnostics.find(BufferID);
    if (DiagFound != BufferDiagnostics.end())
      Diags = DiagFound->second;
    return Diags;
  }

  void getAllDiagnostics(SmallVectorImpl<DiagnosticEntryInfo> &Result);

  bool hadAnyError() const { return HadAnyError; }

  void handleDiagnostic(swift::SourceManager &SM,
                        const swift::DiagnosticInfo &Info) override;
};

} // namespace SourceKit

#endif
