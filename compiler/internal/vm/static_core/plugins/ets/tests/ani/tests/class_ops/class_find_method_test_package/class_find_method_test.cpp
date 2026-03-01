/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ani.h>
#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {
class ClassFindMethodTest : public AniTest {
protected:
    static constexpr ani_int TEST_EXPECTED_VALUE1 = 5;
    static constexpr ani_int TEST_EXPECTED_VALUE2 = 4;
    static constexpr ani_int TEST_EXPECTED_VALUE3 = 9;
    static constexpr ani_int TEST_EXPECTED_VALUE4 = 2;
    static constexpr ani_int TEST_EXPECTED_VALUE5 = 6;
    static constexpr ani_int TEST_EXPECTED_VALUE6 = 3;

    static constexpr ani_int TEST_ARG1 = 2;
    static constexpr ani_int TEST_ARG2 = 3;
    static constexpr ani_double TEST_ARG3 = 1.0;
    static constexpr ani_boolean TEST_ARG4 = ANI_TRUE;
    static constexpr ani_float TEST_ARG5 = 3.0;
    static constexpr ani_float TEST_ARG6 = 4.0;

    static constexpr ani_int TEST_NATIVE_PARAM1 = 2;
    static constexpr ani_int TEST_NATIVE_PARAM2 = 3;

    static constexpr std::string_view ARG_STRING = "abab";

private:
    void GetFunc(ani_function *fn, const char *funcName)
    {
        ani_module module;
        ASSERT_EQ(env_->FindModule("test", &module), ANI_OK);
        ASSERT_NE(module, nullptr);

        ASSERT_EQ(env_->Module_FindFunction(module, funcName, nullptr, fn), ANI_OK);
        ASSERT_NE(fn, nullptr);
    }

