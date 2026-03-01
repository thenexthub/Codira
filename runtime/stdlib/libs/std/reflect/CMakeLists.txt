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

if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    set(REFLECT_SRCS
        class_type_info_codenative.code
        constants.code
        exception.code
        function_info_codenative.code
        interface_type_info_codenative.code
        intrinsic_codenative.code
        modifier_info.code
        package_info_codenative.code
        parameter_info_codenative.code
        primitive_type_info_codenative.code
        struct_type_info_codenative.code
        type_info_codenative.code
        utils.code
        variable_info_codenative.code
        CACHE INTERNAL "")
endif()