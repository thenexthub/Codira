//===-- language/Compability/Evaluate/intrinsics-library.h -------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_EVALUATE_INTRINSICS_LIBRARY_H_
#define LANGUAGE_COMPABILITY_EVALUATE_INTRINSICS_LIBRARY_H_

// Defines structures to be used in F18 for folding intrinsic function with host
// runtime libraries.

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace language::Compability::evaluate {
class FoldingContext;
class DynamicType;
struct SomeType;
template <typename> class Expr;

// Define a callable type that is used to fold scalar intrinsic function using
// host runtime. These callables are responsible for the conversions between
// host types and Fortran abstract types (Scalar<T>). They also deal with
// floating point environment (To set it up to match the Fortran compiling
// options and to clean it up after the call). Floating point errors are
// reported to the FoldingContext. For 16bits float types, 32bits float host
// runtime plus conversions may be used to build the host wrappers if no 16bits
// runtime is available. IEEE 128bits float may also be used for x87 float.
// Potential conversion overflows are reported by the HostRuntimeWrapper in the
// FoldingContext.
using HostRuntimeWrapper = std::function<Expr<SomeType>(
    FoldingContext &, std::vector<Expr<SomeType>> &&)>;

// Returns the folder using host runtime given the intrinsic function name,
// result and argument types. Nullopt if no host runtime is available for such
// intrinsic function.
std::optional<HostRuntimeWrapper> GetHostRuntimeWrapper(const std::string &name,
    DynamicType resultType, const std::vector<DynamicType> &argTypes);
} // namespace language::Compability::evaluate
#endif // FORTRAN_EVALUATE_INTRINSICS_LIBRARY_H_