    void GetObject(ani_object *objectResult, ani_class cls)
    {
        ani_method ctor;
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
        ASSERT_EQ(env_->Object_New(cls, ctor, objectResult), ANI_OK);
    }

public:
    template <bool HAS_METHOD, bool INVALID_DESCRIPTOR = false>
    void CheckClassFindMethod(const char *clsDescriptor, const char *methodName, const char *methodSignature,
                              const ani_value *args = nullptr, ani_int expectedResult = TEST_EXPECTED_VALUE1)
    {
        ani_class cls;
        const std::string className = std::string("test.") + clsDescriptor;
        ASSERT_EQ(env_->FindClass(className.c_str(), &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_method method;
        auto status = env_->Class_FindMethod(cls, methodName, methodSignature, &method);

        if constexpr (!HAS_METHOD) {
            if constexpr (INVALID_DESCRIPTOR) {
                ASSERT_EQ(status, ANI_INVALID_DESCRIPTOR);
            } else {
                ASSERT_EQ(status, ANI_NOT_FOUND);
            }
            return;
        }

        ASSERT_EQ(status, ANI_OK);
        ASSERT_NE(method, nullptr);

        ani_object obj;
        GetObject(&obj, cls);

        ani_int result;
        ASSERT_EQ(env_->Object_CallMethod_Int_A(obj, method, &result, args), ANI_OK);
        std::cout << result << std::endl;
        ASSERT_EQ(result, expectedResult);
    }

    void CheckIntrinsicsFindMethod(const char *moduleDescriptor, const char *clsDescriptor, const char *methodName,
                                   const char *methodSignature)
    {
        ani_class cls;
        const std::string className = std::string(moduleDescriptor) + "." + clsDescriptor;
        ASSERT_EQ(env_->FindClass(className.c_str(), &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_method method;
        auto status = env_->Class_FindMethod(cls, methodName, methodSignature, &method);
        ASSERT_EQ(status, ANI_OK);
    }

    void TupleCreator(ani_ref *result)
    {
        ani_function fn {};
        ani_value args[2U];
        args[0U].i = TEST_ARG1;
        args[1U].l = TEST_ARG2;
        GetFunc(&fn, "CreateTuple");
        env_->Function_Call_Ref_A(fn, result, args);
    }

    void PartialCreator(ani_ref *result, ani_int arg = TEST_ARG1)
    {
        ani_function fn {};
        GetFunc(&fn, "CreatePartial");
        env_->Function_Call_Ref(fn, result, arg);
    }

    void RequiredCreator(ani_ref *result)
    {
        ani_function fn {};
        GetFunc(&fn, "CreateRequired");
        env_->Function_Call_Ref(fn, result, TEST_ARG1);
    }

    void ReadonlyCreator(ani_ref *result)
    {
        ani_function fn {};
        GetFunc(&fn, "CreateReadonly");
        env_->Function_Call_Ref(fn, result, TEST_ARG1);
    }

    void RecordCreator(ani_class *cls, ani_object *result)
    {
        ASSERT_EQ(env_->FindClass("std.core.Record", cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_method ctor;
        ASSERT_EQ(env_->Class_FindMethod(*cls, "<ctor>", ":", &ctor), ANI_OK);

        ASSERT_EQ(env_->Object_New(*cls, ctor, result), ANI_OK);
    }

    void BigintCreator(ani_class *cls, ani_object *result)
    {
        ASSERT_EQ(env_->FindClass("std.core.BigInt", cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_method ctor;
        ASSERT_EQ(env_->Class_FindMethod(*cls, "<ctor>", "i:", &ctor), ANI_OK);

        ani_int arg = TEST_ARG1;

        ASSERT_EQ(env_->Object_New(*cls, ctor, result, arg), ANI_OK);
    }

    void LambdaCreator(ani_ref *result, const char *funcName)
    {
        ani_function fn {};
        GetFunc(&fn, funcName);
        env_->Function_Call_Ref(fn, result);
    }

    void Int(ani_object *obj, ani_int value)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("std.core.Int", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_method ctor;
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &ctor), ANI_OK);

        ASSERT_EQ(env_->Object_New(cls, ctor, obj, value), ANI_OK);
    }

    // NOLINTNEXTLINE(readability-named-parameter)
    static ani_int NativeMethodsFooNative(ani_env *, ani_class)
    {
        const ani_int answer = 42U;
        return answer;
    }
};

TEST_F(ClassFindMethodTest, has_method__A_B)
{
    ani_value args[2U];
    args[0U].i = TEST_ARG1;
    args[1U].i = TEST_ARG2;

    CheckClassFindMethod<true>("A", "int_method", "ii:i", args, TEST_EXPECTED_VALUE1);
    CheckClassFindMethod<true>("A", "int_method", nullptr, args, TEST_EXPECTED_VALUE1);
    CheckClassFindMethod<true>("B", "int_method", "ii:i", args, TEST_EXPECTED_VALUE1);
    CheckClassFindMethod<true>("B", "int_method", nullptr, args, TEST_EXPECTED_VALUE1);

    CheckClassFindMethod<true>("A", "int_override_method", "ii:i", args, TEST_EXPECTED_VALUE2);
    CheckClassFindMethod<true>("A", "int_override_method", nullptr, args, TEST_EXPECTED_VALUE2);

    CheckClassFindMethod<true>("B", "int_override_method", "ii:i", args, TEST_EXPECTED_VALUE3);
    CheckClassFindMethod<true>("B", "int_override_method", nullptr, args, TEST_EXPECTED_VALUE3);
}

TEST_F(ClassFindMethodTest, has_method_C)
{
    ani_value arg;
    arg.i = TEST_ARG1;

    CheckClassFindMethod<true>("C", "imethod", "i:i", &arg, TEST_EXPECTED_VALUE4);

    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8(ARG_STRING.data(), ARG_STRING.size(), &string), ANI_OK);
    arg.r = string;

    CheckClassFindMethod<true>("C", "imethod", "C{std.core.String}:i", &arg, TEST_EXPECTED_VALUE2);
}

TEST_F(ClassFindMethodTest, has_method_C_unusual_types)
{
    ani_value args;
    ani_string string {};
    {
        ASSERT_EQ(env_->String_NewUTF8(ARG_STRING.data(), ARG_STRING.size(), &string), ANI_OK);
        args.r = string;

        CheckClassFindMethod<true>("C", "imethod_optional", nullptr, &args, TEST_EXPECTED_VALUE2);

        env_->GetUndefined(&args.r);
        CheckClassFindMethod<true>("C", "imethod_optional", nullptr, &args, TEST_EXPECTED_VALUE1);

        TupleCreator(&args.r);
        CheckClassFindMethod<true>("C", "imethod_tuple", nullptr, &args, TEST_EXPECTED_VALUE1);
    }
    {
        ani_fixedarray_int params;
        ASSERT_EQ(env_->FixedArray_New_Int(2U, &params), ANI_OK);
        ani_int nativeParams[] = {TEST_NATIVE_PARAM1, TEST_NATIVE_PARAM2};
        const ani_size offset = 0;
        ASSERT_EQ(env_->FixedArray_SetRegion_Int(params, offset, 2U, nativeParams), ANI_OK);

        args.r = params;
        CheckClassFindMethod<true>("C", "method_rest", nullptr, &args, TEST_EXPECTED_VALUE1);
    }
    {
        args.r = string;
        CheckClassFindMethod<true>("C", "method_union", nullptr, &args, TEST_EXPECTED_VALUE2);
    }
    {
        ani_object intValue;
        Int(&intValue, TEST_ARG1);
        args.r = intValue;
        CheckClassFindMethod<true>("C", "method_union", nullptr, &args, TEST_EXPECTED_VALUE4);
    }
}

TEST_F(ClassFindMethodTest, has_method_Derived)
{
    ani_value args;
    args.i = TEST_ARG1;
    CheckClassFindMethod<true>("Derived", "abstract_method", nullptr, &args, TEST_EXPECTED_VALUE4);

    CheckClassFindMethod<true>("Derived", "method", nullptr, &args, TEST_EXPECTED_VALUE2);
}

TEST_F(ClassFindMethodTest, has_method_Overload)
{
    ani_value args;
    args.i = TEST_ARG1;
    CheckClassFindMethod<true>("Overload", "method", "i:i", &args, TEST_EXPECTED_VALUE4);

    ani_value params[2U];
    params[0U].i = TEST_ARG1;
    params[1U].z = TEST_ARG4;
    CheckClassFindMethod<true>("Overload", "method", "iz:i", params, TEST_EXPECTED_VALUE2);

    params[1U].d = TEST_ARG3;
    CheckClassFindMethod<true>("Overload", "method", "id:i", params, TEST_EXPECTED_VALUE5);
}

TEST_F(ClassFindMethodTest, special_types1)
{
    ani_value args;
    {
        PartialCreator(&args.r);
        CheckClassFindMethod<true>("SpecialTypes", "partial_method", nullptr, &args, TEST_EXPECTED_VALUE1);
        PartialCreator(&args.r, TEST_ARG2);
        CheckClassFindMethod<true>("SpecialTypes", "partial_method", nullptr, &args, TEST_EXPECTED_VALUE6);
    }
    {
        RequiredCreator(&args.r);
        CheckClassFindMethod<true>("SpecialTypes", "required_method", nullptr, &args, TEST_EXPECTED_VALUE4);

        ReadonlyCreator(&args.r);
        CheckClassFindMethod<true>("SpecialTypes", "readonly_method", nullptr, &args, TEST_EXPECTED_VALUE4);
    }
    {
        ani_class cls;
        ani_object mapValue;
        RecordCreator(&cls, &mapValue);
        ani_method setter;
        ASSERT_EQ(env_->Class_FindIndexableSetter(cls, nullptr, &setter), ANI_OK);

        ani_string string {};
        ASSERT_EQ(env_->String_NewUTF8(ARG_STRING.data(), ARG_STRING.size(), &string), ANI_OK);

        ani_object intValue;
        Int(&intValue, TEST_ARG1);
        ASSERT_EQ(env_->Object_CallMethod_Void(mapValue, setter, string, intValue), ANI_OK);

        args.r = mapValue;
        CheckClassFindMethod<true>("SpecialTypes", "record_method", nullptr, &args, TEST_EXPECTED_VALUE4);
    }
}

TEST_F(ClassFindMethodTest, special_types2)
{
    ani_value args;

    // NOTE(ypigunova) change native args. Issue #23595
    // NOTE(daizihan) remove this test commented out, since the arg was not correct, need wait #23595
    {
        env_->GetUndefined(&args.r);
        CheckClassFindMethod<true>("SpecialTypes", "null_method", nullptr, &args, TEST_EXPECTED_VALUE1);
    }
    {
        ani_string string {};
        ASSERT_EQ(env_->String_NewUTF8(ARG_STRING.data(), ARG_STRING.size(), &string), ANI_OK);

        args.r = string;
        CheckClassFindMethod<true>("SpecialTypes", "string_literal_method", nullptr, &args, TEST_EXPECTED_VALUE2);
    }
    {
        ani_class cls;
        ani_object bigIntValue;
        BigintCreator(&cls, &bigIntValue);
        args.r = bigIntValue;
        CheckClassFindMethod<true>("SpecialTypes", "bigint_method", nullptr, &args, TEST_EXPECTED_VALUE4);
    }
}

TEST_F(ClassFindMethodTest, lambdas)
{
    ani_value arg;

    {
        LambdaCreator(&arg.r, "LambdaOneArg");
        CheckClassFindMethod<true>("LambdaTypes", "one_arg_method", nullptr, &arg, TEST_EXPECTED_VALUE1);
    }
    {
        LambdaCreator(&arg.r, "LambdaTwoArgs");
        CheckClassFindMethod<true>("LambdaTypes", "two_arg_method", nullptr, &arg, TEST_EXPECTED_VALUE6);
    }
    {
        LambdaCreator(&arg.r, "LambdaOptArg");
        CheckClassFindMethod<true>("LambdaTypes", "opt_arg_method", nullptr, &arg, TEST_EXPECTED_VALUE4);
    }
}

TEST_F(ClassFindMethodTest, generics)
{
    ani_value arg;

    ani_object intValue;
    Int(&intValue, TEST_ARG1);
    arg.r = intValue;
    CheckClassFindMethod<true>("Generic0", "method", nullptr, &arg, TEST_EXPECTED_VALUE1);

    Int(&intValue, TEST_ARG1);
    arg.r = intValue;
    CheckClassFindMethod<true>("Generic1", "method", nullptr, &arg, TEST_EXPECTED_VALUE4);
}

TEST_F(ClassFindMethodTest, binded_method)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("test.FindNativeMethods", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    std::array methods = {
        ani_native_function {"foo", ":i", reinterpret_cast<void *>(NativeMethodsFooNative)},
    };
    ASSERT_EQ(env_->Class_BindNativeMethods(cls, methods.data(), methods.size()), ANI_OK);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", ":i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ClassFindMethodTest, FindOverloadedConstructor)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.FindOverloadedConstructor", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &ctor), ANI_AMBIGUOUS);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "i:", &ctor), ANI_OK);

    ASSERT_EQ(env_->FindClass("test.FindConstructorWithDefaultParams", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &ctor), ANI_AMBIGUOUS);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C{std.core.Int}:", &ctor), ANI_OK);
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C{std.core.Int}C{std.core.Int}:", &ctor), ANI_OK);
}

TEST_F(ClassFindMethodTest, find_intrinsics)
{
    {
        const char *moduleName = "std.core";
        const char *className = "String";

        CheckIntrinsicsFindMethod(moduleName, className, "compareTo", "C{std.core.String}:i");
        CheckIntrinsicsFindMethod(moduleName, className, "repeat", "i:C{std.core.String}");
    }
    {
        const char *moduleName = "std.core";
        const char *className = "ArrayBuffer";

        CheckIntrinsicsFindMethod(moduleName, className, "set", "ib:");
        CheckIntrinsicsFindMethod(moduleName, className, "setValues", "C{std.core.ArrayBuffer}i:");
    }
    {
        const char *moduleName = "std.core";
        const char *className = "Class";

        CheckIntrinsicsFindMethod(moduleName, className, "createInstance", ":C{std.core.Object}");
        CheckIntrinsicsFindMethod(moduleName, className, "getDescriptor", ":C{std.core.String}");
    }
    {
        const char *moduleName = "std.core";
        const char *className = "ArrayBuffer";

        CheckIntrinsicsFindMethod(moduleName, className, "atomicAndI32", "iii:l");
        CheckIntrinsicsFindMethod(moduleName, className, "atomicXorI32", "iii:l");
    }
}

TEST_F(ClassFindMethodTest, method_not_found)
{
    CheckClassFindMethod<false>("A", "bla_bla_bla", nullptr);
    CheckClassFindMethod<false, true>("A", "int_method", "bla_bla_bla");
    CheckClassFindMethod<false>("Overload", "method", "dd:i");
    CheckClassFindMethod<false>("Overload", "method", "iC{std.core.String}:i");
}

TEST_F(ClassFindMethodTest, invalid_argument_name)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass(nullptr, &cls), ANI_INVALID_ARGS);

    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(cls, nullptr, nullptr, &method), ANI_INVALID_ARGS);
}

TEST_F(ClassFindMethodTest, invalid_argument_cls)
{
    ani_method method;
    ASSERT_EQ(env_->Class_FindMethod(nullptr, "int_method", nullptr, &method), ANI_INVALID_ARGS);
}

TEST_F(ClassFindMethodTest, invalid_arguments)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.A", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "int_method", nullptr, nullptr), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->c_api->Class_FindMethod(nullptr, cls, "int_method", nullptr, &method), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Class_FindMethod(cls, "", "ii:i", &method), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_FindMethod(cls, "\t", "ii:i", &method), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Class_FindMethod(cls, "int_method", "", &method), ANI_INVALID_DESCRIPTOR);
    ASSERT_EQ(env_->Class_FindMethod(cls, "int_method", "\t", &method), ANI_INVALID_DESCRIPTOR);
}

