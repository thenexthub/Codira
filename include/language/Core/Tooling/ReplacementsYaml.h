//===-- ReplacementsYaml.h -- Serialiazation for Replacements ---*- C++ -*-===//
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
/// This file defines the structure of a YAML document for serializing
/// replacements.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_REPLACEMENTSYAML_H
#define LANGUAGE_CORE_TOOLING_REPLACEMENTSYAML_H

#include "language/Core/Tooling/Refactoring.h"
#include "toolchain/Support/YAMLTraits.h"
#include <string>

LLVM_YAML_IS_SEQUENCE_VECTOR(language::Core::tooling::Replacement)

namespace toolchain {
namespace yaml {

/// Specialized MappingTraits to describe how a Replacement is
/// (de)serialized.
template <> struct MappingTraits<language::Core::tooling::Replacement> {
  /// Helper to (de)serialize a Replacement since we don't have direct
  /// access to its data members.
  struct NormalizedReplacement {
    NormalizedReplacement(const IO &) : Offset(0), Length(0) {}

    NormalizedReplacement(const IO &, const language::Core::tooling::Replacement &R)
        : FilePath(R.getFilePath()), Offset(R.getOffset()),
          Length(R.getLength()), ReplacementText(R.getReplacementText()) {}

    language::Core::tooling::Replacement denormalize(const IO &) {
      return language::Core::tooling::Replacement(FilePath, Offset, Length,
                                         ReplacementText);
    }

    std::string FilePath;
    unsigned int Offset;
    unsigned int Length;
    std::string ReplacementText;
  };

  static void mapping(IO &Io, language::Core::tooling::Replacement &R) {
    MappingNormalization<NormalizedReplacement, language::Core::tooling::Replacement>
    Keys(Io, R);
    Io.mapRequired("FilePath", Keys->FilePath);
    Io.mapRequired("Offset", Keys->Offset);
    Io.mapRequired("Length", Keys->Length);
    Io.mapRequired("ReplacementText", Keys->ReplacementText);
  }
};

/// Specialized MappingTraits to describe how a
/// TranslationUnitReplacements is (de)serialized.
template <> struct MappingTraits<language::Core::tooling::TranslationUnitReplacements> {
  static void mapping(IO &Io,
                      language::Core::tooling::TranslationUnitReplacements &Doc) {
    Io.mapRequired("MainSourceFile", Doc.MainSourceFile);
    Io.mapRequired("Replacements", Doc.Replacements);
  }
};
} // end namespace yaml
} // end namespace toolchain

#endif
