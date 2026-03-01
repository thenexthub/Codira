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
#include <type_traits>

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

template <typename Type>
// CC-OFFNXT(huge_method[C++],G.FUN.01-CPP,readability-function-size_parameters) function for testing
static Type Static(ani_env *env, ani_class cls, ani_object i0, ani_byte i1, ani_object i2, ani_short i3, ani_object i4,
                   ani_int i5, ani_object i6, ani_long i7, ani_object i8, ani_byte i9, ani_object i10, ani_short i11,
                   ani_object i12, ani_int i13, ani_object i14, ani_long i15, ani_object f0, ani_float f1,
                   ani_object f2, ani_double f3, ani_object f4, ani_float f5, ani_object f6, ani_double f7,
                   ani_object f8, ani_float f9, ani_object f10, ani_double f11, ani_object f12, ani_float f13,
                   ani_object f14, ani_double f15)
{
    ani_int errorCount = 0L;

    ani_class nativeModule;
    CHECK_EQUAL(env->FindClass("quick_native.NativeModule", &nativeModule), ANI_OK);

    ani_class byteWrapper;
    CHECK_EQUAL(env->FindClass("quick_native.ByteWrapper", &byteWrapper), ANI_OK);
    ani_class shortWrapper;
    CHECK_EQUAL(env->FindClass("quick_native.ShortWrapper", &shortWrapper), ANI_OK);
    ani_class intWrapper;
    CHECK_EQUAL(env->FindClass("quick_native.IntWrapper", &intWrapper), ANI_OK);
    ani_class longWrapper;
    CHECK_EQUAL(env->FindClass("quick_native.LongWrapper", &longWrapper), ANI_OK);
    ani_class floatWrapper;
    CHECK_EQUAL(env->FindClass("quick_native.FloatWrapper", &floatWrapper), ANI_OK);
    ani_class doubleWrapper;
    CHECK_EQUAL(env->FindClass("quick_native.DoubleWrapper", &doubleWrapper), ANI_OK);

    ani_boolean equals;
    CHECK_EQUAL(env->Reference_StrictEquals(cls, nativeModule, &equals), ANI_OK);
    CHECK_EQUAL(equals, true);

    auto checkClass = [&errorCount, env](ani_class correctClass, ani_object object) -> bool {
        ani_type objectClass;
        CHECK_EQUAL(env->Object_GetType(object, &objectClass), ANI_OK);

        ani_boolean check;
        CHECK_EQUAL(env->Reference_StrictEquals(objectClass, correctClass, &check), ANI_OK);
        return check;
    };

    CHECK_EQUAL(checkClass(byteWrapper, i0), true);
    CHECK_EQUAL(checkClass(shortWrapper, i2), true);
    CHECK_EQUAL(checkClass(intWrapper, i4), true);
    CHECK_EQUAL(checkClass(longWrapper, i6), true);
    CHECK_EQUAL(checkClass(byteWrapper, i8), true);
    CHECK_EQUAL(checkClass(shortWrapper, i10), true);
    CHECK_EQUAL(checkClass(intWrapper, i12), true);
    CHECK_EQUAL(checkClass(longWrapper, i14), true);

    CHECK_EQUAL(checkClass(floatWrapper, f0), true);
    CHECK_EQUAL(checkClass(doubleWrapper, f2), true);
    CHECK_EQUAL(checkClass(floatWrapper, f4), true);
    CHECK_EQUAL(checkClass(doubleWrapper, f6), true);
    CHECK_EQUAL(checkClass(floatWrapper, f8), true);
    CHECK_EQUAL(checkClass(doubleWrapper, f10), true);
    CHECK_EQUAL(checkClass(floatWrapper, f12), true);
    CHECK_EQUAL(checkClass(doubleWrapper, f14), true);

    ani_int classMagic;
    CHECK_EQUAL(env->Class_GetStaticFieldByName_Int(cls, "classMagic", &classMagic), ANI_OK);
    CHECK_EQUAL(classMagic, 27384L);

    ani_byte b;
    ani_short s;
    ani_int i;
    ani_long l;
    ani_float f;
    ani_double d;

    CHECK_EQUAL(env->Object_GetFieldByName_Byte(i0, "value", &b), ANI_OK);
    CHECK_EQUAL(b, 0L);
    CHECK_EQUAL(i1, 1L);
    CHECK_EQUAL(env->Object_GetFieldByName_Short(i2, "value", &s), ANI_OK);
    CHECK_EQUAL(s, 2L);
    CHECK_EQUAL(i3, 3L);
    CHECK_EQUAL(env->Object_GetFieldByName_Int(i4, "value", &i), ANI_OK);
    CHECK_EQUAL(i, 4L);
    CHECK_EQUAL(i5, 5L);
    CHECK_EQUAL(env->Object_GetFieldByName_Long(i6, "value", &l), ANI_OK);
    CHECK_EQUAL(l, 6L);
    CHECK_EQUAL(i7, 7L);
    CHECK_EQUAL(env->Object_GetFieldByName_Byte(i8, "value", &b), ANI_OK);
    CHECK_EQUAL(b, 8L);
    CHECK_EQUAL(i9, 9L);
    CHECK_EQUAL(env->Object_GetFieldByName_Short(i10, "value", &s), ANI_OK);
    CHECK_EQUAL(s, 10L);
    CHECK_EQUAL(i11, 11L);
    CHECK_EQUAL(env->Object_GetFieldByName_Int(i12, "value", &i), ANI_OK);
    CHECK_EQUAL(i, 12L);
    CHECK_EQUAL(i13, 13L);
    CHECK_EQUAL(env->Object_GetFieldByName_Long(i14, "value", &l), ANI_OK);
    CHECK_EQUAL(l, 14L);
    CHECK_EQUAL(i15, 15L);

    CHECK_EQUAL(env->Object_GetFieldByName_Float(f0, "value", &f), ANI_OK);
    CHECK_EQUAL(f, 0.0L);
    CHECK_EQUAL(f1, 1.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Double(f2, "value", &d), ANI_OK);
    CHECK_EQUAL(d, 2.0L);
    CHECK_EQUAL(f3, 3.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Float(f4, "value", &f), ANI_OK);
    CHECK_EQUAL(f, 4.0L);
    CHECK_EQUAL(f5, 5.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Double(f6, "value", &d), ANI_OK);
    CHECK_EQUAL(d, 6.0L);
    CHECK_EQUAL(f7, 7.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Float(f8, "value", &f), ANI_OK);
    CHECK_EQUAL(f, 8.0L);
    CHECK_EQUAL(f9, 9.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Double(f10, "value", &d), ANI_OK);
    CHECK_EQUAL(d, 10.0L);
    CHECK_EQUAL(f11, 11.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Float(f12, "value", &f), ANI_OK);
    CHECK_EQUAL(f, 12.0L);
    CHECK_EQUAL(f13, 13.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Double(f14, "value", &d), ANI_OK);
    CHECK_EQUAL(d, 14.0L);
    CHECK_EQUAL(f15, 15.0L);

    if constexpr (std::is_arithmetic_v<Type>) {
        return errorCount;
    } else {
        static_assert(std::is_same_v<Type, ani_object>);
        ani_method ctor;
        CHECK_EQUAL(env->Class_FindMethod(intWrapper, "<ctor>", "i:", &ctor), ANI_OK);
        ani_object newObj;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        CHECK_EQUAL(env->Object_New(intWrapper, ctor, &newObj, errorCount), ANI_OK);
        return newObj;
    }
}

