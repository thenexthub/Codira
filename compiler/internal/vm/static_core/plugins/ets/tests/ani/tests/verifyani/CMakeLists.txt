#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

include(cmake/ani_verify_tests.cmake)

add_subdirectory(array_ops)
add_subdirectory(class_ops)
add_subdirectory(error_ops)
add_subdirectory(function_ops)
add_subdirectory(local_scope_ops)
add_subdirectory(object_ops)
add_subdirectory(ref_ops)
add_subdirectory(scope_ops)
add_subdirectory(string_ops)
add_subdirectory(vm_ops)
add_subdirectory(promise_ops)
