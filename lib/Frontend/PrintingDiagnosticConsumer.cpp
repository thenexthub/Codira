//===--- PrintingDiagnosticConsumer.cpp - Print Text Diagnostics ----------===//
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
//  This file implements the PrintingDiagnosticConsumer class.
//
//===----------------------------------------------------------------------===//

#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/AST/ASTBridging.h"
#include "language/AST/DiagnosticBridge.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/Basic/ColorUtils.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/SourceManager.h"
#include "language/Bridging/ASTGen.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/Twine.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

// MARK: Main DiagnosticConsumer entrypoint.
void PrintingDiagnosticConsumer::handleDiagnostic(SourceManager &SM,
                                                  const DiagnosticInfo &Info) {
  if (Info.Kind == DiagnosticKind::Error) {
    DidErrorOccur = true;
  }

  if (SuppressOutput)
    return;

  if (Info.IsChildNote)
    return;

  switch (FormattingStyle) {
  case DiagnosticOptions::FormattingStyle::Codira: {
#if LANGUAGE_BUILD_LANGUAGE_SYNTAX
    // Use the language-syntax formatter.
    auto bufferStack = DiagnosticBridge::getSourceBufferStack(SM, Info.Loc);
    if (Info.Kind != DiagnosticKind::Note || bufferStack.empty())
      DiagBridge.flush(Stream, /*includeTrailingBreak=*/true,
                       /*forceColors=*/ForceColors);

    if (bufferStack.empty()) {
      DiagBridge.emitDiagnosticWithoutLocation(Info, Stream, ForceColors);
    } else {
      DiagBridge.enqueueDiagnostic(SM, Info, bufferStack.front());
    }
    return;
#else
    // Fall through when we don't have the new diagnostics renderer available.
    TOOLCHAIN_FALLTHROUGH;
#endif
  }

  case DiagnosticOptions::FormattingStyle::LLVM:
    printDiagnostic(SM, Info);
    for (auto ChildInfo : Info.ChildDiagnosticInfo) {
      printDiagnostic(SM, *ChildInfo);
    }
    break;
  }
}

void PrintingDiagnosticConsumer::flush(bool includeTrailingBreak) {
#if LANGUAGE_BUILD_LANGUAGE_SYNTAX
  DiagBridge.flush(Stream, includeTrailingBreak,
                   /*forceColors=*/ForceColors);
#endif
}

bool PrintingDiagnosticConsumer::finishProcessing() {
  // If there's an in-flight snippet, flush it.
  flush(false);

#if LANGUAGE_BUILD_LANGUAGE_SYNTAX
  // Print out footnotes for any category that was referenced.
  DiagBridge.printCategoryFootnotes(Stream, ForceColors);
#endif

  return false;
}

// MARK: LLVM style diagnostic printing
void PrintingDiagnosticConsumer::printDiagnostic(SourceManager &SM,
                                                 const DiagnosticInfo &Info) {

  // Determine what kind of diagnostic we're emitting.
  toolchain::SourceMgr::DiagKind SMKind;
  switch (Info.Kind) {
  case DiagnosticKind::Error:
    SMKind = toolchain::SourceMgr::DK_Error;
    break;
  case DiagnosticKind::Warning:
    SMKind = toolchain::SourceMgr::DK_Warning;
    break;

  case DiagnosticKind::Note:
    SMKind = toolchain::SourceMgr::DK_Note;
    break;

  case DiagnosticKind::Remark:
    SMKind = toolchain::SourceMgr::DK_Remark;
    break;
  }

  // Translate ranges.
  SmallVector<toolchain::SMRange, 2> Ranges;
  for (auto R : Info.Ranges)
    Ranges.push_back(getRawRange(SM, R));

  // Translate fix-its.
  SmallVector<toolchain::SMFixIt, 2> FixIts;
  for (DiagnosticInfo::FixIt F : Info.FixIts)
    FixIts.push_back(getRawFixIt(SM, F));

  // Display the diagnostic.
  ColoredStream coloredErrs{Stream};
  raw_ostream &out = ForceColors ? coloredErrs : Stream;
  const toolchain::SourceMgr &rawSM = SM.getLLVMSourceMgr();
  
  // Actually substitute the diagnostic arguments into the diagnostic text.
  toolchain::SmallString<256> Text;
  {
    toolchain::raw_svector_ostream Out(Text);
    DiagnosticEngine::formatDiagnosticText(Out, Info.FormatString,
                                           Info.FormatArgs);

    if (!Info.Category.empty())
      Out << " [#" << Info.Category << "]";
  }

  auto Msg = SM.GetMessage(Info.Loc, SMKind, Text, Ranges, FixIts,
                           EmitMacroExpansionFiles);
  rawSM.PrintMessage(out, Msg, ForceColors);
}

toolchain::SMDiagnostic
SourceManager::GetMessage(SourceLoc Loc, toolchain::SourceMgr::DiagKind Kind,
                          const Twine &Msg,
                          ArrayRef<toolchain::SMRange> Ranges,
                          ArrayRef<toolchain::SMFixIt> FixIts,
                          bool EmitMacroExpansionFiles) const {

  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> LineAndCol;
  StringRef BufferID = "<unknown>";
  std::string LineStr;

  if (Loc.isValid()) {
    BufferID = getDisplayNameForLoc(Loc, EmitMacroExpansionFiles);
    auto CurMB = LLVMSourceMgr.getMemoryBuffer(findBufferContainingLoc(Loc));

    // Scan backward to find the start of the line.
    const char *LineStart = Loc.Value.getPointer();
    const char *BufStart = CurMB->getBufferStart();
    while (LineStart != BufStart && LineStart[-1] != '\n' &&
           LineStart[-1] != '\r')
      --LineStart;

    // Get the end of the line.
    const char *LineEnd = Loc.Value.getPointer();
    const char *BufEnd = CurMB->getBufferEnd();
    while (LineEnd != BufEnd && LineEnd[0] != '\n' && LineEnd[0] != '\r')
      ++LineEnd;
    LineStr = std::string(LineStart, LineEnd);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    for (unsigned i = 0, e = Ranges.size(); i != e; ++i) {
      toolchain::SMRange R = Ranges[i];
      if (!R.isValid()) continue;

      // If the line doesn't contain any part of the range, then ignore it.
      if (R.Start.getPointer() > LineEnd || R.End.getPointer() < LineStart)
        continue;

      // Ignore pieces of the range that go onto other lines.
      if (R.Start.getPointer() < LineStart)
        R.Start = toolchain::SMLoc::getFromPointer(LineStart);
      if (R.End.getPointer() > LineEnd)
        R.End = toolchain::SMLoc::getFromPointer(LineEnd);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.push_back(std::make_pair(R.Start.getPointer()-LineStart,
                                         R.End.getPointer()-LineStart));
    }

    LineAndCol = getPresumedLineAndColumnForLoc(Loc);
  }

  return toolchain::SMDiagnostic(LLVMSourceMgr, Loc.Value, BufferID,
                            LineAndCol.first,
                            LineAndCol.second-1, Kind, Msg.str(),
                            LineStr, ColRanges, FixIts);
}

// These must come after the declaration of AnnotatedSourceSnippet due to the
// `currentSnippet` member.
PrintingDiagnosticConsumer::PrintingDiagnosticConsumer(
    toolchain::raw_ostream &stream)
    : Stream(stream) {}

PrintingDiagnosticConsumer::~PrintingDiagnosticConsumer() {
  flush();
}
