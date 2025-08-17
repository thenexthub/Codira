//===--- AtomicChange.cpp - AtomicChange implementation ---------*- C++ -*-===//
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

#include "language/Core/Tooling/Refactoring/AtomicChange.h"
#include "language/Core/Tooling/ReplacementsYaml.h"
#include "toolchain/Support/YAMLTraits.h"
#include <string>

LLVM_YAML_IS_SEQUENCE_VECTOR(language::Core::tooling::AtomicChange)

namespace {
/// Helper to (de)serialize an AtomicChange since we don't have direct
/// access to its data members.
/// Data members of a normalized AtomicChange can be directly mapped from/to
/// YAML string.
struct NormalizedAtomicChange {
  NormalizedAtomicChange() = default;

  NormalizedAtomicChange(const toolchain::yaml::IO &) {}

  // This converts AtomicChange's internal implementation of the replacements
  // set to a vector of replacements.
  NormalizedAtomicChange(const toolchain::yaml::IO &,
                         const language::Core::tooling::AtomicChange &E)
      : Key(E.getKey()), FilePath(E.getFilePath()), Error(E.getError()),
        InsertedHeaders(E.getInsertedHeaders()),
        RemovedHeaders(E.getRemovedHeaders()),
        Replaces(E.getReplacements().begin(), E.getReplacements().end()) {}

  // This is not expected to be called but needed for template instantiation.
  language::Core::tooling::AtomicChange denormalize(const toolchain::yaml::IO &) {
    toolchain_unreachable("Do not convert YAML to AtomicChange directly with '>>'. "
                     "Use AtomicChange::convertFromYAML instead.");
  }
  std::string Key;
  std::string FilePath;
  std::string Error;
  std::vector<std::string> InsertedHeaders;
  std::vector<std::string> RemovedHeaders;
  std::vector<language::Core::tooling::Replacement> Replaces;
};
} // anonymous namespace

namespace toolchain {
namespace yaml {

/// Specialized MappingTraits to describe how an AtomicChange is
/// (de)serialized.
template <> struct MappingTraits<NormalizedAtomicChange> {
  static void mapping(IO &Io, NormalizedAtomicChange &Doc) {
    Io.mapRequired("Key", Doc.Key);
    Io.mapRequired("FilePath", Doc.FilePath);
    Io.mapRequired("Error", Doc.Error);
    Io.mapRequired("InsertedHeaders", Doc.InsertedHeaders);
    Io.mapRequired("RemovedHeaders", Doc.RemovedHeaders);
    Io.mapRequired("Replacements", Doc.Replaces);
  }
};

/// Specialized MappingTraits to describe how an AtomicChange is
/// (de)serialized.
template <> struct MappingTraits<language::Core::tooling::AtomicChange> {
  static void mapping(IO &Io, language::Core::tooling::AtomicChange &Doc) {
    MappingNormalization<NormalizedAtomicChange, language::Core::tooling::AtomicChange>
        Keys(Io, Doc);
    Io.mapRequired("Key", Keys->Key);
    Io.mapRequired("FilePath", Keys->FilePath);
    Io.mapRequired("Error", Keys->Error);
    Io.mapRequired("InsertedHeaders", Keys->InsertedHeaders);
    Io.mapRequired("RemovedHeaders", Keys->RemovedHeaders);
    Io.mapRequired("Replacements", Keys->Replaces);
  }
};

} // end namespace yaml
} // end namespace toolchain

