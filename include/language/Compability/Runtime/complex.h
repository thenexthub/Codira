//===-- language/Compability/Runtime/complex.h -------------------------*- C++ -*-===//
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

// A single way to expose C++ complex class in files that can be used
// in F18 runtime build. With inclusion of this file std::complex
// and the related names become available, though, they may correspond
// to alternative definitions (e.g. from cuda::std namespace).

#ifndef LANGUAGE_COMPABILITY_RUNTIME_COMPLEX_H
#define LANGUAGE_COMPABILITY_RUNTIME_COMPLEX_H

#include "language/Compability/Common/api-attrs.h"

#if RT_USE_LIBCUDACXX
#include <cuda/std/complex>
#endif

#if RT_USE_LIBCUDACXX && defined(RT_DEVICE_COMPILATION)
namespace language::Compability::runtime::rtcmplx {
using cuda::std::complex;
using cuda::std::conj;
} // namespace language::Compability::runtime::rtcmplx
#else // !(RT_USE_LIBCUDACXX && defined(RT_DEVICE_COMPILATION))
#include <complex>
namespace language::Compability::runtime::rtcmplx {
using std::complex;
using std::conj;
} // namespace language::Compability::runtime::rtcmplx
#endif // !(RT_USE_LIBCUDACXX && defined(RT_DEVICE_COMPILATION))

#endif // FORTRAN_RUNTIME_COMPLEX_H
