/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsInteropScenariosEtsToJs : public EtsInteropTest {};

TEST_F(EtsInteropScenariosEtsToJs, boxed_BigInt_conversion)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_boxed_BigInt_conversion.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, boxed_Boolean_conversion)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_boxed_Boolean_conversion.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, boxed_Byte_conversion)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_boxed_Byte_conversion.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, boxed_Char_conversion)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_boxed_Char_conversion.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, boxed_Double_conversion)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_boxed_Double_conversion.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, boxed_Float_conversion)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_boxed_Float_conversion.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, boxed_Int_conversion)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_boxed_Int_conversion.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, boxed_Long_conversion)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_boxed_Long_conversion.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, boxed_Number_conversion)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_boxed_Number_conversion.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, boxed_Short_conversion)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_boxed_Short_conversion.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, boxed_String_conversion)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_boxed_String_conversion.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_standalone_function_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_standalone_function_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_class_methodCall)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_class_method_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_rest_params)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_rest_params_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_class_methodCall_union)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_class_method_call_union.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_interface_methodCall)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_interface_method_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_interface_methodCall_union)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_interface_method_call_union.js"));
}

// NOTE #15888 enable this after interop is implemented for eTS getter use in JS
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_class_getter)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_class_getter.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_class_getter_union)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_class_getter_union.js"));
}

// NOTE #15889 enable this after interop is implemented for eTS setter use in JS
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_class_setter)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_class_setter.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_class_setter_union)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_class_setter_union.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_lambda_function_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_lambda_function_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_generic_function_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_generic_function_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_generic_function_with_union_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_generic_function_with_union_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_rest_params_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_rest_params_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_spread_params_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_spread_params_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_rest_params_union_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_rest_params_union_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_spread_params_union_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_spread_params_union_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_extend_class)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_extend_class.js"));
}

// NOTE(oignatenko) return and arg types any, unknown, undefined need real TS because transpiling cuts off
// important details. Have a look at declgen_ets2ts
TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_any_call)
{
    // Note this also covers scenario of return type any
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_any_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_unknown_call)
{
    // Note this also covers scenario of return type unknown
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_unknown_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_undefined_call)
{
    // Note this also covers scenario of return type undefined
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_undefined_call.js"));
}
// NOTE (#24570): fix interop tests with tuple
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_tuple_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_tuple_call.js"));
}

// NOTE #15890 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_callable_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_callable_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_generic_type_as_parameter)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_generic_type_as_parameter.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_generic_type_as_return_value)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_generic_type_as_return_value.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, negative_test_overloaded_function)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/negative_test_overloaded_function.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, negative_test_overloaded_method)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/negative_test_overloaded_method.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, negative_test_overloaded_static_method)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/negative_test_overloaded_static_method.js"));
}

// #22991
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_default_value_defined_for_parameter)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_default_value_define_for_parameter.js"));
}

// #22991
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_default_value_defined_for_method_parameter)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_default_value_define_for_method_parameter.js"));
}

// #22991
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_default_value_defined_for_static_method_parameter)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_default_value_define_for_static_method_parameter.js"));
}

// #22991
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_default_value_define_for_parameter_undefine)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_default_value_define_for_parameter_undefine.js"));
}

// #22991
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_default_value_define_for_method_parameter_undefine)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_default_value_define_for_method_parameter_undefine.js"));
}

// #22991
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_default_value_define_for_static_method_parameter_undefine)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_default_value_define_for_static_method_parameter_undefine.js"));
}

// #22991
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_default_value_define_derived_class_method)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_default_value_define_derived_class_method.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_optional_primitive_explicit)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_optional_primitive_explicit.js"));
}

// #22991
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_optional_primitive_default)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_optional_primitive_default.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_primitive)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_primitive.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, negative_test_function_arg_type_primitive)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/negative_test_function_arg_type_primitive.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_primitive_union)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_primitive_union.js"));
}

// NOTE #16812 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_negative_test_function_arg_type_primitive_union)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/negative_test_function_arg_type_primitive_union.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_return_type_primitive)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_return_type_primitive.js"));
}

