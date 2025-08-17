//===-- language/Compability/Runtime/c-or-cpp.h ------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_RUNTIME_C_OR_CPP_H_
#define LANGUAGE_COMPABILITY_RUNTIME_C_OR_CPP_H_

#ifdef __cplusplus
#define IF_CPLUSPLUS(x) x
#define IF_NOT_CPLUSPLUS(x)
#define DEFAULT_VALUE(x) = (x)
#define RESTRICT __restrict
#else
#include <stdbool.h>
#define IF_CPLUSPLUS(x)
#define IF_NOT_CPLUSPLUS(x) x
#define DEFAULT_VALUE(x)
#define RESTRICT restrict
#endif

#define LANGUAGE_COMPABILITY_EXTERN_C_BEGIN IF_CPLUSPLUS(extern "C" {)
#define LANGUAGE_COMPABILITY_EXTERN_C_END IF_CPLUSPLUS( \
  })
#define NORETURN IF_CPLUSPLUS([[noreturn]])
#define NO_ARGUMENTS IF_NOT_CPLUSPLUS(void)

#endif // FORTRAN_RUNTIME_C_OR_CPP_H_
