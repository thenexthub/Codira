//===--- RegistryManager.h - Matcher registry -------------------*- C++ -*-===//
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
//
// RegistryManager to manage registry of all known matchers.
//
// The registry provides a generic interface to construct any matcher by name.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_TOOLS_MLIRQUERY_MATCHER_REGISTRYMANAGER_H
#define MLIR_TOOLS_MLIRQUERY_MATCHER_REGISTRYMANAGER_H

#include "Diagnostics.h"
#include "mlir/Query/Matcher/Marshallers.h"
#include "mlir/Query/Matcher/Registry.h"
#include "mlir/Query/Matcher/VariantValue.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/StringMap.h"
#include "vm/core/ADT/StringRef.h"
#include <string>

namespace mlir::query::matcher {

using MatcherCtor = const internal::MatcherDescriptor *;

struct MatcherCompletion {
  MatcherCompletion() = default;
  MatcherCompletion(toolchain::StringRef typedText, toolchain::StringRef matcherDecl)
      : typedText(typedText.str()), matcherDecl(matcherDecl.str()) {}

  bool operator==(const MatcherCompletion &other) const {
    return typedText == other.typedText && matcherDecl == other.matcherDecl;
  }

  // The text to type to select this matcher.
  std::string typedText;

  // The "declaration" of the matcher, with type information.
  std::string matcherDecl;
};

class RegistryManager {
public:
  RegistryManager() = delete;

  static std::optional<MatcherCtor>
  lookupMatcherCtor(toolchain::StringRef matcherName,
                    const Registry &matcherRegistry);

  static std::vector<ArgKind> getAcceptedCompletionTypes(
      toolchain::ArrayRef<std::pair<MatcherCtor, unsigned>> context);

  static std::vector<MatcherCompletion>
  getMatcherCompletions(ArrayRef<ArgKind> acceptedTypes,
                        const Registry &matcherRegistry);

  static VariantMatcher constructMatcher(MatcherCtor ctor,
                                         internal::SourceRange nameRange,
                                         toolchain::StringRef functionName,
                                         ArrayRef<ParserValue> args,
                                         internal::Diagnostics *error);
};

} // namespace mlir::query::matcher

#endif // MLIR_TOOLS_MLIRQUERY_MATCHER_REGISTRYMANAGER_H