TEST_F(ClassFindMethodTest, has_static_method_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.Phone", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "get_button_names", ":C{std.core.Array}", &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ClassFindMethodTest, has_static_method_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.Phone", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "get_button_names", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ClassFindMethodTest, static_method_not_found_1)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.Phone", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "bla_bla_bla", nullptr, &method), ANI_NOT_FOUND);
}

TEST_F(ClassFindMethodTest, static_method_not_found_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.Phone", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "get_button_names", "bla_bla_bla", &method), ANI_INVALID_DESCRIPTOR);
}

TEST_F(ClassFindMethodTest, static_method_invalid_argument_name)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.Phone", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, nullptr, nullptr, &method), ANI_INVALID_ARGS);
}

TEST_F(ClassFindMethodTest, static_method_invalid_argument_cls)
{
    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(nullptr, "get_button_names", nullptr, &method), ANI_INVALID_ARGS);
}

TEST_F(ClassFindMethodTest, static_method_invalid_arguments)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.Phone", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "get_button_names", nullptr, nullptr), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->c_api->Class_FindStaticMethod(nullptr, cls, "get_button_names", nullptr, &method),
              ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "", ":A{C{std.core.String}}", &method), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "\t", ":A{C{std.core.String}}", &method), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "get_button_names", "", &method), ANI_INVALID_DESCRIPTOR);
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "get_button_names", "\t", &method), ANI_INVALID_DESCRIPTOR);
}

