//===--- DiagnosticConsumer.cpp - Diagnostic Consumer Impl ----------------===//
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
//  This file implements the DiagnosticConsumer class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "language-ast"
#include "language/AST/DiagnosticConsumer.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Basic/Range.h"
#include "language/Basic/SourceManager.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/StringSet.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/raw_ostream.h"
using namespace language;

DiagnosticConsumer::~DiagnosticConsumer() = default;

DiagnosticInfo::FixIt::FixIt(CharSourceRange R, StringRef Str,
                             ArrayRef<DiagnosticArgument> Args) : Range(R) {
  // FIXME: Defer text formatting to later in the pipeline.
  toolchain::raw_string_ostream OS(Text);
  DiagnosticEngine::formatDiagnosticText(OS, Str, Args,
                                         DiagnosticFormatOptions::
                                         formatForFixIts());
}

toolchain::SMLoc DiagnosticConsumer::getRawLoc(SourceLoc loc) {
  return loc.Value;
}

TOOLCHAIN_ATTRIBUTE_UNUSED
static bool hasDuplicateFileNames(
    ArrayRef<FileSpecificDiagnosticConsumer::Subconsumer> subconsumers) {
  toolchain::StringSet<> seenFiles;
  for (const auto &subconsumer : subconsumers) {
    if (subconsumer.getInputFileName().empty()) {
      // We can handle multiple subconsumers that aren't associated with any
      // file, because they only collect diagnostics that aren't in any of the
      // special files. This isn't an important use case to support, but also
      // SmallSet doesn't handle empty strings anyway!
      continue;
    }

    bool isUnique = seenFiles.insert(subconsumer.getInputFileName()).second;
    if (!isUnique)
      return true;
  }
  return false;
}

std::unique_ptr<DiagnosticConsumer>
FileSpecificDiagnosticConsumer::consolidateSubconsumers(
    SmallVectorImpl<Subconsumer> &subconsumers) {
  if (subconsumers.empty())
    return nullptr;
  if (subconsumers.size() == 1)
    return std::move(subconsumers.front()).consumer;
  // Cannot use return
  // std::make_unique<FileSpecificDiagnosticConsumer>(subconsumers); because
  // the constructor is private.
  return std::unique_ptr<DiagnosticConsumer>(
      new FileSpecificDiagnosticConsumer(subconsumers));
}

FileSpecificDiagnosticConsumer::FileSpecificDiagnosticConsumer(
    SmallVectorImpl<Subconsumer> &subconsumers)
    : Subconsumers(std::move(subconsumers)) {
  assert(!Subconsumers.empty() &&
         "don't waste time handling diagnostics that will never get emitted");
  assert(!hasDuplicateFileNames(Subconsumers) &&
         "having multiple subconsumers for the same file is not implemented");
}

void FileSpecificDiagnosticConsumer::computeConsumersOrderedByRange(
    SourceManager &SM) {
  // Look up each file's source range and add it to the "map" (to be sorted).
  for (const unsigned subconsumerIndex: indices(Subconsumers)) {
    const Subconsumer &subconsumer = Subconsumers[subconsumerIndex];
    if (subconsumer.getInputFileName().empty())
      continue;

    std::optional<unsigned> bufferID =
        SM.getIDForBufferIdentifier(subconsumer.getInputFileName());
    assert(bufferID.has_value() && "consumer registered for unknown file");
    CharSourceRange range = SM.getRangeForBuffer(bufferID.value());
    ConsumersOrderedByRange.emplace_back(
        ConsumerAndRange(range, subconsumerIndex));
  }

  // Sort the "map" by buffer /end/ location, for use with std::lower_bound
  // later. (Sorting by start location would produce the same sort, since the
  // ranges must not be overlapping, but since we need to check end locations
  // later it's consistent to sort by that here.)
  std::sort(ConsumersOrderedByRange.begin(), ConsumersOrderedByRange.end());

  // Check that the ranges are non-overlapping. If the files really are all
  // distinct, this should be trivially true, but if it's ever not we might end
  // up mis-filing diagnostics.
  assert(ConsumersOrderedByRange.end() ==
             std::adjacent_find(ConsumersOrderedByRange.begin(),
                                ConsumersOrderedByRange.end(),
                                [](const ConsumerAndRange &left,
                                   const ConsumerAndRange &right) {
                                  return left.overlaps(right);
                                }) &&
         "overlapping ranges despite having distinct files");
}