// NOTE #16103 enable this after interop is implemented for interrface arguments
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_interface_arg_ets_value)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/interface_arg/ets_value.js"));
}

// NOTE #16103 enable this after interop is implemented for interrface arguments
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_interface_generic_arg_ets_value)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/interface_arg/generic_ets_value.js"));
}

// NOTE #16103 enable this after interop is implemented for interrface arguments
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_interface_arg_js_value)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/interface_arg/js_value.js"));
}

// NOTE #16103 enable this after interop is implemented for interrface arguments
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_interface_generic_arg_js_value)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/interface_arg/generic_js_value.js"));
}

// NOTE #16103 enable this after interop is implemented for interrface arguments
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_interface_arg_js_anon_value)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/interface_arg/js_anon_value.js"));
}

// NOTE #16103 enable this after interop is implemented for interrface arguments
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_negative_test_interface_arg_wrong_ets_value)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/interface_arg/negative_wrong_ets_value.js"));
}

// NOTE #16103 enable this after interop is implemented for interrface arguments
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_negative_test_interface_arg_incomplete_value)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/interface_arg/negative_incomplete_value.js"));
}

// NOTE #16103 enable this after interop is implemented for interrface arguments
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_negative_test_interface_arg_not_callable)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/interface_arg/negative_not_callable.js"));
}

// NOTE #16103 enable this after interop is implemented for interrface arguments
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_negative_test_interface_arg_wrong_prop_type)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/interface_arg/negative_wrong_prop_type.js"));
}

// #24686
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_async_function_literal)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_async_function_literal.js"));
}

// #24686
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_async_function_any)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_async_function_any.js"));
}
// NOTE (#24570): fix interop test with tuple
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_async_function_extra_set)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_async_function_extra_set.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_async_function_subset_by_ref)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_async_function_subset_by_ref.js"));
}

// #24686
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_async_function_subset_by_value)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_async_function_subset_by_value.js"));
}

// #24686
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_async_function_user_class)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_async_function_user_class.js"));
}

// #24686
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_async_function_user_interface_ret)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_async_function_user_interface_ret.js"));
}

// #24686
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_async_function_user_interface_param)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_async_function_user_interface_param.js"));
}

// #22991
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_conflict_array)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_conflict_array.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_conflict_boolean)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_conflict_boolean.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_conflict_string)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_conflict_string.js"));
}

// NOTE(splatov) #17857 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_conflict_arraybuffer)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_conflict_arraybuffer.js"));
}

// NOTE(splatov) #17858 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_conflict_dataview)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_conflict_dataview.js"));
}

// NOTE(splatov) #17859 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_conflict_date)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_conflict_date.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_conflict_error)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_conflict_error.js"));
}

// NOTE(splatov) #17858 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_type_conflict_map)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_conflict_map.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_return_value_is_this)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_return_value_is_this.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_return_value_is_omitted)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_return_value_is_omitted.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_rest_parameter)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_rest_parameter.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_spread_parameter)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_spread_parameter.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_overload)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_overload_set.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_callable_return_value)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_callable_return_value.js"));
}

// NOTE(nikitayegorov) #18198 enable this after literal types are fixed
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_function_arg_string_literal_type)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_string_literal_type.js"));
}

// NOTE(nikitayegorov) #18198 enable this after literal types are fixed
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_intersection_type)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_intersection_type.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_symbol_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_symbol_call.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_arg_type_class_with_symbol_call)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_arg_type_class_with_symbol_call.js"));
}

// NOTE(smirnovams) enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosEtsToJs, DISABLED_test_class_in_place_field_declarations)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_class_in_place_field_declarations.js"));
}

TEST_F(EtsInteropScenariosEtsToJs, test_function_returns_composite_type)
{
    ASSERT_EQ(true, RunJsTestSuite("js_suites/test_function_returns_composite_type.js"));
}

}  // namespace ark::ets::interop::js::testing
