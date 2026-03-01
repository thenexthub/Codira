/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License") \;
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

#ifndef PANDA_ENUMERATE_NAPI_H
#define PANDA_ENUMERATE_NAPI_H

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// CC-OFFNXT(G.PRE.02) code generation
#define EVERY_SECOND2(first, second) , second
#define EVERY_SECOND3(first, second, ...) , second EVERY_SECOND2(__VA_ARGS__)
#define EVERY_SECOND4(first, second, ...) , second EVERY_SECOND3(__VA_ARGS__)
#define EVERY_SECOND5(first, second, ...) , second EVERY_SECOND4(__VA_ARGS__)
#define EVERY_SECOND6(first, second, ...) , second EVERY_SECOND5(__VA_ARGS__)
#define EVERY_SECOND7(first, second, ...) , second EVERY_SECOND6(__VA_ARGS__)
#define EVERY_SECOND8(first, second, ...) , second EVERY_SECOND7(__VA_ARGS__)
#define EVERY_SECOND9(first, second, ...) , second EVERY_SECOND8(__VA_ARGS__)

#define PARAMS_PAIR2(first, second) , first second
#define PARAMS_PAIR3(first, second, ...) , first second PARAMS_PAIR2(__VA_ARGS__)
#define PARAMS_PAIR4(first, second, ...) , first second PARAMS_PAIR3(__VA_ARGS__)
#define PARAMS_PAIR5(first, second, ...) , first second PARAMS_PAIR4(__VA_ARGS__)
#define PARAMS_PAIR6(first, second, ...) , first second PARAMS_PAIR5(__VA_ARGS__)
#define PARAMS_PAIR7(first, second, ...) , first second PARAMS_PAIR6(__VA_ARGS__)
#define PARAMS_PAIR8(first, second, ...) , first second PARAMS_PAIR7(__VA_ARGS__)
#define PARAMS_PAIR9(first, second, ...) , first second PARAMS_PAIR8(__VA_ARGS__)

#define COUNT_EVERY_SECOND(_2, __2, _3, __3, _4, __4, _5, __5, _6, __6, _7, __7, _8, __8, num, ...) EVERY_SECOND##num
#define EVERY_SECOND(first, second, ...) \
    second COUNT_EVERY_SECOND(__VA_ARGS__, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1)(__VA_ARGS__)

#define COUNT_PARAMS_PAIR(_2, __2, _3, __3, _4, __4, _5, __5, _6, __6, _7, __7, _8, __8, num, ...) PARAMS_PAIR##num
#define PARAMS_PAIR(first, second, ...) \
    first second COUNT_PARAMS_PAIR(__VA_ARGS__, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1)(__VA_ARGS__)