std::optional<FileSpecificDiagnosticConsumer::Subconsumer *>
FileSpecificDiagnosticConsumer::subconsumerForLocation(SourceManager &SM,
                                                       SourceLoc loc) {
  // Diagnostics with invalid locations always go to every consumer.
  if (loc.isInvalid())
    return std::nullopt;

  // What if a there's a FileSpecificDiagnosticConsumer but there are no
  // subconsumers in it? (This situation occurs for the fix-its
  // FileSpecificDiagnosticConsumer.) In such a case, bail out now.
  if (Subconsumers.empty())
    return std::nullopt;

  // This map is generated on first use and cached, to allow the
  // FileSpecificDiagnosticConsumer to be set up before the source files are
  // actually loaded.
  if (ConsumersOrderedByRange.empty()) {

    // It's possible to get here while a bridging header PCH is being
    // attached-to, if there's some sort of AST-reader warning or error, which
    // happens before CompilerInstance::setUpInputs(), at which point _no_
    // source buffers are loaded in yet. In that case we return None, rather
    // than trying to build a nonsensical map (and actually crashing since we
    // can't find buffers for the inputs).
    assert(!Subconsumers.empty());
    if (!SM.getIDForBufferIdentifier(Subconsumers.begin()->getInputFileName())
             .has_value()) {
      assert(toolchain::none_of(Subconsumers, [&](const Subconsumer &subconsumer) {
        return SM.getIDForBufferIdentifier(subconsumer.getInputFileName())
            .has_value();
      }));
      return std::nullopt;
    }
    auto *mutableThis = const_cast<FileSpecificDiagnosticConsumer*>(this);
    mutableThis->computeConsumersOrderedByRange(SM);
  }

  // This std::lower_bound call is doing a binary search for the first range
  // that /might/ contain 'loc'. Specifically, since the ranges are sorted
  // by end location, it's looking for the first range where the end location
  // is greater than or equal to 'loc'.
  const ConsumerAndRange *possiblyContainingRangeIter = std::lower_bound(
      ConsumersOrderedByRange.begin(), ConsumersOrderedByRange.end(), loc,
      [](const ConsumerAndRange &entry, SourceLoc loc) -> bool {
        return entry.endsAfter(loc);
      });

  if (possiblyContainingRangeIter != ConsumersOrderedByRange.end() &&
      possiblyContainingRangeIter->contains(loc)) {
    auto *consumerAndRangeForLocation =
        const_cast<ConsumerAndRange *>(possiblyContainingRangeIter);
    return &(*this)[*consumerAndRangeForLocation];
  }

  return std::nullopt;
}

void FileSpecificDiagnosticConsumer::handleDiagnostic(
    SourceManager &SM, const DiagnosticInfo &Info) {

  HasAnErrorBeenConsumed |= Info.Kind == DiagnosticKind::Error;

  auto subconsumer = findSubconsumer(SM, Info);
  if (subconsumer) {
    subconsumer.value()->handleDiagnostic(SM, Info);
    return;
  }
  // Last resort: spray it everywhere
  for (auto &subconsumer : Subconsumers)
    subconsumer.handleDiagnostic(SM, Info);
}

std::optional<FileSpecificDiagnosticConsumer::Subconsumer *>
FileSpecificDiagnosticConsumer::findSubconsumer(SourceManager &SM,
                                                const DiagnosticInfo &Info) {
  // Ensure that a note goes to the same place as the preceding non-note.
  switch (Info.Kind) {
  case DiagnosticKind::Error:
  case DiagnosticKind::Warning:
  case DiagnosticKind::Remark: {
    auto subconsumer = findSubconsumerForNonNote(SM, Info);
    SubconsumerForSubsequentNotes = subconsumer;
    return subconsumer;
  }
  case DiagnosticKind::Note:
    return SubconsumerForSubsequentNotes;
  }
  toolchain_unreachable("covered switch");
}

std::optional<FileSpecificDiagnosticConsumer::Subconsumer *>
FileSpecificDiagnosticConsumer::findSubconsumerForNonNote(
    SourceManager &SM, const DiagnosticInfo &Info) {
  const auto subconsumer = subconsumerForLocation(SM, Info.Loc);
  if (!subconsumer)
    return std::nullopt; // No place to put it; might be in an imported module
  if ((*subconsumer)->getConsumer())
    return subconsumer; // A primary file with a .dia file
  // Try to put it in the responsible primary input
  if (Info.BufferIndirectlyCausingDiagnostic.isInvalid())
    return std::nullopt;
  const auto currentPrimarySubconsumer =
      subconsumerForLocation(SM, Info.BufferIndirectlyCausingDiagnostic);
  assert(!currentPrimarySubconsumer ||
         (*currentPrimarySubconsumer)->getConsumer() &&
             "current primary must have a .dia file");
  return currentPrimarySubconsumer;
}

bool FileSpecificDiagnosticConsumer::finishProcessing() {
  tellSubconsumersToInformDriverOfIncompleteBatchModeCompilation();

  // Deliberately don't use std::any_of here because we don't want early-exit
  // behavior.

  bool hadError = false;
  for (auto &subconsumer : Subconsumers)
    hadError |= subconsumer.getConsumer() &&
                subconsumer.getConsumer()->finishProcessing();
  return hadError;
}

void FileSpecificDiagnosticConsumer::
    tellSubconsumersToInformDriverOfIncompleteBatchModeCompilation() {
  if (!HasAnErrorBeenConsumed)
    return;
  for (auto &info : ConsumersOrderedByRange)
    (*this)[info].informDriverOfIncompleteBatchModeCompilation();
}

void NullDiagnosticConsumer::handleDiagnostic(SourceManager &SM,
                                              const DiagnosticInfo &Info) {
  TOOLCHAIN_DEBUG({
    toolchain::dbgs() << "NullDiagnosticConsumer received diagnostic: ";
    DiagnosticEngine::formatDiagnosticText(toolchain::dbgs(), Info.FormatString,
                                           Info.FormatArgs);
    toolchain::dbgs() << "\n";
  });
}

ForwardingDiagnosticConsumer::ForwardingDiagnosticConsumer(DiagnosticEngine &Target)
  : TargetEngine(Target) {}

void ForwardingDiagnosticConsumer::handleDiagnostic(
    SourceManager &SM, const DiagnosticInfo &Info) {
  TOOLCHAIN_DEBUG({
    toolchain::dbgs() << "ForwardingDiagnosticConsumer received diagnostic: ";
    DiagnosticEngine::formatDiagnosticText(toolchain::dbgs(), Info.FormatString,
                                           Info.FormatArgs);
    toolchain::dbgs() << "\n";
  });
  for (auto *C : TargetEngine.getConsumers()) {
    C->handleDiagnostic(SM, Info);
  }
}
