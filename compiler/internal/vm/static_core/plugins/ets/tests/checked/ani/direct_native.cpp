/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include <array>
#include <iostream>

#include "ani.h"

// NOLINTBEGIN(readability-magic-numbers)

// CC-OFFNXT(G.PRE.02-CPP) testing macro with source line info for finding errors
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_EQUAL(a, b)                                                                                 \
    {                                                                                                     \
        auto aValue = (a);                                                                                \
        auto bValue = (b);                                                                                \
        if (aValue != bValue) {                                                                           \
            ++errorCount;                                                                                 \
            std::cerr << "LINE " << __LINE__ << ", " << #a << ": " << aValue << " != " << bValue << '\n'; \
        }                                                                                                 \
    }

// CC-OFFNXT(G.FUN.01-CPP, readability-function-size_parameters) function for testing
static ani_int Direct(ani_int im1, ani_byte i0, ani_short i1, ani_int i2, ani_long i3, ani_float f0, ani_double f1,
                      ani_double f2, ani_int i4, ani_int i5, ani_int i6, ani_long i7, ani_int i8, ani_short i9,
                      ani_int i10, ani_int i11, ani_int i12, ani_int i13, ani_int i14, ani_float f3, ani_float f4,
                      ani_double f5, ani_double f6, ani_double f7, ani_double f8, ani_double f9, ani_double f10,
                      ani_double f11, ani_double f12, ani_double f13, ani_double f14)
{
    ani_int errorCount = 0L;

    CHECK_EQUAL(im1, -1L);
    CHECK_EQUAL(static_cast<ani_int>(i0), 0L);
    CHECK_EQUAL(i1, 1L);
    CHECK_EQUAL(i2, 2L);
    CHECK_EQUAL(i3, 3L);
    CHECK_EQUAL(i4, 4L);
    CHECK_EQUAL(i5, 5L);
    CHECK_EQUAL(i6, 6L);
    CHECK_EQUAL(i7, 7L);
    CHECK_EQUAL(i8, 8L);
    CHECK_EQUAL(i9, 9L);
    CHECK_EQUAL(i10, 10L);
    CHECK_EQUAL(i11, 11L);
    CHECK_EQUAL(i12, 12L);
    CHECK_EQUAL(i13, 13L);
    CHECK_EQUAL(i14, 14L);

    CHECK_EQUAL(f0, 0.0L);
    CHECK_EQUAL(f1, 1.0L);
    CHECK_EQUAL(f2, 2.0L);
    CHECK_EQUAL(f3, 3.0L);
    CHECK_EQUAL(f4, 4.0L);
    CHECK_EQUAL(f5, 5.0L);
    CHECK_EQUAL(f6, 6.0L);
    CHECK_EQUAL(f7, 7.0L);
    CHECK_EQUAL(f8, 8.0L);
    CHECK_EQUAL(f9, 9.0L);
    CHECK_EQUAL(f10, 10.0L);
    CHECK_EQUAL(f11, 11.0L);
    CHECK_EQUAL(f12, 12.0L);
    CHECK_EQUAL(f13, 13.0L);
    CHECK_EQUAL(f14, 14.0L);

    return errorCount;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    static const char *className = "direct_native.NativeModule";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        std::cerr << "Not found '" << className << "'" << std::endl;
        return ANI_ERROR;
    }

    std::array methods = {
        ani_native_function {"direct", nullptr, reinterpret_cast<void *>(Direct)},
    };

    if (ANI_OK != env->Class_BindStaticNativeMethods(cls, methods.data(), methods.size())) {
        std::cerr << "Cannot bind native methods to '" << className << "'" << std::endl;
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}

// NOLINTEND(readability-magic-numbers)