namespace language::Core {
namespace tooling {
namespace {

// Returns true if there is any line that violates \p ColumnLimit in range
// [Start, End].
bool violatesColumnLimit(toolchain::StringRef Code, unsigned ColumnLimit,
                         unsigned Start, unsigned End) {
  auto StartPos = Code.rfind('\n', Start);
  StartPos = (StartPos == toolchain::StringRef::npos) ? 0 : StartPos + 1;

  auto EndPos = Code.find("\n", End);
  if (EndPos == toolchain::StringRef::npos)
    EndPos = Code.size();

  toolchain::SmallVector<toolchain::StringRef, 8> Lines;
  Code.substr(StartPos, EndPos - StartPos).split(Lines, '\n');
  for (toolchain::StringRef Line : Lines)
    if (Line.size() > ColumnLimit)
      return true;
  return false;
}

std::vector<Range>
getRangesForFormatting(toolchain::StringRef Code, unsigned ColumnLimit,
                       ApplyChangesSpec::FormatOption Format,
                       const language::Core::tooling::Replacements &Replaces) {
  // kNone suppresses formatting entirely.
  if (Format == ApplyChangesSpec::kNone)
    return {};
  std::vector<language::Core::tooling::Range> Ranges;
  // This works assuming that replacements are ordered by offset.
  // FIXME: use `getAffectedRanges()` to calculate when it does not include '\n'
  // at the end of an insertion in affected ranges.
  int Offset = 0;
  for (const language::Core::tooling::Replacement &R : Replaces) {
    int Start = R.getOffset() + Offset;
    int End = Start + R.getReplacementText().size();
    if (!R.getReplacementText().empty() &&
        R.getReplacementText().back() == '\n' && R.getLength() == 0 &&
        R.getOffset() > 0 && R.getOffset() <= Code.size() &&
        Code[R.getOffset() - 1] == '\n')
      // If we are inserting at the start of a line and the replacement ends in
      // a newline, we don't need to format the subsequent line.
      --End;
    Offset += R.getReplacementText().size() - R.getLength();

    if (Format == ApplyChangesSpec::kAll ||
        violatesColumnLimit(Code, ColumnLimit, Start, End))
      Ranges.emplace_back(Start, End - Start);
  }
  return Ranges;
}

inline toolchain::Error make_string_error(const toolchain::Twine &Message) {
  return toolchain::make_error<toolchain::StringError>(Message,
                                             toolchain::inconvertibleErrorCode());
}

// Creates replacements for inserting/deleting #include headers.
toolchain::Expected<Replacements>
createReplacementsForHeaders(toolchain::StringRef FilePath, toolchain::StringRef Code,
                             toolchain::ArrayRef<AtomicChange> Changes,
                             const format::FormatStyle &Style) {
  // Create header insertion/deletion replacements to be cleaned up
  // (i.e. converted to real insertion/deletion replacements).
  Replacements HeaderReplacements;
  for (const auto &Change : Changes) {
    for (toolchain::StringRef Header : Change.getInsertedHeaders()) {
      std::string EscapedHeader =
          Header.starts_with("<") || Header.starts_with("\"")
              ? Header.str()
              : ("\"" + Header + "\"").str();
      std::string ReplacementText = "#include " + EscapedHeader;
      // Offset UINT_MAX and length 0 indicate that the replacement is a header
      // insertion.
      toolchain::Error Err = HeaderReplacements.add(
          tooling::Replacement(FilePath, UINT_MAX, 0, ReplacementText));
      if (Err)
        return std::move(Err);
    }
    for (const std::string &Header : Change.getRemovedHeaders()) {
      // Offset UINT_MAX and length 1 indicate that the replacement is a header
      // deletion.
      toolchain::Error Err =
          HeaderReplacements.add(Replacement(FilePath, UINT_MAX, 1, Header));
      if (Err)
        return std::move(Err);
    }
  }

  // cleanupAroundReplacements() converts header insertions/deletions into
  // actual replacements that add/remove headers at the right location.
  return language::Core::format::cleanupAroundReplacements(Code, HeaderReplacements,
                                                  Style);
}

// Combine replacements in all Changes as a `Replacements`. This ignores the
// file path in all replacements and replaces them with \p FilePath.
toolchain::Expected<Replacements>
combineReplacementsInChanges(toolchain::StringRef FilePath,
                             toolchain::ArrayRef<AtomicChange> Changes) {
  Replacements Replaces;
  for (const auto &Change : Changes)
    for (const auto &R : Change.getReplacements())
      if (auto Err = Replaces.add(Replacement(
              FilePath, R.getOffset(), R.getLength(), R.getReplacementText())))
        return std::move(Err);
  return Replaces;
}

} // end namespace

AtomicChange::AtomicChange(const SourceManager &SM,
                           SourceLocation KeyPosition) {
  const FullSourceLoc FullKeyPosition(KeyPosition, SM);
  auto FileIDAndOffset = FullKeyPosition.getSpellingLoc().getDecomposedLoc();
  OptionalFileEntryRef FE = SM.getFileEntryRefForID(FileIDAndOffset.first);
  assert(FE && "Cannot create AtomicChange with invalid location.");
  FilePath = std::string(FE->getName());
  Key = FilePath + ":" + std::to_string(FileIDAndOffset.second);
}

AtomicChange::AtomicChange(const SourceManager &SM, SourceLocation KeyPosition,
                           toolchain::Any M)
    : AtomicChange(SM, KeyPosition) {
  Metadata = std::move(M);
}

AtomicChange::AtomicChange(std::string Key, std::string FilePath,
                           std::string Error,
                           std::vector<std::string> InsertedHeaders,
                           std::vector<std::string> RemovedHeaders,
                           language::Core::tooling::Replacements Replaces)
    : Key(std::move(Key)), FilePath(std::move(FilePath)),
      Error(std::move(Error)), InsertedHeaders(std::move(InsertedHeaders)),
      RemovedHeaders(std::move(RemovedHeaders)), Replaces(std::move(Replaces)) {
}

bool AtomicChange::operator==(const AtomicChange &Other) const {
  if (Key != Other.Key || FilePath != Other.FilePath || Error != Other.Error)
    return false;
  if (!(Replaces == Other.Replaces))
    return false;
  // FXIME: Compare header insertions/removals.
  return true;
}

std::string AtomicChange::toYAMLString() {
  std::string YamlContent;
  toolchain::raw_string_ostream YamlContentStream(YamlContent);

  toolchain::yaml::Output YAML(YamlContentStream);
  YAML << *this;
  return YamlContent;
}

AtomicChange AtomicChange::convertFromYAML(toolchain::StringRef YAMLContent) {
  NormalizedAtomicChange NE;
  toolchain::yaml::Input YAML(YAMLContent);
  YAML >> NE;
  AtomicChange E(NE.Key, NE.FilePath, NE.Error, NE.InsertedHeaders,
                 NE.RemovedHeaders, tooling::Replacements());
  for (const auto &R : NE.Replaces) {
    toolchain::Error Err = E.Replaces.add(R);
    if (Err)
      toolchain_unreachable(
          "Failed to add replacement when Converting YAML to AtomicChange.");
    toolchain::consumeError(std::move(Err));
  }
  return E;
}

toolchain::Error AtomicChange::replace(const SourceManager &SM,
                                  const CharSourceRange &Range,
                                  toolchain::StringRef ReplacementText) {
  return Replaces.add(Replacement(SM, Range, ReplacementText));
}

toolchain::Error AtomicChange::replace(const SourceManager &SM, SourceLocation Loc,
                                  unsigned Length, toolchain::StringRef Text) {
  return Replaces.add(Replacement(SM, Loc, Length, Text));
}

toolchain::Error AtomicChange::insert(const SourceManager &SM, SourceLocation Loc,
                                 toolchain::StringRef Text, bool InsertAfter) {
  if (Text.empty())
    return toolchain::Error::success();
  Replacement R(SM, Loc, 0, Text);
  toolchain::Error Err = Replaces.add(R);
  if (Err) {
    return toolchain::handleErrors(
        std::move(Err), [&](const ReplacementError &RE) -> toolchain::Error {
          if (RE.get() != replacement_error::insert_conflict)
            return toolchain::make_error<ReplacementError>(RE);
          unsigned NewOffset = Replaces.getShiftedCodePosition(R.getOffset());
          if (!InsertAfter)
            NewOffset -=
                RE.getExistingReplacement()->getReplacementText().size();
          Replacement NewR(R.getFilePath(), NewOffset, 0, Text);
          Replaces = Replaces.merge(Replacements(NewR));
          return toolchain::Error::success();
        });
  }
  return toolchain::Error::success();
}

void AtomicChange::addHeader(toolchain::StringRef Header) {
  InsertedHeaders.push_back(std::string(Header));
}

void AtomicChange::removeHeader(toolchain::StringRef Header) {
  RemovedHeaders.push_back(std::string(Header));
}

toolchain::Expected<std::string>
applyAtomicChanges(toolchain::StringRef FilePath, toolchain::StringRef Code,
                   toolchain::ArrayRef<AtomicChange> Changes,
                   const ApplyChangesSpec &Spec) {
  toolchain::Expected<Replacements> HeaderReplacements =
      createReplacementsForHeaders(FilePath, Code, Changes, Spec.Style);
  if (!HeaderReplacements)
    return make_string_error(
        "Failed to create replacements for header changes: " +
        toolchain::toString(HeaderReplacements.takeError()));

  toolchain::Expected<Replacements> Replaces =
      combineReplacementsInChanges(FilePath, Changes);
  if (!Replaces)
    return make_string_error("Failed to combine replacements in all changes: " +
                             toolchain::toString(Replaces.takeError()));

  Replacements AllReplaces = std::move(*Replaces);
  for (const auto &R : *HeaderReplacements) {
    toolchain::Error Err = AllReplaces.add(R);
    if (Err)
      return make_string_error(
          "Failed to combine existing replacements with header replacements: " +
          toolchain::toString(std::move(Err)));
  }

  if (Spec.Cleanup) {
    toolchain::Expected<Replacements> CleanReplaces =
        format::cleanupAroundReplacements(Code, AllReplaces, Spec.Style);
    if (!CleanReplaces)
      return make_string_error("Failed to cleanup around replacements: " +
                               toolchain::toString(CleanReplaces.takeError()));
    AllReplaces = std::move(*CleanReplaces);
  }

  // Apply all replacements.
  toolchain::Expected<std::string> ChangedCode =
      applyAllReplacements(Code, AllReplaces);
  if (!ChangedCode)
    return make_string_error("Failed to apply all replacements: " +
                             toolchain::toString(ChangedCode.takeError()));

  // Sort inserted headers. This is done even if other formatting is turned off
  // as incorrectly sorted headers are always just wrong, it's not a matter of
  // taste.
  Replacements HeaderSortingReplacements = format::sortIncludes(
      Spec.Style, *ChangedCode, AllReplaces.getAffectedRanges(), FilePath);
  ChangedCode = applyAllReplacements(*ChangedCode, HeaderSortingReplacements);
  if (!ChangedCode)
    return make_string_error(
        "Failed to apply replacements for sorting includes: " +
        toolchain::toString(ChangedCode.takeError()));

  AllReplaces = AllReplaces.merge(HeaderSortingReplacements);

  std::vector<Range> FormatRanges = getRangesForFormatting(
      *ChangedCode, Spec.Style.ColumnLimit, Spec.Format, AllReplaces);
  if (!FormatRanges.empty()) {
    Replacements FormatReplacements =
        format::reformat(Spec.Style, *ChangedCode, FormatRanges, FilePath);
    ChangedCode = applyAllReplacements(*ChangedCode, FormatReplacements);
    if (!ChangedCode)
      return make_string_error(
          "Failed to apply replacements for formatting changed code: " +
          toolchain::toString(ChangedCode.takeError()));
  }
  return ChangedCode;
}

} // end namespace tooling
} // end namespace language::Core