// CC-OFFNXT(G.PRE.06) list generation
#define ENUMERATE_NAPI(FN_MACRO)                                                                                       \
    FN_MACRO(napi_resolve_deferred, napi_env, env, napi_deferred, deferred, napi_value, resolution)                    \
    FN_MACRO(napi_open_escapable_handle_scope, napi_env, env, napi_escapable_handle_scope *, result)                   \
    FN_MACRO(napi_typeof, napi_env, env, napi_value, value, napi_valuetype *, result)                                  \
    FN_MACRO(napi_is_dataview, napi_env, env, napi_value, value, bool *, result)                                       \
    FN_MACRO(napi_is_typedarray, napi_env, env, napi_value, value, bool *, result)                                     \
    FN_MACRO(napi_create_int32, napi_env, env, int32_t, value, napi_value *, result)                                   \
    FN_MACRO(napi_unwrap, napi_env, env, napi_value, js_object, void **, result)                                       \
    FN_MACRO(napi_new_instance, napi_env, env, napi_value, constructor, size_t, argc, const napi_value *, argv,        \
             napi_value *, result)                                                                                     \
    FN_MACRO(napi_is_exception_pending, napi_env, env, bool *, result)                                                 \
    FN_MACRO(napi_close_escapable_handle_scope, napi_env, env, napi_escapable_handle_scope, scope)                     \
    FN_MACRO(napi_get_null, napi_env, env, napi_value *, result)                                                       \
    FN_MACRO(napi_set_element, napi_env, env, napi_value, object, uint32_t, index, napi_value, value)                  \
    FN_MACRO(napi_call_function, napi_env, env, napi_value, recv, napi_value, func, size_t, argc, const napi_value *,  \
             argv, napi_value *, result)                                                                               \
    FN_MACRO(napi_is_error, napi_env, env, napi_value, value, bool *, result)                                          \
    FN_MACRO(napi_create_arraybuffer, napi_env, env, size_t, byte_length, void **, data, napi_value *, result)         \
    FN_MACRO(napi_get_value_uint32, napi_env, env, napi_value, value, uint32_t *, result)                              \
    FN_MACRO(napi_delete_reference, napi_env, env, napi_ref, ref)                                                      \
    FN_MACRO(napi_get_new_target, napi_env, env, napi_callback_info, cbinfo, napi_value *, result)                     \
    FN_MACRO(napi_get_undefined, napi_env, env, napi_value *, result)                                                  \
    FN_MACRO(napi_strict_equals, napi_env, env, napi_value, lhs, napi_value, rhs, bool *, result)                      \
    FN_MACRO(napi_has_property, napi_env, env, napi_value, property, napi_value, name, bool *, result)                 \
    FN_MACRO(napi_has_element, napi_env, env, napi_value, property, uint32_t, index, bool *, result)                   \
    FN_MACRO(napi_has_named_property, napi_env, env, napi_value, object, const char *, utf8name, bool *, result)       \
    FN_MACRO(napi_has_own_property, napi_env, env, napi_value, property, napi_value, name, bool *, result)             \
    FN_MACRO(napi_throw_error, napi_env, env, const char *, code, const char *, msg)                                   \
    FN_MACRO(napi_instanceof, napi_env, env, napi_value, object, napi_value, constructor, bool *, result)              \
    FN_MACRO(napi_reject_deferred, napi_env, env, napi_deferred, deferred, napi_value, rejection)                      \
    FN_MACRO(napi_get_reference_value, napi_env, env, napi_ref, ref, napi_value *, result)                             \
    FN_MACRO(napi_get_value_string_utf8, napi_env, env, napi_value, value, char *, buf, size_t, bufsize, size_t *,     \
             result)                                                                                                   \
    FN_MACRO(napi_get_value_int32, napi_env, env, napi_value, value, int32_t *, result)                                \
    FN_MACRO(napi_get_element, napi_env, env, napi_value, object, uint32_t, index, napi_value *, result)               \
    FN_MACRO(napi_get_property, napi_env, env, napi_value, object, napi_value, key, napi_value *, result)              \
    FN_MACRO(napi_create_promise, napi_env, env, napi_deferred *, deferred, napi_value *, promise)                     \
    FN_MACRO(napi_create_reference, napi_env, env, napi_value, value, uint32_t, initial_refcount, napi_ref *, result)  \
    FN_MACRO(napi_create_array_with_length, napi_env, env, size_t, length, napi_value *, result)                       \
    FN_MACRO(napi_create_array, napi_env, env, napi_value *, result)                                                   \
    FN_MACRO(napi_is_promise, napi_env, env, napi_value, value, bool *, is_promise)                                    \
    FN_MACRO(napi_get_value_string_utf16, napi_env, env, napi_value, value, char16_t *, buf, size_t, bufsize,          \
             size_t *, result)                                                                                         \
    FN_MACRO(napi_get_array_length, napi_env, env, napi_value, value, uint32_t *, result)                              \
    FN_MACRO(napi_get_value_int64, napi_env, env, napi_value, value, int64_t *, result)                                \
    FN_MACRO(napi_fatal_exception, napi_env, env, napi_value, err)                                                     \
    FN_MACRO(napi_get_boolean, napi_env, env, bool, value, napi_value *, result)                                       \
    FN_MACRO(napi_object_seal, napi_env, env, napi_value, object)                                                      \
    FN_MACRO(napi_get_value_bool, napi_env, env, napi_value, value, bool *, result)                                    \
    FN_MACRO(napi_get_named_property, napi_env, env, napi_value, object, const char *, utf8name, napi_value *, result) \
    FN_MACRO(napi_create_error, napi_env, env, napi_value, code, napi_value, msg, napi_value *, result)                \
    FN_MACRO(napi_create_type_error, napi_env, env, napi_value, code, napi_value, msg, napi_value *, result)           \
    FN_MACRO(napi_coerce_to_bool, napi_env, env, napi_value, value, napi_value *, result)                              \
    FN_MACRO(napi_coerce_to_string, napi_env, env, napi_value, value, napi_value *, result)                            \
    FN_MACRO(napi_create_int64, napi_env, env, int64_t, value, napi_value *, result)                                   \
    FN_MACRO(napi_reference_ref, napi_env, env, napi_ref, ref, uint32_t *, result)                                     \
    FN_MACRO(napi_throw_type_error, napi_env, env, const char *, code, const char *, msg)                              \
    FN_MACRO(napi_get_global, napi_env, env, napi_value *, result)                                                     \
    FN_MACRO(napi_create_function, napi_env, env, const char *, utf8name, size_t, length, napi_callback, cb, void *,   \
             data, napi_value *, result)                                                                               \
    FN_MACRO(napi_escape_handle, napi_env, env, napi_escapable_handle_scope, scope, napi_value, escapee, napi_value *, \
             result)                                                                                                   \
    FN_MACRO(napi_set_named_property, napi_env, env, napi_value, object, const char *, utf8name, napi_value, value)    \
    FN_MACRO(napi_set_property, napi_env, env, napi_value, object, napi_value, name, napi_value, value)                \
    FN_MACRO(napi_wrap, napi_env, env, napi_value, js_object, void *, native_object, napi_finalize, finalize_cb,       \
             void *, finalize_hint, napi_ref *, result)                                                                \
    FN_MACRO(napi_get_cb_info, napi_env, env, napi_callback_info, cbinfo, size_t *, argc, napi_value *, argv,          \
             napi_value *, this_arg, void **, data)                                                                    \
    FN_MACRO(napi_is_array, napi_env, env, napi_value, value, bool *, result)                                          \
    FN_MACRO(napi_throw, napi_env, env, napi_value, error)                                                             \
    FN_MACRO(napi_open_handle_scope, napi_env, env, napi_handle_scope *, result)                                       \
    FN_MACRO(napi_create_uint32, napi_env, env, uint32_t, value, napi_value *, result)                                 \
    FN_MACRO(napi_create_double, napi_env, env, double, value, napi_value *, result)                                   \
    FN_MACRO(napi_create_bigint_words, napi_env, env, int, sign_bit, size_t, word_count, const uint64_t *, words,      \
             napi_value *, result)                                                                                     \
    FN_MACRO(napi_get_value_bigint_uint64, napi_env, env, napi_value, value, uint64_t *, result, bool *, lossless)     \
    FN_MACRO(napi_get_value_bigint_words, napi_env, env, napi_value, value, int *, sign_bit, size_t *, word_count,     \
             uint64_t *, words)                                                                                        \
    FN_MACRO(napi_get_arraybuffer_info, napi_env, env, napi_value, arraybuffer, void **, data, size_t *, byte_length)  \
    FN_MACRO(napi_define_class, napi_env, env, const char *, utf8name, size_t, length, napi_callback, constructor,     \
             void *, data, size_t, property_count, const napi_property_descriptor *, properties, napi_value *, result) \
    FN_MACRO(napi_is_date, napi_env, env, napi_value, value, bool *, is_date)                                          \
    FN_MACRO(napi_get_and_clear_last_exception, napi_env, env, napi_value *, result)                                   \
    FN_MACRO(napi_create_string_utf8, napi_env, env, const char *, str, size_t, length, napi_value *, result)          \
    FN_MACRO(napi_create_symbol, napi_env, env, napi_value, value, napi_value *, result)                               \
    FN_MACRO(napi_get_value_double, napi_env, env, napi_value, value, double *, result)                                \
    FN_MACRO(napi_is_arraybuffer, napi_env, env, napi_value, value, bool *, result)                                    \
    FN_MACRO(napi_close_handle_scope, napi_env, env, napi_handle_scope, scope)                                         \
    FN_MACRO(napi_create_string_utf16, napi_env, env, const char16_t *, str, size_t, length, napi_value *, result)     \
    FN_MACRO(napi_get_last_error_info, napi_env, env, const napi_extended_error_info **, result)                       \
    FN_MACRO(napi_create_typedarray, napi_env, env, napi_typedarray_type, type, size_t, length, napi_value,            \
             arraybuffer, size_t, byte_offset, napi_value *, result)                                                   \
    FN_MACRO(napi_create_dataview, napi_env, env, size_t, length, napi_value, arraybuffer, size_t, byte_offset,        \
             napi_value *, result)                                                                                     \
    FN_MACRO(napi_get_property_names, napi_env, env, napi_value, object, napi_value *, result)                         \
    FN_MACRO(napi_create_object, napi_env, env, napi_value *, result)

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif  // PANDA_ENUMERATE_NAPI_H
