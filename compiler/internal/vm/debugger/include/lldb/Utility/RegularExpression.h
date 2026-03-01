//===-- RegularExpression.h -------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef LLDB_UTILITY_REGULAREXPRESSION_H
#define LLDB_UTILITY_REGULAREXPRESSION_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/Regex.h"

namespace lldb_private {

class RegularExpression {
public:
  /// The default constructor that initializes the object state such that it
  /// contains no compiled regular expression.
  RegularExpression() = default;

  /// Constructor for a regular expression.
  ///
  /// Compile a regular expression using the supplied regular expression text.
  /// The compiled regular expression lives in this object so that it can be
  /// readily used for regular expression matches. Execute() can be called
  /// after the regular expression is compiled.
  ///
  /// \param[in] string
  ///     An llvm::StringRef that represents the regular expression to compile.
  //      String is not referenced anymore after the object is constructed.
  //
  /// \param[in] flags
  ///     An llvm::Regex::RegexFlags that modifies the matching behavior. The
  ///     default is NoFlags.
  explicit RegularExpression(
      llvm::StringRef string,
      llvm::Regex::RegexFlags flags = llvm::Regex::NoFlags);

  ~RegularExpression() = default;

  RegularExpression(const RegularExpression &rhs);
  RegularExpression(RegularExpression &&rhs) = default;

  RegularExpression &operator=(RegularExpression &&rhs) = default;
  RegularExpression &operator=(const RegularExpression &rhs) = default;

  /// Execute a regular expression match using the compiled regular expression
  /// that is already in this object against the given \a string. If any parens
  /// are used for regular expression matches.
  ///
  /// \param[in] string
  ///     The string to match against the compile regular expression.
  ///
  /// \param[out] matches
  ///     A pointer to a SmallVector to hold the matches.
  ///
  /// \return
  ///     true if \a string matches the compiled regular expression, false
  ///     otherwise incl. the case regular exression failed to compile.
  bool Execute(llvm::StringRef string,
               llvm::SmallVectorImpl<llvm::StringRef> *matches = nullptr) const;

  /// Access the regular expression text.
  ///
  /// \return
  ///     The NULL terminated C string that was used to compile the
  ///     current regular expression
  llvm::StringRef GetText() const;

  /// Test if this object contains a valid regular expression.
  ///
  /// \return
  ///     true if the regular expression compiled and is ready for execution,
  ///     false otherwise.
  bool IsValid() const;

  /// Return an error if the regular expression failed to compile.
  ///
  /// \return
  ///     A string error if the regular expression failed to compile, success
  ///     otherwise.
  llvm::Error GetError() const;

  bool operator==(const RegularExpression &rhs) const {
    return GetText() == rhs.GetText();
  }

private:
  /// A copy of the original regular expression text.
  std::string m_regex_text;
  /// The compiled regular expression.
  mutable llvm::Regex m_regex;
};

} // namespace lldb_private

#endif // LLDB_UTILITY_REGULAREXPRESSION_H