TEST_F(ClassFindMethodTest, static_method_find_static_method_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.Operations", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "sum", "ii:i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_int sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, method, &sum, TEST_EXPECTED_VALUE1, TEST_EXPECTED_VALUE5), ANI_OK);
    ASSERT_EQ(sum, TEST_EXPECTED_VALUE1 + TEST_EXPECTED_VALUE5);
}

TEST_F(ClassFindMethodTest, static_method_find_static_method_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.ClassA", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method methodA {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "calculateSquareArea", "ii:i", &methodA), ANI_OK);
    ASSERT_NE(methodA, nullptr);

    ani_static_method methodB {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "calculateSquareArea", "iii:i", &methodB), ANI_OK);
    ASSERT_NE(methodB, nullptr);

    ani_int sumA = 0;
    ani_value args[2U];
    args[0U].i = TEST_EXPECTED_VALUE1;
    args[1U].i = TEST_EXPECTED_VALUE5;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, methodA, &sumA, args), ANI_OK);
    ASSERT_EQ(sumA, TEST_EXPECTED_VALUE1 * TEST_EXPECTED_VALUE5);

    ani_int sumB = 0;
    ani_value argsB[TEST_EXPECTED_VALUE6];
    argsB[0U].i = TEST_EXPECTED_VALUE1;
    argsB[1U].i = TEST_EXPECTED_VALUE1;
    argsB[2U].i = TEST_EXPECTED_VALUE5;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, methodB, &sumB, argsB), ANI_OK);
    ASSERT_EQ(sumB, TEST_EXPECTED_VALUE1 * TEST_EXPECTED_VALUE1 * TEST_EXPECTED_VALUE5);
}

