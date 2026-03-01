//===- TemplatingUtils.h - Templater for text templates -----------------===//
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

#ifndef MLIR_LIB_TARGET_IRDLTOCPP_TEMPLATINGUTILS_H
#define MLIR_LIB_TARGET_IRDLTOCPP_TEMPLATINGUTILS_H

#include "vm/core/ADT/SmallString.h"
#include "vm/core/ADT/StringMap.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
#include <variant>
#include <vector>

namespace mlir::irdl::detail {

/// A dictionary stores a mapping of template variable names to their assigned
/// string values.
using dictionary = toolchain::StringMap<toolchain::SmallString<8>>;

/// Template Code as used by IRDL-to-Cpp.
///
/// For efficiency, produces a bytecode representation of an input template.
///   - LiteralToken: A contiguous stream of characters to be printed
///   - ReplacementToken: A template variable that will be replaced
class Template {
public:
  Template(toolchain::StringRef str) {
    bool processingReplacementToken = false;
    while (!str.empty()) {
      auto [token, remainder] = str.split("__");

      if (processingReplacementToken) {
        assert(!token.empty() && "replacement name cannot be empty");
        bytecode.emplace_back(ReplacementToken{token});
      } else {
        if (!token.empty())
          bytecode.emplace_back(LiteralToken{token});
      }

      processingReplacementToken = !processingReplacementToken;
      str = remainder;
    }
  }

  /// Render will apply a dictionary to the Template and send the rendered
  /// result to the specified output stream.
  void render(toolchain::raw_ostream &out, const dictionary &replacements) const {
    for (auto instruction : bytecode) {
      if (auto *inst = std::get_if<LiteralToken>(&instruction)) {
        out << inst->text;
        continue;
      }

      if (auto *inst = std::get_if<ReplacementToken>(&instruction)) {
        auto replacement = replacements.find(inst->keyName);
#ifndef NDEBUG
        if (replacement == replacements.end()) {
          toolchain::errs() << "Missing template key: " << inst->keyName << "\n";
          llvm_unreachable("Missing template key");
        }
#endif
        out << replacement->second;
        continue;
      }

      llvm_unreachable("non-exhaustive bytecode visit");
    }
  }

private:
  struct LiteralToken {
    toolchain::StringRef text;
  };

  struct ReplacementToken {
    toolchain::StringRef keyName;
  };

  std::vector<std::variant<LiteralToken, ReplacementToken>> bytecode;
};

} // namespace mlir::irdl::detail

#endif // MLIR_LIB_TARGET_IRDLTOCPP_TEMPLATINGUTILS_H
