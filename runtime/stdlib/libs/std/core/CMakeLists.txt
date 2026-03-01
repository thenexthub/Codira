# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

add_subdirectory(native)

set(CODIRA_CORE_SRCS
    native.code
    box.code
    object.code
    duration.code
    sleep.code
    option.code
    to_string.code
    endian.code
    float.code
    iterator.code
    collection.code
    flow.code
    hashable.code
    equatable.code
    c_pointer_resource.code
    c_string_resource.code
    c_type.code
    libc.code
    thread_common.code
    thread_local.code
    countable.code
    range.code
    arithmetic_exception.code
    illegal_argument_exception.code
    illegal_format_exception.code
    illegal_state_exception.code
    index_out_of_bounds_exception.code
    negative_array_size_exception.code
    stack_trace_element.code
    none_value_exception.code
    unsupported_exception.code
    overflow_exception.code
    out_of_memory_error.code
    resource.code
    spawn_exception.code
    char_extension.code
    byte_extension.code
    type_alias.code
    atexit.code
    main_args.code
    annotation.code
    illegal_memory_exception.code
    array_common.code
    string_builder.code
    string.code
    timeout_exception.code
    string_common.code
    incompatible_package_exception.code)
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    set(CODENATIVE_CORE_SRCS
        ${CODIRA_CORE_SRCS}
        runtime_call_throw_exception.code
        future.code
        thread.code
        exception.code
        error.code
        array.code
        array_intrinsic.code
        atomic.code
        stack_overflow_error.code
        print.code
        comparable.code
        CACHE INTERNAL "")
endif()