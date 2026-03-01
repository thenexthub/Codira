# Copyright (c) 2023-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

panda_ets_interop_js_gtest(ets_interop_js_test__scenarios
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/scenarios.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/scenarios.ets
    JS_SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/scenarios.test.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/negative_test_function_arg_type_primitive.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/negative_test_function_arg_type_primitive_union.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/negative_test_overloaded_function.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/negative_test_overloaded_method.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/negative_test_overloaded_static_method.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_async_function_any.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_async_function_extra_set.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_async_function_literal.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_async_function_subset_by_ref.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_async_function_subset_by_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_async_function_user_class.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_async_function_user_interface_param.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_async_function_user_interface_ret.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_boxed_BigInt_conversion.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_boxed_Boolean_conversion.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_boxed_Byte_conversion.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_boxed_Char_conversion.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_boxed_Double_conversion.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_boxed_Float_conversion.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_boxed_Int_conversion.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_boxed_Long_conversion.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_boxed_Number_conversion.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_boxed_Short_conversion.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_boxed_String_conversion.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_callable_return_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_class_getter.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_class_getter_union.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_class_in_place_field_declarations.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_class_method_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_class_method_call_union.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_class_setter.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_class_setter_union.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_default_value_define_derived_class_method.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_default_value_define_for_method_parameter.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_default_value_define_for_method_parameter_undefine.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_default_value_define_for_parameter.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_default_value_define_for_parameter_undefine.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_default_value_define_for_static_method_parameter.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_default_value_define_for_static_method_parameter_undefine.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_extend_class.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_string_literal_type.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_any_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_callable_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_class_with_symbol_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_conflict_arraybuffer.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_conflict_array.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_conflict_boolean.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_conflict_dataview.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_conflict_date.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_conflict_error.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_conflict_map.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_conflict_string.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_optional_primitive_default.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_optional_primitive_explicit.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_primitive.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_primitive_union.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_symbol_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_tuple_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_undefined_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_arg_type_unknown_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_overload_set.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_rest_params_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_rest_params_union_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_returns_composite_type.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_return_type_primitive.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_spread_params_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_function_spread_params_union_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_generic_function_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_generic_function_with_union_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_generic_type_as_parameter.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_generic_type_as_return_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_interface_method_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_interface_method_call_union.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_intersection_type.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_lambda_function_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_rest_parameter.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_rest_params_call.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_return_value_is_omitted.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_return_value_is_this.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_spread_parameter.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/test_standalone_function_call.js
        
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/interface_arg/ets_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/interface_arg/generic_ets_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/interface_arg/generic_js_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/interface_arg/js_anon_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/interface_arg/js_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/interface_arg/negative_incomplete_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/interface_arg/negative_not_callable.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/interface_arg/negative_wrong_ets_value.js
        ${CMAKE_CURRENT_SOURCE_DIR}/js_suites/interface_arg/negative_wrong_prop_type.js
    COMPILATION_JS_WITH_CODES_ON
)

add_subdirectory(indirect_call)