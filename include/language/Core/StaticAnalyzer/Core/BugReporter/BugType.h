//===---  BugType.h - Bug Information Description ---------------*- C++ -*-===//
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
//  This file defines BugType, a class representing a bug type.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_BUGREPORTER_BUGTYPE_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_BUGREPORTER_BUGTYPE_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/CommonBugCategories.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include <string>
#include <variant>

namespace language::Core {

namespace ento {

class BugReporter;

class BugType {
private:
  using CheckerNameInfo = std::variant<CheckerNameRef, const CheckerFrontend *>;

  const CheckerNameInfo CheckerName;
  const std::string Description;
  const std::string Category;
  bool SuppressOnSink;

  virtual void anchor();

public:
  // Straightforward constructor where the checker name is specified directly.
  // TODO: As far as I know all applications of this constructor involve ugly
  // hacks that could be avoided by switching to the other constructor.
  // When those are all eliminated, this constructor should be removed to
  // eliminate the `variant` and simplify this class.
  BugType(CheckerNameRef CheckerName, StringRef Desc,
          StringRef Cat = categories::LogicError, bool SuppressOnSink = false)
      : CheckerName(CheckerName), Description(Desc), Category(Cat),
        SuppressOnSink(SuppressOnSink) {}
  // Constructor that can be called from the constructor of a checker object.
  // At that point the checker name is not yet available, but we can save a
  // pointer to the checker and use that to query the name.
  BugType(const CheckerFrontend *Checker, StringRef Desc,
          StringRef Cat = categories::LogicError, bool SuppressOnSink = false)
      : CheckerName(Checker), Description(Desc), Category(Cat),
        SuppressOnSink(SuppressOnSink) {}
  virtual ~BugType() = default;

  StringRef getDescription() const { return Description; }
  StringRef getCategory() const { return Category; }
  StringRef getCheckerName() const {
    if (const auto *CNR = std::get_if<CheckerNameRef>(&CheckerName))
      return *CNR;

    return std::get<const CheckerFrontend *>(CheckerName)->getName();
  }

  /// isSuppressOnSink - Returns true if bug reports associated with this bug
  ///  type should be suppressed if the end node of the report is post-dominated
  ///  by a sink node.
  bool isSuppressOnSink() const { return SuppressOnSink; }
};

/// Trivial convenience class for the common case when a certain checker
/// frontend always uses the same bug type. This way instead of writing
/// ```
///   CheckerFrontend LongCheckerFrontendName;
///   BugType LongCheckerFrontendNameBug{LongCheckerFrontendName, "..."};
/// ```
/// we can use `CheckerFrontendWithBugType LongCheckerFrontendName{"..."}`.
class CheckerFrontendWithBugType : public CheckerFrontend, public BugType {
public:
  CheckerFrontendWithBugType(StringRef Desc,
                             StringRef Cat = categories::LogicError,
                             bool SuppressOnSink = false)
      : BugType(this, Desc, Cat, SuppressOnSink) {}
};

} // namespace ento

} // end clang namespace
#endif