TEST_F(ClassFindMethodTest, static_method_find_static_method_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.ClassA", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method methodB {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "sumB", "iii:i", &methodB), ANI_OK);
    ASSERT_NE(methodB, nullptr);

    ani_int sumB = 0;
    ani_value argsB[TEST_EXPECTED_VALUE6];
    argsB[0].i = TEST_EXPECTED_VALUE1;
    argsB[1].i = TEST_EXPECTED_VALUE1;
    argsB[2U].i = TEST_EXPECTED_VALUE5;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, methodB, &sumB, argsB), ANI_OK);
    ASSERT_EQ(sumB, TEST_EXPECTED_VALUE1 + TEST_EXPECTED_VALUE1 + TEST_EXPECTED_VALUE5);
}

TEST_F(ClassFindMethodTest, static_method_find_static_method_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.ClassB", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method methodA {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "sumA", "ii:i", &methodA), ANI_OK);
    ASSERT_NE(methodA, nullptr);

    ani_int sumB = 0;
    ani_value args[2U];
    args[0].i = TEST_EXPECTED_VALUE1;
    args[1].i = TEST_EXPECTED_VALUE5;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, methodA, &sumB, args), ANI_OK);
    ASSERT_EQ(sumB, TEST_EXPECTED_VALUE1 + TEST_EXPECTED_VALUE5);
}

TEST_F(ClassFindMethodTest, static_method_find_static_method_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.ClassA", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method methodA {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "calculateSquareArea", "ii:i", &methodA), ANI_OK);
    ASSERT_NE(methodA, nullptr);

    ani_int sumA = 0;
    ani_value args[2U];
    args[0].i = TEST_EXPECTED_VALUE1;
    args[1].i = TEST_EXPECTED_VALUE5;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, methodA, &sumA, args), ANI_OK);
    ASSERT_EQ(sumA, TEST_EXPECTED_VALUE1 * TEST_EXPECTED_VALUE5);
}

TEST_F(ClassFindMethodTest, static_method_find_static_method_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.BasicCalculator", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method staticMethod {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "multiply", "ii:i", &staticMethod), ANI_OK);
    ASSERT_NE(staticMethod, nullptr);
    ani_int multiply = 0;
    ASSERT_EQ(
        env_->Class_CallStaticMethod_Int(cls, staticMethod, &multiply, TEST_EXPECTED_VALUE1, TEST_EXPECTED_VALUE5),
        ANI_OK);
    ASSERT_EQ(multiply, TEST_EXPECTED_VALUE1 * TEST_EXPECTED_VALUE5);

    ani_class subCls {};
    ASSERT_EQ(env_->FindClass("test.SubCalculator", &subCls), ANI_OK);
    ASSERT_NE(subCls, nullptr);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_method addNormalMethod {};
    ASSERT_EQ(env_->Class_FindMethod(subCls, "add", nullptr, &addNormalMethod), ANI_OK);
    ASSERT_NE(addNormalMethod, nullptr);

    ani_object object1 {};
    ASSERT_EQ(env_->Object_New(subCls, constructorMethod, &object1, 5U, 6U), ANI_OK);
    ASSERT_NE(object1, nullptr);
    ani_int result = 0;
    ani_value args[2U];
    args[0U].i = TEST_EXPECTED_VALUE1;
    args[1U].i = TEST_EXPECTED_VALUE5;
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object1, addNormalMethod, &result, args), ANI_OK);
    ASSERT_EQ(result, TEST_EXPECTED_VALUE1 + TEST_EXPECTED_VALUE5);

    ani_static_method subStaticMethod {};
    ASSERT_EQ(env_->Class_FindStaticMethod(subCls, "divide", nullptr, &subStaticMethod), ANI_OK);
    ASSERT_NE(subStaticMethod, nullptr);

    ani_int divideResult = 0;
    ani_value args2[2U];
    args2[0U].i = TEST_EXPECTED_VALUE5;
    args2[1U].i = TEST_EXPECTED_VALUE6;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, subStaticMethod, &divideResult, args2), ANI_OK);
    ASSERT_EQ(divideResult, TEST_EXPECTED_VALUE5 / TEST_EXPECTED_VALUE6);
}

TEST_F(ClassFindMethodTest, find_method_combine_scenes_002)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.test002A.test002B.TestA002", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "sum", "ii:i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, method, &object, TEST_EXPECTED_VALUE1, TEST_EXPECTED_VALUE5), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_value args[2U];
    ani_int arg1 = TEST_EXPECTED_VALUE4;
    ani_int arg2 = TEST_EXPECTED_VALUE6;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_int sum = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, arg1 + arg2);
}

