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

set(Mangle_SRC BaseMangler.cpp ASTMangler.cpp CHIRMangler.cpp Compression.cpp)
set(Mangle_UNITTESTS_SRC BaseMangler.cpp ASTMangler.cpp CHIRMangler.cpp Compression.cpp)
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    get_filename_component(CHIRMANGLE CHIRTypeManglingUtils.cpp ABSOLUTE)
    list(APPEND Mangle_SRC ${CHIRMANGLE} CHIRManglingUtils.cpp)
endif()
add_library(CodiraMangle OBJECT ${Mangle_SRC})
add_library(CodiraUnittestsMangle OBJECT ${Mangle_UNITTESTS_SRC})

# use llvm
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_dependencies(CodiraMangle codenative)
endif()

# IsEnumTrans2Class method in CodeGenUtils.h could be separated with llvm util methods.
target_include_directories(CodiraMangle PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_options(CodiraMangle PRIVATE ${CODEC_WITH_LLVM_EXTRA_WARNINGS})
