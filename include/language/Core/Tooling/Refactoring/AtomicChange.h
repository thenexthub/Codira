//===--- AtomicChange.h - AtomicChange class --------------------*- C++ -*-===//
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
//  This file defines AtomicChange which is used to create a set of source
//  changes, e.g. replacements and header insertions.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_ATOMICCHANGE_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_ATOMICCHANGE_H

#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Format/Format.h"
#include "language/Core/Tooling/Core/Replacement.h"
#include "toolchain/ADT/Any.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Error.h"

namespace language::Core {
namespace tooling {

/// An atomic change is used to create and group a set of source edits,
/// e.g. replacements or header insertions. Edits in an AtomicChange should be
/// related, e.g. replacements for the same type reference and the corresponding
/// header insertion/deletion.
///
/// An AtomicChange is uniquely identified by a key and will either be fully
/// applied or not applied at all.
///
/// Calling setError on an AtomicChange stores the error message and marks it as
/// bad, i.e. none of its source edits will be applied.
class AtomicChange {
public:
  /// Creates an atomic change around \p KeyPosition with the key being a
  /// concatenation of the file name and the offset of \p KeyPosition.
  /// \p KeyPosition should be the location of the key syntactical element that
  /// is being changed, e.g. the call to a refactored method.
  AtomicChange(const SourceManager &SM, SourceLocation KeyPosition);

  AtomicChange(const SourceManager &SM, SourceLocation KeyPosition,
               toolchain::Any Metadata);

  /// Creates an atomic change for \p FilePath with a customized key.
  AtomicChange(toolchain::StringRef FilePath, toolchain::StringRef Key)
      : Key(Key), FilePath(FilePath) {}

  AtomicChange(AtomicChange &&) = default;
  AtomicChange(const AtomicChange &) = default;

  AtomicChange &operator=(AtomicChange &&) = default;
  AtomicChange &operator=(const AtomicChange &) = default;

  bool operator==(const AtomicChange &Other) const;

  /// Returns the atomic change as a YAML string.
  std::string toYAMLString();

  /// Converts a YAML-encoded automic change to AtomicChange.
  static AtomicChange convertFromYAML(toolchain::StringRef YAMLContent);

  /// Returns the key of this change, which is a concatenation of the
  /// file name and offset of the key position.
  const std::string &getKey() const { return Key; }

  /// Returns the path of the file containing this atomic change.
  const std::string &getFilePath() const { return FilePath; }

  /// If this change could not be created successfully, e.g. because of
  /// conflicts among replacements, use this to set an error description.
  /// Thereby, places that cannot be fixed automatically can be gathered when
  /// applying changes.
  void setError(toolchain::StringRef Error) { this->Error = std::string(Error); }

  /// Returns whether an error has been set on this list.
  bool hasError() const { return !Error.empty(); }

  /// Returns the error message or an empty string if it does not exist.
  const std::string &getError() const { return Error; }

  /// Adds a replacement that replaces the given Range with
  /// ReplacementText.
  /// \returns An toolchain::Error carrying ReplacementError on error.
  toolchain::Error replace(const SourceManager &SM, const CharSourceRange &Range,
                      toolchain::StringRef ReplacementText);

  /// Adds a replacement that replaces range [Loc, Loc+Length) with
  /// \p Text.
  /// \returns An toolchain::Error carrying ReplacementError on error.
  toolchain::Error replace(const SourceManager &SM, SourceLocation Loc,
                      unsigned Length, toolchain::StringRef Text);

  /// Adds a replacement that inserts \p Text at \p Loc. If this
  /// insertion conflicts with an existing insertion (at the same position),
  /// this will be inserted before/after the existing insertion depending on
  /// \p InsertAfter. Users should use `replace` with `Length=0` instead if they
  /// do not want conflict resolving by default. If the conflicting replacement
  /// is not an insertion, an error is returned.
  ///
  /// \returns An toolchain::Error carrying ReplacementError on error.
  toolchain::Error insert(const SourceManager &SM, SourceLocation Loc,
                     toolchain::StringRef Text, bool InsertAfter = true);

  /// Adds a header into the file that contains the key position.
  /// Header can be in angle brackets or double quotation marks. By default
  /// (header is not quoted), header will be surrounded with double quotes.
  void addHeader(toolchain::StringRef Header);

  /// Removes a header from the file that contains the key position.
  void removeHeader(toolchain::StringRef Header);

  /// Returns a const reference to existing replacements.
  const Replacements &getReplacements() const { return Replaces; }

  Replacements &getReplacements() { return Replaces; }

  toolchain::ArrayRef<std::string> getInsertedHeaders() const {
    return InsertedHeaders;
  }

  toolchain::ArrayRef<std::string> getRemovedHeaders() const {
    return RemovedHeaders;
  }

  const toolchain::Any &getMetadata() const { return Metadata; }

private:
  AtomicChange() {}

  AtomicChange(std::string Key, std::string FilePath, std::string Error,
               std::vector<std::string> InsertedHeaders,
               std::vector<std::string> RemovedHeaders,
               language::Core::tooling::Replacements Replaces);

  // This uniquely identifies an AtomicChange.
  std::string Key;
  std::string FilePath;
  std::string Error;
  std::vector<std::string> InsertedHeaders;
  std::vector<std::string> RemovedHeaders;
  tooling::Replacements Replaces;

  // This field stores metadata which is ignored for the purposes of applying
  // edits to source, but may be useful for other consumers of AtomicChanges. In
  // particular, consumers can use this to direct how they want to consume each
  // edit.
  toolchain::Any Metadata;
};

using AtomicChanges = std::vector<AtomicChange>;

// Defines specs for applying changes.
struct ApplyChangesSpec {
  // If true, cleans up redundant/erroneous code around changed code with
  // clang-format's cleanup functionality, e.g. redundant commas around deleted
  // parameter or empty namespaces introduced by deletions.
  bool Cleanup = true;

  format::FormatStyle Style = format::getNoStyle();

  // Options for selectively formatting changes with clang-format:
  // kAll: Format all changed lines.
  // kNone: Don't format anything.
  // kViolations: Format lines exceeding the `ColumnLimit` in `Style`.
  enum FormatOption { kAll, kNone, kViolations };

  FormatOption Format = kNone;
};

/// Applies all AtomicChanges in \p Changes to the \p Code.
///
/// This completely ignores the file path in each change and replaces them with
/// \p FilePath, i.e. callers are responsible for ensuring all changes are for
/// the same file.
///
/// \returns The changed code if all changes are applied successfully;
/// otherwise, an toolchain::Error carrying toolchain::StringError is returned (the Error
/// message can be converted to string with `toolchain::toString()` and the
/// error_code should be ignored).
toolchain::Expected<std::string>
applyAtomicChanges(toolchain::StringRef FilePath, toolchain::StringRef Code,
                   toolchain::ArrayRef<AtomicChange> Changes,
                   const ApplyChangesSpec &Spec);

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_ATOMICCHANGE_H