TEST_F(ClassFindMethodTest, find_method_combine_scenes_003)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.TestA003", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_method publicMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "int_method", nullptr, &publicMethod), ANI_OK);
    ASSERT_NE(publicMethod, nullptr);

    ani_method protectedMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "float_method", nullptr, &protectedMethod), ANI_OK);
    ASSERT_NE(protectedMethod, nullptr);

    ani_method privateMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "bool_method", nullptr, &privateMethod), ANI_OK);
    ASSERT_NE(privateMethod, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object, TEST_EXPECTED_VALUE1, TEST_EXPECTED_VALUE5), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_value args[2U];
    ani_int arg1 = TEST_EXPECTED_VALUE4;
    ani_int arg2 = TEST_EXPECTED_VALUE6;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_int publicResult = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, publicMethod, &publicResult, args), ANI_OK);
    ASSERT_EQ(publicResult, arg1 * arg2);

    ani_value args2[2U];
    ani_float arg3 = TEST_ARG5;
    ani_float arg4 = TEST_ARG6;
    args2[0U].f = arg3;
    args2[1U].f = arg4;
    ani_float protectedResult = 0.F;
    ASSERT_EQ(env_->Object_CallMethod_Float_A(object, protectedMethod, &protectedResult, args2), ANI_OK);
    ASSERT_FLOAT_EQ(protectedResult, arg3 + arg4);

    ani_boolean privateResult = ANI_FALSE;
    ani_value args3[2U];
    args3[0U].z = ANI_TRUE;
    args3[1U].z = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, privateMethod, &privateResult, args3), ANI_OK);
    ASSERT_EQ(privateResult, ANI_TRUE);
}

TEST_F(ClassFindMethodTest, find_method_combine_scenes_004)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.TestA004", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_method funcAMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "int_method", nullptr, &funcAMethod), ANI_OK);
    ASSERT_NE(funcAMethod, nullptr);

    ani_method funcBMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "float_method", nullptr, &funcBMethod), ANI_OK);
    ASSERT_NE(funcBMethod, nullptr);

    ani_method funcCMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "bool_method", nullptr, &funcCMethod), ANI_OK);
    ASSERT_NE(funcCMethod, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object, TEST_EXPECTED_VALUE1, TEST_EXPECTED_VALUE5), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_value args[2U];
    ani_int arg1 = TEST_EXPECTED_VALUE4;
    ani_int arg2 = TEST_EXPECTED_VALUE6;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_int funAResult = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, funcAMethod, &funAResult, args), ANI_OK);
    ASSERT_EQ(funAResult, arg1 * arg2);

    ani_value args2[2U];
    ani_float arg3 = TEST_ARG5;
    ani_float arg4 = TEST_ARG6;
    args2[0U].f = arg3;
    args2[1U].f = arg4;
    ani_float funBResult = 0.F;
    ASSERT_EQ(env_->Object_CallMethod_Float_A(object, funcBMethod, &funBResult, args2), ANI_OK);
    ASSERT_FLOAT_EQ(funBResult, arg3 + arg4);

    ani_boolean funCResult = ANI_FALSE;
    ani_value args3[2U];
    args3[0U].z = ANI_TRUE;
    args3[1U].z = ANI_FALSE;
    ASSERT_EQ(env_->Object_CallMethod_Boolean_A(object, funcCMethod, &funCResult, args3), ANI_OK);
    ASSERT_EQ(funCResult, ANI_TRUE);
}

TEST_F(ClassFindMethodTest, find_method_combine_scenes_005)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.TestA005", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "int_method", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_value args[2U];
    ani_int arg1 = TEST_EXPECTED_VALUE4;
    ani_int arg2 = TEST_EXPECTED_VALUE6;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_int result = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, arg1 + arg2);
}

TEST_F(ClassFindMethodTest, find_method_combine_scenes_006)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.Teen", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "add_method", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_value args[2U];
    ani_int arg1 = TEST_EXPECTED_VALUE4;
    ani_int arg2 = TEST_EXPECTED_VALUE6;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_int result = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, arg1 * arg2);
}

TEST_F(ClassFindMethodTest, find_method_combine_scenes_007)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.Student", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "add_method", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_value args[2U];
    ani_int arg1 = TEST_EXPECTED_VALUE4;
    ani_int arg2 = TEST_EXPECTED_VALUE6;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_int result = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, arg1 + arg2);
}

TEST_F(ClassFindMethodTest, find_method_combine_scenes_008)
{
    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("test.TestB008", &clsB), ANI_OK);
    ASSERT_NE(clsB, nullptr);

    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("test.TestA008", &clsA), ANI_OK);
    ASSERT_NE(clsA, nullptr);

    ani_method constructorMethodB {};
    ASSERT_EQ(env_->Class_FindMethod(clsB, "<ctor>", nullptr, &constructorMethodB), ANI_OK);
    ASSERT_NE(constructorMethodB, nullptr);

    ani_method constructorMethodA {};
    ASSERT_EQ(env_->Class_FindMethod(clsA, "<ctor>", nullptr, &constructorMethodA), ANI_OK);
    ASSERT_NE(constructorMethodA, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(clsB, "int_method", "C{test.TestA008}:i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_object objectB {};
    ASSERT_EQ(env_->Object_New(clsB, constructorMethodB, &objectB), ANI_OK);
    ASSERT_NE(objectB, nullptr);

    ani_object objectA {};
    ASSERT_EQ(env_->Object_New(clsA, constructorMethodA, &objectA), ANI_OK);
    ASSERT_NE(objectA, nullptr);

    ani_value args[1];
    args[0].r = objectA;
    ani_int result = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int_A(objectB, method, &result, args), ANI_OK);
    ASSERT_EQ(result, 66U);
}

