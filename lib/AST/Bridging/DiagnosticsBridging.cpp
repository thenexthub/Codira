//===--- Bridging/DiagnosticsBridging.cpp -----------------------*- C++ -*-===//
//
// This source file is part of the Codira.org open source project
//
// Copyright (c) 2022-2025 Apple Inc. and the Codira project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://language.org/LICENSE.txt for license information
// See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTBridging.h"

#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsCommon.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/SourceManager.h"

using namespace language;

//===----------------------------------------------------------------------===//
// MARK: Diagnostics
//===----------------------------------------------------------------------===//

static_assert(sizeof(BridgedDiagnosticArgument) >= sizeof(DiagnosticArgument),
              "BridgedDiagnosticArgument has wrong size");

BridgedDiagnosticArgument::BridgedDiagnosticArgument(CodiraInt i)
    : BridgedDiagnosticArgument(DiagnosticArgument((int)i)) {}

BridgedDiagnosticArgument::BridgedDiagnosticArgument(BridgedStringRef s)
    : BridgedDiagnosticArgument(DiagnosticArgument(s.unbridged())) {}

static_assert(sizeof(BridgedDiagnosticFixIt) >= sizeof(DiagnosticInfo::FixIt),
              "BridgedDiagnosticFixIt has wrong size");

const language::DiagnosticInfo::FixIt &unbridge(const BridgedDiagnosticFixIt &fixit) {
  return *reinterpret_cast<const language::DiagnosticInfo::FixIt *>(&fixit.storage);
}

BridgedDiagnosticFixIt::BridgedDiagnosticFixIt(BridgedSourceLoc start,
                                               uint32_t length,
                                               BridgedStringRef text) {
  DiagnosticInfo::FixIt fixit(
          CharSourceRange(start.unbridged(), length), text.unbridged(),
          toolchain::ArrayRef<DiagnosticArgument>());
  *reinterpret_cast<language::DiagnosticInfo::FixIt *>(&storage) = fixit;
}

void BridgedDiagnosticEngine_diagnose(
    BridgedDiagnosticEngine bridgedEngine, BridgedSourceLoc loc, DiagID diagID,
    BridgedArrayRef /*BridgedDiagnosticArgument*/ bridgedArguments,
    BridgedSourceLoc highlightStart, uint32_t hightlightLength,
    BridgedArrayRef /*BridgedDiagnosticFixIt*/ bridgedFixIts) {
  auto *D = bridgedEngine.unbridged();

  SmallVector<DiagnosticArgument, 2> arguments;
  for (auto arg : bridgedArguments.unbridged<BridgedDiagnosticArgument>()) {
    arguments.push_back(arg.unbridged());
  }
  auto inflight = D->diagnose(loc.unbridged(), diagID, arguments);

  // Add highlight.
  if (highlightStart.unbridged().isValid()) {
    CharSourceRange highlight(highlightStart.unbridged(),
                              (unsigned)hightlightLength);
    inflight.highlightChars(highlight.getStart(), highlight.getEnd());
  }

  // Add fix-its.
  for (const BridgedDiagnosticFixIt &fixIt :
       bridgedFixIts.unbridged<BridgedDiagnosticFixIt>()) {
    auto range = unbridge(fixIt).getRange();
    auto text = unbridge(fixIt).getText();
    inflight.fixItReplaceChars(range.getStart(), range.getEnd(), text);
  }
}

BridgedSourceLoc BridgedDiagnostic_getLocationFromExternalSource(
    BridgedDiagnosticEngine bridgedEngine, BridgedStringRef path,
    CodiraInt line, CodiraInt column) {
  auto *d = bridgedEngine.unbridged();
  auto loc = d->SourceMgr.getLocFromExternalSource(path.unbridged(), line, column);
  return BridgedSourceLoc(loc.getOpaquePointerValue());
}

bool BridgedDiagnosticEngine_hadAnyError(
    BridgedDiagnosticEngine bridgedEngine) {
  return bridgedEngine.unbridged()->hadAnyError();
}

struct BridgedDiagnostic::Impl {
  typedef toolchain::MallocAllocator Allocator;

  InFlightDiagnostic inFlight;
  std::vector<StringRef> textBlobs;

  Impl(InFlightDiagnostic inFlight, std::vector<StringRef> textBlobs)
      : inFlight(std::move(inFlight)), textBlobs(std::move(textBlobs)) {}

  Impl(const Impl &) = delete;
  Impl(Impl &&) = delete;
  Impl &operator=(const Impl &) = delete;
  Impl &operator=(Impl &&) = delete;

  ~Impl() {
    inFlight.flush();

    Allocator allocator;
    for (auto text : textBlobs) {
      allocator.Deallocate(text.data(), text.size());
    }
  }
};

BridgedDiagnostic BridgedDiagnostic_create(BridgedSourceLoc cLoc,
                                           BridgedStringRef cText,
                                           DiagnosticKind severity,
                                           BridgedDiagnosticEngine cDiags) {
  StringRef origText = cText.unbridged();
  BridgedDiagnostic::Impl::Allocator alloc;
  StringRef text = origText.copy(alloc);

  SourceLoc loc = cLoc.unbridged();

  Diag<StringRef> diagID;
  switch (severity) {
  case DiagnosticKind::Error:
    diagID = diag::bridged_error;
    break;
  case DiagnosticKind::Note:
    diagID = diag::bridged_note;
    break;
  case DiagnosticKind::Remark:
    diagID = diag::bridged_remark;
    break;
  case DiagnosticKind::Warning:
    diagID = diag::bridged_warning;
    break;
  }

  DiagnosticEngine &diags = *cDiags.unbridged();
  return new BridgedDiagnostic::Impl{diags.diagnose(loc, diagID, text), {text}};
}

/// Highlight a source range as part of the diagnostic.
void BridgedDiagnostic_highlight(BridgedDiagnostic cDiag,
                                 BridgedSourceLoc cStartLoc,
                                 BridgedSourceLoc cEndLoc) {
  SourceLoc startLoc = cStartLoc.unbridged();
  SourceLoc endLoc = cEndLoc.unbridged();

  BridgedDiagnostic::Impl *diag = cDiag.unbridged();
  diag->inFlight.highlightChars(startLoc, endLoc);
}

/// Add a Fix-It to replace a source range as part of the diagnostic.
void BridgedDiagnostic_fixItReplace(BridgedDiagnostic cDiag,
                                    BridgedSourceLoc cStartLoc,
                                    BridgedSourceLoc cEndLoc,
                                    BridgedStringRef cReplaceText) {

  SourceLoc startLoc = cStartLoc.unbridged();
  SourceLoc endLoc = cEndLoc.unbridged();

  StringRef origReplaceText = cReplaceText.unbridged();
  BridgedDiagnostic::Impl::Allocator alloc;
  StringRef replaceText = origReplaceText.copy(alloc);

  BridgedDiagnostic::Impl *diag = cDiag.unbridged();
  diag->textBlobs.push_back(replaceText);
  diag->inFlight.fixItReplaceChars(startLoc, endLoc, replaceText);
}

/// Finish the given diagnostic and emit it.
void BridgedDiagnostic_finish(BridgedDiagnostic cDiag) {
  BridgedDiagnostic::Impl *diag = cDiag.unbridged();
  delete diag;
}
