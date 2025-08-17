//=== SourceMgrAdapter.cpp - SourceMgr to SourceManager Adapter -----------===//
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
// This file implements the adapter that maps diagnostics from toolchain::SourceMgr
// to Clang's SourceManager.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/SourceMgrAdapter.h"
#include "language/Core/Basic/Diagnostic.h"

using namespace language::Core;

void SourceMgrAdapter::handleDiag(const toolchain::SMDiagnostic &Diag,
                                  void *Context) {
  static_cast<SourceMgrAdapter *>(Context)->handleDiag(Diag);
}

SourceMgrAdapter::SourceMgrAdapter(SourceManager &SM,
                                   DiagnosticsEngine &Diagnostics,
                                   unsigned ErrorDiagID, unsigned WarningDiagID,
                                   unsigned NoteDiagID,
                                   OptionalFileEntryRef DefaultFile)
    : SrcMgr(SM), Diagnostics(Diagnostics), ErrorDiagID(ErrorDiagID),
      WarningDiagID(WarningDiagID), NoteDiagID(NoteDiagID),
      DefaultFile(DefaultFile) {}

SourceMgrAdapter::~SourceMgrAdapter() {}

SourceLocation SourceMgrAdapter::mapLocation(const toolchain::SourceMgr &LLVMSrcMgr,
                                             toolchain::SMLoc Loc) {
  // Map invalid locations.
  if (!Loc.isValid())
    return SourceLocation();

  // Find the buffer containing the location.
  unsigned BufferID = LLVMSrcMgr.FindBufferContainingLoc(Loc);
  if (!BufferID)
    return SourceLocation();

  // If we haven't seen this buffer before, copy it over.
  auto Buffer = LLVMSrcMgr.getMemoryBuffer(BufferID);
  auto KnownBuffer = FileIDMapping.find(std::make_pair(&LLVMSrcMgr, BufferID));
  if (KnownBuffer == FileIDMapping.end()) {
    FileID FileID;
    if (DefaultFile) {
      // Map to the default file.
      FileID = SrcMgr.getOrCreateFileID(*DefaultFile, SrcMgr::C_User);

      // Only do this once.
      DefaultFile = std::nullopt;
    } else {
      // Make a copy of the memory buffer.
      StringRef bufferName = Buffer->getBufferIdentifier();
      auto bufferCopy = std::unique_ptr<toolchain::MemoryBuffer>(
          toolchain::MemoryBuffer::getMemBufferCopy(Buffer->getBuffer(),
                                               bufferName));

      // Add this memory buffer to the Clang source manager.
      FileID = SrcMgr.createFileID(std::move(bufferCopy));
    }

    // Save the mapping.
    KnownBuffer = FileIDMapping
                      .insert(std::make_pair(
                          std::make_pair(&LLVMSrcMgr, BufferID), FileID))
                      .first;
  }

  // Translate the offset into the file.
  unsigned Offset = Loc.getPointer() - Buffer->getBufferStart();
  return SrcMgr.getLocForStartOfFile(KnownBuffer->second)
      .getLocWithOffset(Offset);
}

SourceRange SourceMgrAdapter::mapRange(const toolchain::SourceMgr &LLVMSrcMgr,
                                       toolchain::SMRange Range) {
  if (!Range.isValid())
    return SourceRange();

  SourceLocation Start = mapLocation(LLVMSrcMgr, Range.Start);
  SourceLocation End = mapLocation(LLVMSrcMgr, Range.End);
  return SourceRange(Start, End);
}

void SourceMgrAdapter::handleDiag(const toolchain::SMDiagnostic &Diag) {
  // Map the location.
  SourceLocation Loc;
  if (auto *LLVMSrcMgr = Diag.getSourceMgr())
    Loc = mapLocation(*LLVMSrcMgr, Diag.getLoc());

  // Extract the message.
  StringRef Message = Diag.getMessage();

  // Map the diagnostic kind.
  unsigned DiagID;
  switch (Diag.getKind()) {
  case toolchain::SourceMgr::DK_Error:
    DiagID = ErrorDiagID;
    break;

  case toolchain::SourceMgr::DK_Warning:
    DiagID = WarningDiagID;
    break;

  case toolchain::SourceMgr::DK_Remark:
    toolchain_unreachable("remarks not implemented");

  case toolchain::SourceMgr::DK_Note:
    DiagID = NoteDiagID;
    break;
  }

  // Report the diagnostic.
  DiagnosticBuilder Builder = Diagnostics.Report(Loc, DiagID) << Message;

  if (auto *LLVMSrcMgr = Diag.getSourceMgr()) {
    // Translate ranges.
    SourceLocation StartOfLine = Loc.getLocWithOffset(-Diag.getColumnNo());
    for (auto Range : Diag.getRanges()) {
      Builder << SourceRange(StartOfLine.getLocWithOffset(Range.first),
                             StartOfLine.getLocWithOffset(Range.second));
    }

    // Translate Fix-Its.
    for (const toolchain::SMFixIt &FixIt : Diag.getFixIts()) {
      CharSourceRange Range(mapRange(*LLVMSrcMgr, FixIt.getRange()), false);
      Builder << FixItHint::CreateReplacement(Range, FixIt.getText());
    }
  }
}