TEST_F(ClassFindMethodTest, find_func_abstract)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.CombineA", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "func", "ii:i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("test.CombineB", &clsB), ANI_OK);
    ASSERT_NE(clsB, nullptr);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(clsB, ctor, &object), ANI_OK);
    ani_value args[2];  // NOLINT(modernize-avoid-c-arrays)
    args[0].i = TEST_EXPECTED_VALUE4;
    args[1].i = TEST_EXPECTED_VALUE6;

    ani_int res = 0;
    // Call the method and verify the return value.
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, method, &res, args), ANI_OK);
    ASSERT_EQ(res, TEST_EXPECTED_VALUE4 - TEST_EXPECTED_VALUE6);
}

TEST_F(ClassFindMethodTest, find_func_final)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.AF", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "func", "ii:i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);
    ani_value args[2];  // NOLINT(modernize-avoid-c-arrays)
    args[0].i = TEST_EXPECTED_VALUE4;
    args[1].i = TEST_EXPECTED_VALUE6;

    ani_int res = 0;
    // Call the method and verify the return value.
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, method, &res, args), ANI_OK);
    ASSERT_EQ(res, TEST_EXPECTED_VALUE4 + TEST_EXPECTED_VALUE6);
}

TEST_F(ClassFindMethodTest, find_func_in_class_c)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.CombineC", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "func", "ii:i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);
    ani_value args[2];  // NOLINT(modernize-avoid-c-arrays)
    args[0].i = TEST_EXPECTED_VALUE4;
    args[1].i = TEST_EXPECTED_VALUE6;

    ani_int res = 0;
    // Call the method and verify the return value.
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, method, &res, args), ANI_OK);
    ASSERT_EQ(res, TEST_EXPECTED_VALUE4 - TEST_EXPECTED_VALUE6 - 1);
}

TEST_F(ClassFindMethodTest, find_func_in_namespace)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.nsa.nsb.A", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "func", "ii:i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);
    ani_value args[2];  // NOLINT(modernize-avoid-c-arrays)
    args[0].i = TEST_EXPECTED_VALUE4;
    args[1].i = TEST_EXPECTED_VALUE6;

    ani_int res = 0;
    // Call the method and verify the return value.
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, method, &res, args), ANI_OK);
    ASSERT_EQ(res, TEST_EXPECTED_VALUE4 + TEST_EXPECTED_VALUE6);
}

TEST_F(ClassFindMethodTest, duplicate_no_signature)
{
    ani_class baseCls {};
    ASSERT_EQ(env_->FindClass("test.A", &baseCls), ANI_OK);
    ASSERT_NE(baseCls, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(baseCls, "overloadedBase", nullptr, &method), ANI_AMBIGUOUS);

    ani_class cls;
    ASSERT_EQ(env_->FindClass("test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(cls, "overloadedBase", nullptr, &method), ANI_AMBIGUOUS);
    ASSERT_EQ(env_->Class_FindMethod(cls, "overloaded", nullptr, &method), ANI_AMBIGUOUS);
}

TEST_F(ClassFindMethodTest, static_duplicate_no_signature)
{
    ani_class baseCls {};
    ASSERT_EQ(env_->FindClass("test.A", &baseCls), ANI_OK);
    ASSERT_NE(baseCls, nullptr);

    ani_static_method method {};
    ASSERT_EQ(env_->Class_FindStaticMethod(baseCls, "staticOverloadedBase", nullptr, &method), ANI_AMBIGUOUS);

    ani_class cls;
    ASSERT_EQ(env_->FindClass("test.B", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticOverloadedBase", nullptr, &method), ANI_AMBIGUOUS);
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "staticOverloaded", nullptr, &method), ANI_AMBIGUOUS);
}

TEST_F(ClassFindMethodTest, same_name_static_virtual_function)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.AllFunctions", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ASSERT_NE(ctor, nullptr);

    ani_static_method staticMethod {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "foo", ":i", &staticMethod), ANI_OK);
    ASSERT_NE(staticMethod, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", ":i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_int fooVal {};
    ASSERT_EQ(env_->Object_CallMethod_Int(object, method, &fooVal), ANI_OK);
    ASSERT_EQ(fooVal, 43U);

    ani_int staticFooVal {};
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, staticMethod, &staticFooVal), ANI_OK);
    ASSERT_EQ(staticFooVal, 42U);
}

TEST_F(ClassFindMethodTest, no_static_virtual_function)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.MixedFunctions", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_static_method staticMethod {};
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "bar", ":i", &staticMethod), ANI_NOT_FOUND);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "foo", ":i", &method), ANI_NOT_FOUND);
}

TEST_F(ClassFindMethodTest, static_no_duplicate)
{
    ani_class baseCls {};
    ASSERT_EQ(env_->FindClass("test.NotOverloaded", &baseCls), ANI_OK);
    ASSERT_NE(baseCls, nullptr);

    ani_static_method smethod {};
    ASSERT_EQ(env_->Class_FindStaticMethod(baseCls, "notOverloaded", nullptr, &smethod), ANI_OK);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(baseCls, "notOverloaded", nullptr, &method), ANI_OK);
}

