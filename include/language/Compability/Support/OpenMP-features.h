//===-- language/Compability/Support/OpenMP-features.h -----------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SUPPORT_OPENMP_FEATURES_H_
#define LANGUAGE_COMPABILITY_SUPPORT_OPENMP_FEATURES_H_

namespace language::Compability::common {

/// Set _OPENMP macro according to given version number
template <typename FortranPredefinitions>
void setOpenMPMacro(int version, FortranPredefinitions &predefinitions) {
  switch (version) {
  case 31:
  default:
    predefinitions.emplace_back("_OPENMP", "201107");
    break;
  case 40:
    predefinitions.emplace_back("_OPENMP", "201307");
    break;
  case 45:
    predefinitions.emplace_back("_OPENMP", "201511");
    break;
  case 50:
    predefinitions.emplace_back("_OPENMP", "201811");
    break;
  case 51:
    predefinitions.emplace_back("_OPENMP", "202011");
    break;
  case 52:
    predefinitions.emplace_back("_OPENMP", "202111");
    break;
  case 60:
    predefinitions.emplace_back("_OPENMP", "202411");
    break;
  }
}
} // namespace language::Compability::common
#endif // FORTRAN_SUPPORT_OPENMP_FEATURES_H_
