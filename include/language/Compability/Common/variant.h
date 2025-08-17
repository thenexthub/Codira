//===-- language/Compability/Common/variant.h --------------------------*- C++ -*-===//
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

// A single way to expose C++ variant class in files that can be used
// in F18 runtime build. With inclusion of this file std::variant
// and the related names become available, though, they may correspond
// to alternative definitions (e.g. from cuda::std namespace).

#ifndef LANGUAGE_COMPABILITY_COMMON_VARIANT_H
#define LANGUAGE_COMPABILITY_COMMON_VARIANT_H

#if RT_USE_LIBCUDACXX
#include <cuda/std/variant>
namespace std {
using cuda::std::get;
using cuda::std::monostate;
using cuda::std::variant;
using cuda::std::variant_size_v;
using cuda::std::visit;
} // namespace std
#else // !RT_USE_LIBCUDACXX
#include <variant>
#endif // !RT_USE_LIBCUDACXX

#endif // FORTRAN_COMMON_VARIANT_H
