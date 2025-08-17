#===-- lib/Testing/CMakeLists.txt ------------------------------------------===#
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://toolchain.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
#===------------------------------------------------------------------------===#

add_library(NonGTestTesting EXCLUDE_FROM_ALL
    testing.cpp
    fp-testing.cpp
)
set_target_properties(NonGTestTesting PROPERTIES FOLDER "Flang/Tests")

if (LLVM_LINK_LLVM_DYLIB)
  set(toolchain_libs LLVM)
else()
  toolchain_map_components_to_libnames(toolchain_libs Support)
endif()
target_link_libraries(NonGTestTesting ${toolchain_libs})