template <typename Type>
// CC-OFFNXT(huge_method[C++],G.FUN.01-CPP,readability-function-size_parameters) function for testing
static Type Virtual(ani_env *env, ani_object that, ani_object i0, ani_byte i1, ani_object i2, ani_short i3,
                    ani_object i4, ani_int i5, ani_object i6, ani_long i7, ani_object i8, ani_byte i9, ani_object i10,
                    ani_short i11, ani_object i12, ani_int i13, ani_object i14, ani_long i15, ani_object f0,
                    ani_float f1, ani_object f2, ani_double f3, ani_object f4, ani_float f5, ani_object f6,
                    ani_double f7, ani_object f8, ani_float f9, ani_object f10, ani_double f11, ani_object f12,
                    ani_float f13, ani_object f14, ani_double f15)
{
    ani_int errorCount = 0L;

    ani_class nativeModule;
    CHECK_EQUAL(env->FindClass("quick_native.NativeModule", &nativeModule), ANI_OK);

    ani_class byteWrapper;
    CHECK_EQUAL(env->FindClass("quick_native.ByteWrapper", &byteWrapper), ANI_OK);
    ani_class shortWrapper;
    CHECK_EQUAL(env->FindClass("quick_native.ShortWrapper", &shortWrapper), ANI_OK);
    ani_class intWrapper;
    CHECK_EQUAL(env->FindClass("quick_native.IntWrapper", &intWrapper), ANI_OK);
    ani_class longWrapper;
    CHECK_EQUAL(env->FindClass("quick_native.LongWrapper", &longWrapper), ANI_OK);
    ani_class floatWrapper;
    CHECK_EQUAL(env->FindClass("quick_native.FloatWrapper", &floatWrapper), ANI_OK);
    ani_class doubleWrapper;
    CHECK_EQUAL(env->FindClass("quick_native.DoubleWrapper", &doubleWrapper), ANI_OK);

    auto checkClass = [&errorCount, env](ani_class correctClass, ani_object object) -> bool {
        ani_type objectClass;
        CHECK_EQUAL(env->Object_GetType(object, &objectClass), ANI_OK);

        ani_boolean check;
        CHECK_EQUAL(env->Reference_StrictEquals(objectClass, correctClass, &check), ANI_OK);
        return check;
    };

    CHECK_EQUAL(checkClass(nativeModule, that), true);

    CHECK_EQUAL(checkClass(byteWrapper, i0), true);
    CHECK_EQUAL(checkClass(shortWrapper, i2), true);
    CHECK_EQUAL(checkClass(intWrapper, i4), true);
    CHECK_EQUAL(checkClass(longWrapper, i6), true);
    CHECK_EQUAL(checkClass(byteWrapper, i8), true);
    CHECK_EQUAL(checkClass(shortWrapper, i10), true);
    CHECK_EQUAL(checkClass(intWrapper, i12), true);
    CHECK_EQUAL(checkClass(longWrapper, i14), true);

    CHECK_EQUAL(checkClass(floatWrapper, f0), true);
    CHECK_EQUAL(checkClass(doubleWrapper, f2), true);
    CHECK_EQUAL(checkClass(floatWrapper, f4), true);
    CHECK_EQUAL(checkClass(doubleWrapper, f6), true);
    CHECK_EQUAL(checkClass(floatWrapper, f8), true);
    CHECK_EQUAL(checkClass(doubleWrapper, f10), true);
    CHECK_EQUAL(checkClass(floatWrapper, f12), true);
    CHECK_EQUAL(checkClass(doubleWrapper, f14), true);

    ani_int magic;
    CHECK_EQUAL(env->Object_GetFieldByName_Int(that, "magic", &magic), ANI_OK);
    CHECK_EQUAL(magic, 42378L);

    ani_byte b;
    ani_short s;
    ani_int i;
    ani_long l;
    ani_float f;
    ani_double d;

    CHECK_EQUAL(env->Object_GetFieldByName_Byte(i0, "value", &b), ANI_OK);
    CHECK_EQUAL(b, 0L);
    CHECK_EQUAL(i1, 1L);
    CHECK_EQUAL(env->Object_GetFieldByName_Short(i2, "value", &s), ANI_OK);
    CHECK_EQUAL(s, 2L);
    CHECK_EQUAL(i3, 3L);
    CHECK_EQUAL(env->Object_GetFieldByName_Int(i4, "value", &i), ANI_OK);
    CHECK_EQUAL(i, 4L);
    CHECK_EQUAL(i5, 5L);
    CHECK_EQUAL(env->Object_GetFieldByName_Long(i6, "value", &l), ANI_OK);
    CHECK_EQUAL(l, 6L);
    CHECK_EQUAL(i7, 7L);
    CHECK_EQUAL(env->Object_GetFieldByName_Byte(i8, "value", &b), ANI_OK);
    CHECK_EQUAL(b, 8L);
    CHECK_EQUAL(i9, 9L);
    CHECK_EQUAL(env->Object_GetFieldByName_Short(i10, "value", &s), ANI_OK);
    CHECK_EQUAL(s, 10L);
    CHECK_EQUAL(i11, 11L);
    CHECK_EQUAL(env->Object_GetFieldByName_Int(i12, "value", &i), ANI_OK);
    CHECK_EQUAL(i, 12L);
    CHECK_EQUAL(i13, 13L);
    CHECK_EQUAL(env->Object_GetFieldByName_Long(i14, "value", &l), ANI_OK);
    CHECK_EQUAL(l, 14L);
    CHECK_EQUAL(i15, 15L);

    CHECK_EQUAL(env->Object_GetFieldByName_Float(f0, "value", &f), ANI_OK);
    CHECK_EQUAL(f, 0.0L);
    CHECK_EQUAL(f1, 1.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Double(f2, "value", &d), ANI_OK);
    CHECK_EQUAL(d, 2.0L);
    CHECK_EQUAL(f3, 3.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Float(f4, "value", &f), ANI_OK);
    CHECK_EQUAL(f, 4.0L);
    CHECK_EQUAL(f5, 5.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Double(f6, "value", &d), ANI_OK);
    CHECK_EQUAL(d, 6.0L);
    CHECK_EQUAL(f7, 7.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Float(f8, "value", &f), ANI_OK);
    CHECK_EQUAL(f, 8.0L);
    CHECK_EQUAL(f9, 9.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Double(f10, "value", &d), ANI_OK);
    CHECK_EQUAL(d, 10.0L);
    CHECK_EQUAL(f11, 11.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Float(f12, "value", &f), ANI_OK);
    CHECK_EQUAL(f, 12.0L);
    CHECK_EQUAL(f13, 13.0L);
    CHECK_EQUAL(env->Object_GetFieldByName_Double(f14, "value", &d), ANI_OK);
    CHECK_EQUAL(d, 14.0L);
    CHECK_EQUAL(f15, 15.0L);

    if constexpr (std::is_arithmetic_v<Type>) {
        return errorCount;
    } else {
        static_assert(std::is_same_v<Type, ani_object>);
        ani_method ctor;
        CHECK_EQUAL(env->Class_FindMethod(intWrapper, "<ctor>", "i:", &ctor), ANI_OK);
        ani_object newObj;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        CHECK_EQUAL(env->Object_New(intWrapper, ctor, &newObj, errorCount), ANI_OK);
        return newObj;
    }
}

