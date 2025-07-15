//===--- DiagnosticBridge.cpp - Diagnostic Bridge to language-syntax ---------===//
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
//
//  This file implements the DiagnosticBridge class.
//
//===----------------------------------------------------------------------===//

#include "language/AST/DiagnosticBridge.h"
#include "language/AST/ASTBridging.h"
#include "language/AST/DiagnosticConsumer.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/Basic/SourceManager.h"
#include "language/Bridging/ASTGen.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

#if LANGUAGE_BUILD_LANGUAGE_SYNTAX

/// Enqueue a diagnostic with ASTGen's diagnostic rendering.
static void addQueueDiagnostic(void *queuedDiagnostics,
                               void *perFrontendState,
                               const DiagnosticInfo &info, SourceManager &SM) {
  toolchain::SmallString<256> text;
  {
    toolchain::raw_svector_ostream out(text);
    DiagnosticEngine::formatDiagnosticText(out, info.FormatString,
                                           info.FormatArgs);
  }

  // Map the highlight ranges.
  SmallVector<BridgedCharSourceRange, 2> highlightRanges;
  for (const auto &range : info.Ranges) {
    if (range.isValid())
      highlightRanges.push_back(range);
  }

  StringRef documentationPath = info.CategoryDocumentationURL;

  SmallVector<BridgedFixIt, 2> fixIts;
  for (const auto &fixIt : info.FixIts) {
    fixIts.push_back(BridgedFixIt{ fixIt.getRange(), fixIt.getText() });
  }

  language_ASTGen_addQueuedDiagnostic(
      queuedDiagnostics, perFrontendState,
      text.str(),
      info.Kind, info.Loc,
      info.Category,
      documentationPath,
      highlightRanges.data(), highlightRanges.size(),
      toolchain::ArrayRef<BridgedFixIt>(fixIts));

  // TODO: A better way to do this would be to pass the notes as an
  // argument to `language_ASTGen_addQueuedDiagnostic` but that requires
  // bridging of `Note` structure and new serialization.
  for (auto *childNote : info.ChildDiagnosticInfo) {
    addQueueDiagnostic(queuedDiagnostics, perFrontendState, *childNote, SM);
  }
}

void DiagnosticBridge::emitDiagnosticWithoutLocation(
    const DiagnosticInfo &info, toolchain::raw_ostream &out, bool forceColors) {
  ASSERT(queuedDiagnostics == nullptr);

  // If we didn't have per-frontend state before, create it now.
  if (!perFrontendState) {
    perFrontendState = language_ASTGen_createPerFrontendDiagnosticState();
  }

  toolchain::SmallString<256> text;
  {
    toolchain::raw_svector_ostream out(text);
    DiagnosticEngine::formatDiagnosticText(out, info.FormatString,
                                           info.FormatArgs);
  }

  BridgedStringRef bridgedRenderedString{nullptr, 0};
  language_ASTGen_renderSingleDiagnostic(
      perFrontendState, text.str(), info.Kind, info.Category,
      toolchain::StringRef(info.CategoryDocumentationURL), forceColors ? 1 : 0,
      &bridgedRenderedString);

  auto renderedString = bridgedRenderedString.unbridged();
  if (renderedString.data()) {
    out << "<unknown>:0: ";
    out.write(renderedString.data(), renderedString.size());
    language_ASTGen_freeBridgedString(renderedString);
    out << "\n";
  }
}

void DiagnosticBridge::enqueueDiagnostic(SourceManager &SM,
                                         const DiagnosticInfo &Info,
                                         unsigned innermostBufferID) {
  // If we didn't have per-frontend state before, create it now.
  if (!perFrontendState) {
    perFrontendState = language_ASTGen_createPerFrontendDiagnosticState();
  }

  // If there are no enqueued diagnostics, or we have hit a non-note
  // diagnostic, flush any enqueued diagnostics and start fresh.
  if (!queuedDiagnostics)
    queuedDiagnostics = language_ASTGen_createQueuedDiagnostics();

  queueBuffer(SM, innermostBufferID);
  addQueueDiagnostic(queuedDiagnostics, perFrontendState, Info, SM);
}

void DiagnosticBridge::flush(toolchain::raw_ostream &OS, bool includeTrailingBreak,
                             bool forceColors) {
  if (!queuedDiagnostics)
    return;

  BridgedStringRef bridgedRenderedString{nullptr, 0};
  language_ASTGen_renderQueuedDiagnostics(queuedDiagnostics, /*contextSize=*/2,
                                       forceColors ? 1 : 0,
                                       &bridgedRenderedString);
  auto renderedString = bridgedRenderedString.unbridged();
  if (renderedString.data()) {
    OS.write(renderedString.data(), renderedString.size());
    language_ASTGen_freeBridgedString(renderedString);
  }
  language_ASTGen_destroyQueuedDiagnostics(queuedDiagnostics);
  queuedDiagnostics = nullptr;
  queuedBuffers.clear();

  if (includeTrailingBreak)
    OS << "\n";
}

