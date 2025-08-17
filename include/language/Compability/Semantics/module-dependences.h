//===-- language/Compability/Semantics/module-dependences.h ------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_MODULE_DEPENDENCES_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_MODULE_DEPENDENCES_H_

#include <cinttypes>
#include <map>
#include <optional>
#include <string>

namespace language::Compability::semantics {

using ModuleCheckSumType = std::uint64_t;

class ModuleDependences {
public:
  void AddDependence(
      std::string &&name, bool intrinsic, ModuleCheckSumType hash) {
    if (intrinsic) {
      intrinsicMap_.insert_or_assign(std::move(name), hash);
    } else {
      nonIntrinsicMap_.insert_or_assign(std::move(name), hash);
    }
  }
  std::optional<ModuleCheckSumType> GetRequiredHash(
      const std::string &name, bool intrinsic) {
    if (intrinsic) {
      if (auto iter{intrinsicMap_.find(name)}; iter != intrinsicMap_.end()) {
        return iter->second;
      }
    } else {
      if (auto iter{nonIntrinsicMap_.find(name)};
          iter != nonIntrinsicMap_.end()) {
        return iter->second;
      }
    }
    return std::nullopt;
  }

private:
  std::map<std::string, ModuleCheckSumType> intrinsicMap_, nonIntrinsicMap_;
};

} // namespace language::Compability::semantics
#endif // FORTRAN_SEMANTICS_MODULE_DEPENDENCES_H_
