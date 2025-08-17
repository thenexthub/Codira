//===--- UsingDeclarationsSorter.cpp ----------------------------*- C++ -*-===//
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
///
/// \file
/// This file implements UsingDeclarationsSorter, a TokenAnalyzer that
/// sorts consecutive using declarations.
///
//===----------------------------------------------------------------------===//

#include "UsingDeclarationsSorter.h"
#include "language/Core/Format/Format.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/Regex.h"

#include <algorithm>

#define DEBUG_TYPE "using-declarations-sorter"

namespace language::Core {
namespace format {

namespace {

// The order of using declaration is defined as follows:
// Split the strings by "::" and discard any initial empty strings. The last
// element of each list is a non-namespace name; all others are namespace
// names. Sort the lists of names lexicographically, where the sort order of
// individual names is that all non-namespace names come before all namespace
// names, and within those groups, names are in case-insensitive lexicographic
// order.
int compareLabelsLexicographicNumeric(StringRef A, StringRef B) {
  SmallVector<StringRef, 2> NamesA;
  A.split(NamesA, "::", /*MaxSplit=*/-1, /*KeepEmpty=*/false);
  SmallVector<StringRef, 2> NamesB;
  B.split(NamesB, "::", /*MaxSplit=*/-1, /*KeepEmpty=*/false);
  size_t SizeA = NamesA.size();
  size_t SizeB = NamesB.size();
  for (size_t I = 0, E = std::min(SizeA, SizeB); I < E; ++I) {
    if (I + 1 == SizeA) {
      // I is the last index of NamesA and NamesA[I] is a non-namespace name.

      // Non-namespace names come before all namespace names.
      if (SizeB > SizeA)
        return -1;

      // Two names within a group compare case-insensitively.
      return NamesA[I].compare_insensitive(NamesB[I]);
    }

    // I is the last index of NamesB and NamesB[I] is a non-namespace name.
    // Non-namespace names come before all namespace names.
    if (I + 1 == SizeB)
      return 1;

    // Two namespaces names within a group compare case-insensitively.
    int C = NamesA[I].compare_insensitive(NamesB[I]);
    if (C != 0)
      return C;
  }
  return 0;
}

int compareLabelsLexicographic(StringRef A, StringRef B) {
  SmallVector<StringRef, 2> NamesA;
  A.split(NamesA, "::", /*MaxSplit=*/-1, /*KeepEmpty=*/false);
  SmallVector<StringRef, 2> NamesB;
  B.split(NamesB, "::", /*MaxSplit=*/-1, /*KeepEmpty=*/false);
  size_t SizeA = NamesA.size();
  size_t SizeB = NamesB.size();
  for (size_t I = 0, E = std::min(SizeA, SizeB); I < E; ++I) {
    // Two namespaces names within a group compare case-insensitively.
    int C = NamesA[I].compare_insensitive(NamesB[I]);
    if (C != 0)
      return C;
  }
  if (SizeA < SizeB)
    return -1;
  return SizeA == SizeB ? 0 : 1;
}

int compareLabels(
    StringRef A, StringRef B,
    FormatStyle::SortUsingDeclarationsOptions SortUsingDeclarations) {
  if (SortUsingDeclarations == FormatStyle::SUD_LexicographicNumeric)
    return compareLabelsLexicographicNumeric(A, B);
  return compareLabelsLexicographic(A, B);
}

struct UsingDeclaration {
  const AnnotatedLine *Line;
  std::string Label;