void DiagnosticBridge::printCategoryFootnotes(toolchain::raw_ostream &os,
                                              bool forceColors) {
  if (!perFrontendState)
    return;

  BridgedStringRef bridgedRenderedString{nullptr, 0};
  language_ASTGen_renderCategoryFootnotes(
      perFrontendState, forceColors ? 1 : 0, &bridgedRenderedString);

  auto renderedString = bridgedRenderedString.unbridged();
  if (auto renderedData = renderedString.data()) {
    os.write(renderedData, renderedString.size());
    language_ASTGen_freeBridgedString(renderedString);
  }
}

void *DiagnosticBridge::getSourceFileSyntax(SourceManager &sourceMgr,
                                            unsigned bufferID,
                                            StringRef displayName) {
  auto known = sourceFileSyntax.find({&sourceMgr, bufferID});
  if (known != sourceFileSyntax.end())
    return known->second;

  auto bufferContents = sourceMgr.getEntireTextForBuffer(bufferID);
  auto sourceFile = language_ASTGen_parseSourceFile(
      bufferContents, StringRef{"module"}, displayName,
      /*declContextPtr=*/nullptr, BridgedGeneratedSourceFileKindNone);

  sourceFileSyntax[{&sourceMgr, bufferID}] = sourceFile;
  return sourceFile;
}

void DiagnosticBridge::queueBuffer(SourceManager &sourceMgr,
                                   unsigned bufferID) {
  QueuedBuffer knownSourceFile = queuedBuffers[bufferID];
  if (knownSourceFile)
    return;

  // Find the parent and position in parent, if there is one.
  int parentID = -1;
  int positionInParent = 0;
  std::string displayName;
  auto generatedSourceInfo = sourceMgr.getGeneratedSourceInfo(bufferID);
  if (generatedSourceInfo) {
    SourceLoc parentLoc = generatedSourceInfo->originalSourceRange.getEnd();
    if (parentLoc.isValid()) {
      parentID = sourceMgr.findBufferContainingLoc(parentLoc);
      positionInParent = sourceMgr.getLocOffsetInBuffer(parentLoc, parentID);

      // Queue the parent buffer.
      queueBuffer(sourceMgr, parentID);
    }

    if (!generatedSourceInfo->macroName.empty()) {
      if (generatedSourceInfo->attachedMacroCustomAttr)
        displayName = "macro expansion @" + generatedSourceInfo->macroName;
      else
        displayName = "macro expansion #" + generatedSourceInfo->macroName;
    }
  }

  if (displayName.empty()) {
    displayName =
        sourceMgr.getDisplayNameForLoc(sourceMgr.getLocForBufferStart(bufferID))
            .str();
  }

  auto sourceFile = getSourceFileSyntax(sourceMgr, bufferID, displayName);
  language_ASTGen_addQueuedSourceFile(queuedDiagnostics, bufferID, sourceFile,
                                   (const uint8_t *)displayName.data(),
                                   displayName.size(), parentID,
                                   positionInParent);
  queuedBuffers[bufferID] = sourceFile;
}

SmallVector<unsigned, 1>
DiagnosticBridge::getSourceBufferStack(SourceManager &sourceMgr,
                                       SourceLoc loc) {
  SmallVector<unsigned, 1> stack;
  while (true) {
    if (loc.isInvalid())
      return stack;

    unsigned bufferID = sourceMgr.findBufferContainingLoc(loc);
    stack.push_back(bufferID);

    auto generatedSourceInfo = sourceMgr.getGeneratedSourceInfo(bufferID);
    if (!generatedSourceInfo)
      return stack;

    loc = generatedSourceInfo->originalSourceRange.getStart();
  }
}

DiagnosticBridge::~DiagnosticBridge() {
  if (perFrontendState) {
    language_ASTGen_destroyPerFrontendDiagnosticState(perFrontendState);
  }

  assert(!queuedDiagnostics && "unflushed diagnostics");
  for (const auto &sourceFileSyntax : sourceFileSyntax) {
    language_ASTGen_destroySourceFile(sourceFileSyntax.second);
  }
}

#endif // LANGUAGE_BUILD_LANGUAGE_SYNTAX
