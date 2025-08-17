//===-- EnvironmentDefaults.h -----------------------------------*- C++ -*-===//
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

// EnvironmentDefaults is a list of default values for environment variables
// that may be specified at compile time and set by the runtime during
// program startup if the variable is not already present in the environment.
// EnvironmentDefaults is intended to allow options controlled by environment
// variables to also be set on the command line at compile time without needing
// to define option-specific runtime calls or duplicate logic within the
// runtime. For example, the -fconvert command line option is implemented in
// terms of an default value for the FORT_CONVERT environment variable.

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_ENVIRONMENTDEFAULTS_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_ENVIRONMENTDEFAULTS_H

#include <vector>

namespace fir {
class FirOpBuilder;
class GlobalOp;
} // namespace fir

namespace mlir {
class Location;
class Value;
} // namespace mlir

namespace language::Compability::lower {
struct EnvironmentDefault;
} // namespace language::Compability::lower

namespace fir::runtime {

/// Create the list of environment variable defaults for the runtime to set. The
/// form of the generated list is defined in the runtime header file
/// environment-default-list.h
mlir::Value genEnvironmentDefaults(
    fir::FirOpBuilder &builder, mlir::Location loc,
    const std::vector<language::Compability::lower::EnvironmentDefault> &envDefaults);

} // namespace fir::runtime
#endif // FORTRAN_OPTIMIZER_BUILDER_RUNTIME_ENVIRONMENTDEFAULTS_H