TEST_F(ClassFindMethodTest, check_initalization)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("test.NotOverloaded", &cls), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("test.NotOverloaded"));
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "notOverloaded", nullptr, &method), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("test.NotOverloaded"));
}

TEST_F(ClassFindMethodTest, wrong_signature)
{
    ani_class cls {};
    ani_method m {};
    ASSERT_EQ(env_->FindClass("test.C", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_FindMethod(cls, "imethod", "C{std/core/String}:i", &m), ANI_INVALID_DESCRIPTOR);
    ASSERT_EQ(env_->Class_FindMethod(cls, "imethod", "C{std.core.String}:i", &m), ANI_OK);

    ani_static_method sm {};
    ASSERT_EQ(env_->FindClass("test.CheckWrongSignature", &cls), ANI_OK);
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "method", "C{std/core/String}:", &sm), ANI_INVALID_DESCRIPTOR);
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "method", "C{std.core.String}:", &sm), ANI_OK);
}

TEST_F(ClassFindMethodTest, check_hierarchy)
{
    ani_class clsParent {};
    ASSERT_EQ(env_->FindClass("test.Parent", &clsParent), ANI_OK);
    ani_class clsChild {};
    ASSERT_EQ(env_->FindClass("test.Child", &clsChild), ANI_OK);

    // |-------------------------------------------------------------------------------------------|
    // |                  |    Parent class   |     Child class   |    The same method is found    |
    // |------------------|-------------------|-------------------|--------------------------------|
    // |  Parent method   |       ANI_OK      |       ANI_OK      |              Yes               |
    // |   Child method   |   ANI_NOT_FOUND   |       ANI_OK      |               No               |
    // | Overrided method |       ANI_OK      |       ANI_OK      |          No specified          |
    // |------------------|-------------------|-------------------|--------------------------------|

    ani_method parentMethodInParent {};
    ASSERT_EQ(env_->Class_FindMethod(clsParent, "parentMethod", ":", &parentMethodInParent), ANI_OK);
    ani_method childMethodInParent {};
    ASSERT_EQ(env_->Class_FindMethod(clsParent, "childMethod", ":", &childMethodInParent), ANI_NOT_FOUND);
    ani_method overridedMethodInParent {};
    ASSERT_EQ(env_->Class_FindMethod(clsParent, "overridedMethod", ":", &overridedMethodInParent), ANI_OK);

    ani_method parentMethodInChild {};
    ASSERT_EQ(env_->Class_FindMethod(clsChild, "parentMethod", ":", &parentMethodInChild), ANI_OK);
    ani_method childMethodInChild {};
    ASSERT_EQ(env_->Class_FindMethod(clsChild, "childMethod", ":", &childMethodInChild), ANI_OK);
    ani_method overridedMethodInChild {};
    ASSERT_EQ(env_->Class_FindMethod(clsChild, "overridedMethod", ":", &overridedMethodInChild), ANI_OK);

    ASSERT_EQ(parentMethodInParent, parentMethodInChild);
    ASSERT_NE(childMethodInParent, childMethodInChild);
}

TEST_F(ClassFindMethodTest, check_hierarchy_static)
{
    ani_class clsParent {};
    ASSERT_EQ(env_->FindClass("test.Parent", &clsParent), ANI_OK);
    ani_class clsChild {};
    ASSERT_EQ(env_->FindClass("test.Child", &clsChild), ANI_OK);

    // |--------------------------------------------------------------------------------------------------|
    // |                         |    Parent class   |     Child class   |    The same method is found    |
    // |-------------------------|-------------------|-------------------|--------------------------------|
    // |  Parent static method   |       ANI_OK      |       ANI_OK      |              Yes               |
    // |   Child static method   |   ANI_NOT_FOUND   |       ANI_OK      |               No               |
    // | Overrided static method |       ANI_OK      |       ANI_OK      |               No               |
    // |-------------------------|-------------------|-------------------|--------------------------------|

    ani_static_method parentMethodInParent {};
    ASSERT_EQ(env_->Class_FindStaticMethod(clsParent, "parentStaticMethod", ":", &parentMethodInParent), ANI_OK);
    ani_static_method childMethodInParent {};
    ASSERT_EQ(env_->Class_FindStaticMethod(clsParent, "childStaticMethod", ":", &childMethodInParent), ANI_NOT_FOUND);
    ani_static_method overridedMethodInParent {};
    ASSERT_EQ(env_->Class_FindStaticMethod(clsParent, "overridedStaticMethod", ":", &overridedMethodInParent), ANI_OK);

    ani_static_method parentMethodInChild {};
    ASSERT_EQ(env_->Class_FindStaticMethod(clsChild, "parentStaticMethod", ":", &parentMethodInChild), ANI_OK);
    ani_static_method childMethodInChild {};
    ASSERT_EQ(env_->Class_FindStaticMethod(clsChild, "childStaticMethod", ":", &childMethodInChild), ANI_OK);
    ani_static_method overridedMethodInChild {};
    ASSERT_EQ(env_->Class_FindStaticMethod(clsChild, "overridedStaticMethod", ":", &overridedMethodInChild), ANI_OK);

    ASSERT_EQ(parentMethodInParent, parentMethodInChild);
    ASSERT_NE(childMethodInParent, childMethodInChild);
    ASSERT_NE(overridedMethodInParent, overridedMethodInChild);
}

}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
