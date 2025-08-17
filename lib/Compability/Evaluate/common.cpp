//===-- lib/Evaluate/common.cpp -------------------------------------------===//
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

#include "language/Compability/Evaluate/common.h"
#include "language/Compability/Common/idioms.h"

using namespace language::Compability::parser::literals;

namespace language::Compability::evaluate {

void RealFlagWarnings(
    FoldingContext &context, const RealFlags &flags, const char *operation) {
  static constexpr auto warning{common::UsageWarning::FoldingException};
  if (context.languageFeatures().ShouldWarn(warning)) {
    if (flags.test(RealFlag::Overflow)) {
      context.messages().Say(warning, "overflow on %s"_warn_en_US, operation);
    }
    if (flags.test(RealFlag::DivideByZero)) {
      if (std::strcmp(operation, "division") == 0) {
        context.messages().Say(warning, "division by zero"_warn_en_US);
      } else {
        context.messages().Say(
            warning, "division by zero on %s"_warn_en_US, operation);
      }
    }
    if (flags.test(RealFlag::InvalidArgument)) {
      context.messages().Say(
          warning, "invalid argument on %s"_warn_en_US, operation);
    }
    if (flags.test(RealFlag::Underflow)) {
      context.messages().Say(warning, "underflow on %s"_warn_en_US, operation);
    }
  }
}

ConstantSubscript &FoldingContext::StartImpliedDo(
    parser::CharBlock name, ConstantSubscript n) {
  auto pair{impliedDos_.insert(std::make_pair(name, n))};
  CHECK(pair.second);
  return pair.first->second;
}

std::optional<ConstantSubscript> FoldingContext::GetImpliedDo(
    parser::CharBlock name) const {
  if (auto iter{impliedDos_.find(name)}; iter != impliedDos_.cend()) {
    return {iter->second};
  } else {
    return std::nullopt;
  }
}

void FoldingContext::EndImpliedDo(parser::CharBlock name) {
  auto iter{impliedDos_.find(name)};
  if (iter != impliedDos_.end()) {
    impliedDos_.erase(iter);
  }
}
} // namespace language::Compability::evaluate