  UsingDeclaration(const AnnotatedLine *Line, const std::string &Label)
      : Line(Line), Label(Label) {}
};

/// Computes the label of a using declaration starting at tthe using token
/// \p UsingTok.
/// If \p UsingTok doesn't begin a using declaration, returns the empty string.
/// Note that this detects specifically using declarations, as in:
/// using A::B::C;
/// and not type aliases, as in:
/// using A = B::C;
/// Type aliases are in general not safe to permute.
std::string computeUsingDeclarationLabel(const FormatToken *UsingTok) {
  assert(UsingTok && UsingTok->is(tok::kw_using) && "Expecting a using token");
  std::string Label;
  const FormatToken *Tok = UsingTok->Next;
  if (Tok && Tok->is(tok::kw_typename)) {
    Label.append("typename ");
    Tok = Tok->Next;
  }
  if (Tok && Tok->is(tok::coloncolon)) {
    Label.append("::");
    Tok = Tok->Next;
  }
  bool HasIdentifier = false;
  while (Tok && Tok->is(tok::identifier)) {
    HasIdentifier = true;
    Label.append(Tok->TokenText.str());
    Tok = Tok->Next;
    if (!Tok || Tok->isNot(tok::coloncolon))
      break;
    Label.append("::");
    Tok = Tok->Next;
  }
  if (HasIdentifier && Tok && Tok->isOneOf(tok::semi, tok::comma))
    return Label;
  return "";
}

void endUsingDeclarationBlock(
    SmallVectorImpl<UsingDeclaration> *UsingDeclarations,
    const SourceManager &SourceMgr, tooling::Replacements *Fixes,
    FormatStyle::SortUsingDeclarationsOptions SortUsingDeclarations) {
  bool BlockAffected = false;
  for (const UsingDeclaration &Declaration : *UsingDeclarations) {
    if (Declaration.Line->Affected) {
      BlockAffected = true;
      break;
    }
  }
  if (!BlockAffected) {
    UsingDeclarations->clear();
    return;
  }
  SmallVector<UsingDeclaration, 4> SortedUsingDeclarations(
      UsingDeclarations->begin(), UsingDeclarations->end());
  auto Comp = [SortUsingDeclarations](const UsingDeclaration &Lhs,
                                      const UsingDeclaration &Rhs) -> bool {
    return compareLabels(Lhs.Label, Rhs.Label, SortUsingDeclarations) < 0;
  };
  toolchain::stable_sort(SortedUsingDeclarations, Comp);
  SortedUsingDeclarations.erase(
      toolchain::unique(SortedUsingDeclarations,
                   [](const UsingDeclaration &a, const UsingDeclaration &b) {
                     return a.Label == b.Label;
                   }),
      SortedUsingDeclarations.end());
  for (size_t I = 0, E = UsingDeclarations->size(); I < E; ++I) {
    if (I >= SortedUsingDeclarations.size()) {
      // This using declaration has been deduplicated, delete it.
      auto Begin =
          (*UsingDeclarations)[I].Line->First->WhitespaceRange.getBegin();
      auto End = (*UsingDeclarations)[I].Line->Last->Tok.getEndLoc();
      auto Range = CharSourceRange::getCharRange(Begin, End);
      auto Err = Fixes->add(tooling::Replacement(SourceMgr, Range, ""));
      if (Err) {
        toolchain::errs() << "Error while sorting using declarations: "
                     << toolchain::toString(std::move(Err)) << "\n";
      }
      continue;
    }
    if ((*UsingDeclarations)[I].Line == SortedUsingDeclarations[I].Line)
      continue;
    auto Begin = (*UsingDeclarations)[I].Line->First->Tok.getLocation();
    auto End = (*UsingDeclarations)[I].Line->Last->Tok.getEndLoc();
    auto SortedBegin =
        SortedUsingDeclarations[I].Line->First->Tok.getLocation();
    auto SortedEnd = SortedUsingDeclarations[I].Line->Last->Tok.getEndLoc();
    StringRef Text(SourceMgr.getCharacterData(SortedBegin),
                   SourceMgr.getCharacterData(SortedEnd) -
                       SourceMgr.getCharacterData(SortedBegin));
    LLVM_DEBUG({
      StringRef OldText(SourceMgr.getCharacterData(Begin),
                        SourceMgr.getCharacterData(End) -
                            SourceMgr.getCharacterData(Begin));
      toolchain::dbgs() << "Replacing '" << OldText << "' with '" << Text << "'\n";
    });
    auto Range = CharSourceRange::getCharRange(Begin, End);
    auto Err = Fixes->add(tooling::Replacement(SourceMgr, Range, Text));
    if (Err) {
      toolchain::errs() << "Error while sorting using declarations: "
                   << toolchain::toString(std::move(Err)) << "\n";
    }
  }
  UsingDeclarations->clear();
}

} // namespace

UsingDeclarationsSorter::UsingDeclarationsSorter(const Environment &Env,
                                                 const FormatStyle &Style)
    : TokenAnalyzer(Env, Style) {}

std::pair<tooling::Replacements, unsigned> UsingDeclarationsSorter::analyze(
    TokenAnnotator &Annotator, SmallVectorImpl<AnnotatedLine *> &AnnotatedLines,
    FormatTokenLexer &Tokens) {
  const SourceManager &SourceMgr = Env.getSourceManager();
  AffectedRangeMgr.computeAffectedLines(AnnotatedLines);
  tooling::Replacements Fixes;
  SmallVector<UsingDeclaration, 4> UsingDeclarations;
  for (const AnnotatedLine *Line : AnnotatedLines) {
    const auto *FirstTok = Line->First;
    if (Line->InPPDirective || !Line->startsWith(tok::kw_using) ||
        FirstTok->Finalized) {
      endUsingDeclarationBlock(&UsingDeclarations, SourceMgr, &Fixes,
                               Style.SortUsingDeclarations);
      continue;
    }
    if (FirstTok->NewlinesBefore > 1) {
      endUsingDeclarationBlock(&UsingDeclarations, SourceMgr, &Fixes,
                               Style.SortUsingDeclarations);
    }
    const auto *UsingTok =
        FirstTok->is(tok::comment) ? FirstTok->getNextNonComment() : FirstTok;
    std::string Label = computeUsingDeclarationLabel(UsingTok);
    if (Label.empty()) {
      endUsingDeclarationBlock(&UsingDeclarations, SourceMgr, &Fixes,
                               Style.SortUsingDeclarations);
      continue;
    }
    UsingDeclarations.push_back(UsingDeclaration(Line, Label));
  }
  endUsingDeclarationBlock(&UsingDeclarations, SourceMgr, &Fixes,
                           Style.SortUsingDeclarations);
  return {Fixes, 0};
}

} // namespace format
} // namespace language::Core
