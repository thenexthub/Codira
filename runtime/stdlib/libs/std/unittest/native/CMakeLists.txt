# Copyright © 2025 Huawei Technologies Co., Ltd. All Rights Reserved.
# This source file is part of the Codira project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

file(GLOB unittest_ffi_src ./*.c)
set(libname cangjie-std-unittestFFI)

add_library(${libname}-objs OBJECT ${unittest_ffi_src})
target_compile_options(${libname}-objs PRIVATE ${CMAKE_C_COVERAGE_FLAGS})
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_library(${libname} STATIC $<TARGET_OBJECTS:${libname}-objs>)
endif()
install_cangjie_library_ffi(${libname})
