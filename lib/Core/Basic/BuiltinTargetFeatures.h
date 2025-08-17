//===-- CodeGenFunction.h - Target features for builtin ---------*- C++ -*-===//
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
// This is the internal required target features for builtin.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_BUILTINTARGETFEATURES_H
#define LANGUAGE_CORE_LIB_BASIC_BUILTINTARGETFEATURES_H
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"

using toolchain::StringRef;

namespace language::Core {
namespace Builtin {
/// TargetFeatures - This class is used to check whether the builtin function
/// has the required tagert specific features. It is able to support the
/// combination of ','(and), '|'(or), and '()'. By default, the priority of
/// ',' is higher than that of '|' .
/// E.g:
/// A,B|C means the builtin function requires both A and B, or C.
/// If we want the builtin function requires both A and B, or both A and C,
/// there are two ways: A,B|A,C or A,(B|C).
/// The FeaturesList should not contain spaces, and brackets must appear in
/// pairs.
class TargetFeatures {
  struct FeatureListStatus {
    bool HasFeatures;
    StringRef CurFeaturesList;
  };

  const toolchain::StringMap<bool> &CallerFeatureMap;

  FeatureListStatus getAndFeatures(StringRef FeatureList) {
    int InParentheses = 0;
    bool HasFeatures = true;
    size_t SubexpressionStart = 0;
    for (size_t i = 0, e = FeatureList.size(); i < e; ++i) {
      char CurrentToken = FeatureList[i];
      switch (CurrentToken) {
      default:
        break;
      case '(':
        if (InParentheses == 0)
          SubexpressionStart = i + 1;
        ++InParentheses;
        break;
      case ')':
        --InParentheses;
        assert(InParentheses >= 0 && "Parentheses are not in pair");
        [[fallthrough]];
      case '|':
      case ',':
        if (InParentheses == 0) {
          if (HasFeatures && i != SubexpressionStart) {
            StringRef F = FeatureList.slice(SubexpressionStart, i);
            HasFeatures = CurrentToken == ')' ? hasRequiredFeatures(F)
                                              : CallerFeatureMap.lookup(F);
          }
          SubexpressionStart = i + 1;
          if (CurrentToken == '|') {
            return {HasFeatures, FeatureList.substr(SubexpressionStart)};
          }
        }
        break;
      }
    }
    assert(InParentheses == 0 && "Parentheses are not in pair");
    if (HasFeatures && SubexpressionStart != FeatureList.size())
      HasFeatures =
          CallerFeatureMap.lookup(FeatureList.substr(SubexpressionStart));
    return {HasFeatures, StringRef()};
  }

public:
  bool hasRequiredFeatures(StringRef FeatureList) {
    FeatureListStatus FS = {false, FeatureList};
    while (!FS.HasFeatures && !FS.CurFeaturesList.empty())
      FS = getAndFeatures(FS.CurFeaturesList);
    return FS.HasFeatures;
  }

  TargetFeatures(const toolchain::StringMap<bool> &CallerFeatureMap)
      : CallerFeatureMap(CallerFeatureMap) {}
};

} // namespace Builtin
} // namespace language::Core
#endif /* CLANG_LIB_BASIC_BUILTINTARGETFEATURES_H */