static ani_int ModuleLevelFunc([[maybe_unused]] ani_env *env, ani_int i)
{
    return i;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    static const char *className = "quick_native.NativeModule";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        std::cerr << "Not found '" << className << "'" << std::endl;
        return ANI_ERROR;
    }

    std::array methods = {
        ani_native_function {"generalVirtualFinalPrim", nullptr, reinterpret_cast<void *>(Virtual<ani_int>)},
        ani_native_function {"quickVirtualFinalPrim", nullptr, reinterpret_cast<void *>(Virtual<ani_int>)},
        ani_native_function {"generalVirtualFinalObj", nullptr, reinterpret_cast<void *>(Virtual<ani_object>)},
        ani_native_function {"quickVirtualFinalObj", nullptr, reinterpret_cast<void *>(Virtual<ani_object>)},
    };

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        std::cerr << "Cannot bind native methods to '" << className << "'" << std::endl;
        return ANI_ERROR;
    };

    std::array staticMethods = {
        ani_native_function {"generalStaticPrim", nullptr, reinterpret_cast<void *>(Static<ani_int>)},
        ani_native_function {"quickStaticPrim", nullptr, reinterpret_cast<void *>(Static<ani_int>)},
        ani_native_function {"generalStaticObj", nullptr, reinterpret_cast<void *>(Static<ani_object>)},
        ani_native_function {"quickStaticObj", nullptr, reinterpret_cast<void *>(Static<ani_object>)},
    };

    if (ANI_OK != env->Class_BindStaticNativeMethods(cls, staticMethods.data(), staticMethods.size())) {
        std::cerr << "Cannot bind static native methods to '" << className << "'" << std::endl;
        return ANI_ERROR;
    };

    static const char *moduleName = "quick_native";
    ani_module module;
    if (ANI_OK != env->FindModule(moduleName, &module)) {
        std::cerr << "Module '" << moduleName << "' not found\n";
        return ANI_ERROR;
    }

    std::array functions = {
        ani_native_function {"generalModuleLevelFunc", nullptr, reinterpret_cast<void *>(ModuleLevelFunc)},
        ani_native_function {"quickModuleLevelFunc", nullptr, reinterpret_cast<void *>(ModuleLevelFunc)},
    };

    if (ANI_OK != env->Module_BindNativeFunctions(module, functions.data(), functions.size())) {
        std::cerr << "Cannot bind native functions to '" << moduleName << "'\n";
        return ANI_ERROR;
    }

    *result = ANI_VERSION_1;
    return ANI_OK;
}

// NOLINTEND(readability-magic-numbers)
