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

set(CODIRA_LIB_DIR "modules")

function(add_cangjie_library target_name
)
    set(options
        IS_PACKAGE
        IS_STDXLIB
        IS_CODENATIVE_BACKEND
        NO_SUB_PKG)
    set(one_value_args
        OUTPUT_NAME
        OUTPUT_DIR
        PACKAGE_NAME
        MODULE_NAME
        SOURCE_DIR)
    set(multi_value_args SOURCES DEPENDS FFI)
    cmake_parse_arguments(
        CODIRALIB
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN})

    # The pre-process source files
    set(source_files)
    foreach(file ${CODIRALIB_SOURCES})
        get_filename_component(file_path ${file} PATH)
        if(IS_ABSOLUTE "${file_path}")
            list(APPEND source_files "${file}")
        else()
            list(APPEND source_files "${CODIRALIB_SOURCE_DIR}/${file}")
        endif()
    endforeach()

    set(BACKEND)
    if(CODIRALIB_IS_CODENATIVE_BACKEND)
        set(BACKEND "codenative")
    endif()
    # Set output directory
    set(output_dir)
    set(output_bc_dir)
    if(CODIRALIB_IS_STDXLIB)
        if(NOT ("${CODIRALIB_MODULE_NAME}" STREQUAL ""))
            set(output_dir "${CODIRA_LIB_DIR}/${TARGET_TRIPLE_DIRECTORY_PREFIX}_${BACKEND}/${CODIRALIB_MODULE_NAME}")
            set(output_bc_dir "${CODIRA_LIB_DIR}/${TARGET_TRIPLE_DIRECTORY_PREFIX}_${BACKEND}_bc/${CODIRALIB_MODULE_NAME}")
        else()
            set(output_dir "${CODIRA_LIB_DIR}/${TARGET_TRIPLE_DIRECTORY_PREFIX}_${BACKEND}")
            set(output_bc_dir "${CODIRA_LIB_DIR}/${TARGET_TRIPLE_DIRECTORY_PREFIX}_${BACKEND}_bc")
        endif()
        if(NOT ("${CODIRALIB_OUTPUT_DIR}" STREQUAL ""))
            set(output_dir "${output_dir}/${CODIRALIB_OUTPUT_DIR}")
            set(output_bc_dir "${output_bc_dir}/${CODIRALIB_OUTPUT_DIR}")
        endif()
    endif()

    set(cangjie_compile_flags)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        list(APPEND cangjie_compile_flags "-g")
    elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
        list(APPEND cangjie_compile_flags "-g")
        if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
            # The -g will enable aggressive-parallel-compile, so we need limit --apc to 1 to disable it forcibly.
            list(APPEND cangjie_compile_flags "--apc=1")
        endif()
    else()
       if(NOT CODIRA_BUILD_STDLIB_WITH_COVERAGE)
            list(APPEND cangjie_compile_flags "--trimpath")
            list(APPEND cangjie_compile_flags "${CMAKE_SOURCE_DIR}/src/")
        endif()
    endif()

    if(NOT ("${CODIRALIB_MODULE_NAME}" STREQUAL ""))
        set(output_full_name "${CMAKE_BINARY_DIR}/${output_dir}/${CODIRALIB_MODULE_NAME}.${CODIRALIB_PACKAGE_NAME}")
    else()
        set(output_full_name "${CMAKE_BINARY_DIR}/${output_dir}/${CODIRALIB_PACKAGE_NAME}")
    endif()

    set(output_full_name_prefix "${CMAKE_BINARY_DIR}/${output_dir}/${CODIRALIB_PACKAGE_NAME}")
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        set(output_full_name "${output_full_name}.a") # Set output path and output name
        if(NOT ("${CODIRALIB_MODULE_NAME}" STREQUAL ""))
            set(output_lto_bc_full_name "${CMAKE_BINARY_DIR}/${output_bc_dir}/lib${CODIRALIB_MODULE_NAME}.${CODIRALIB_PACKAGE_NAME}")
        else()
            set(output_lto_bc_full_name "${CMAKE_BINARY_DIR}/${output_bc_dir}/lib${CODIRALIB_PACKAGE_NAME}")
        endif()
        set(output_lto_bc_full_name "${output_lto_bc_full_name}.bc") # Set output path and output name
    endif()

    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        list(APPEND cangjie_compile_flags "--output-type=staticlib")
    endif()

    if(TRIPLE STREQUAL "arm-linux-ohos")
        list(APPEND cangjie_compile_flags "--disable-reflection")
    endif()

    # Set compiler path
    if(CMAKE_CROSSCOMPILING)
        set(CODIRA_NATIVE_CODIRA_TOOLS_PATH ${CMAKE_BINARY_DIR}/../build/bin)
    endif()
    # Do not use ${CMAKE_EXECUTABLE_SUFFIX} here, because its value is determined by the target platform, not the host.
    # Determine the suffix according to the host instead.
    set(cangjie_compiler_tool "codec$<$<BOOL:${CMAKE_HOST_WIN32}>:.exe>")

    # Set no-sub-pkg
    if(CODIRALIB_NO_SUB_PKG)
        set(no_sub_pkg "--no-sub-pkg")
    endif()

    set(output_argument "--output") # Output argument to specify the output file dir and name
    set(module_name_argument) # Module name argument to specify which module the project belongs to
    set(CODENATIVE_PATH)
    # Use the installed llvm tools,
    # in case the backend is compiled from source in previous native-building step
    set(CODENATIVE_PATH $ENV{CODIRA_HOME}/third_party/llvm/bin)
    set(COMPILE_CMD)
    if(CODIRALIB_IS_PACKAGE)
        set(COMPILE_CMD
            ${cangjie_compiler_tool}
            ${no_prelude}
            ${no_sub_pkg}
            ${cangjie_compile_flags}
            -p
            ${CODIRALIB_SOURCE_DIR}
            ${module_name_argument})
    else()
        set(COMPILE_CMD
            ${cangjie_compiler_tool}
            ${no_prelude}
            ${cangjie_compile_flags}
            ${source_files}
            ${module_name_argument})
    endif()
    if(CMAKE_CROSSCOMPILING)
        set(COMPILE_CMD ${COMPILE_CMD} "--target=${TRIPLE}")
        if(NOT ("${CODIRA_TARGET_TOOLCHAIN}" STREQUAL ""))
            set(COMPILE_CMD ${COMPILE_CMD} "-B${CODIRA_TARGET_TOOLCHAIN}")
        endif()
    endif()

    list(APPEND COMPILE_CMD "-Woff=all")

    foreach(build_args ${CODIRA_BUILD_ARGS})
        list(APPEND COMPILE_CMD "${build_args}")
    endforeach()

    set(COMPILE_BC_CMD
        ${COMPILE_CMD}
        --lto=full
        ${output_argument}
        ${output_lto_bc_full_name})
    set(COMPILE_CMD ${COMPILE_CMD} ${output_argument} ${output_full_name})
    
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        if(TRIPLE STREQUAL "arm-linux-ohos")
            list(APPEND COMPILE_CMD "$<IF:$<CONFIG:MinSizeRel>,-Os,-O0>")
            # .bc files is for LTO mode and LTO mode does not support -Os and -Oz.
            list(APPEND COMPILE_BC_CMD "-O0")
        else()
            list(APPEND COMPILE_CMD "$<IF:$<CONFIG:MinSizeRel>,-Os,-O2>")
            # The .bc files is for LTO mode and LTO mode does not support -Os and -Oz.
            list(APPEND COMPILE_BC_CMD "-O2")
        endif()
    endif()
    
    if(CODIRA_BUILD_STDLIB_WITH_COVERAGE)
        list(APPEND COMPILE_CMD "--coverage")
    endif()

    set(ENV{LD_LIBRARY_PATH} $ENV{LD_LIBRARY_PATH}:${CMAKE_BINARY_DIR}/lib)
    string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX}_${BACKEND} output_code_lib_dir)
    add_custom_target(
        ${target_name} ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${output_dir}
        COMMAND ${CMAKE_COMMAND} -E env "CODIRA_PATH=${CMAKE_BINARY_DIR}/modules/${output_code_lib_dir}"  "LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib"
                ${COMPILE_CMD}
        BYPRODUCTS ${output_full_name}
        DEPENDS ${CODIRALIB_DEPENDS} ${CODIRALIB_SOURCE_DIR}
        COMMENT "Generating ${target_name}")
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND
       AND NOT WIN32
       AND NOT DARWIN)
        add_custom_target(
            ${target_name}_bc ALL
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${output_bc_dir}
            COMMAND ${CMAKE_COMMAND} -E env "CODIRA_PATH=${CMAKE_BINARY_DIR}/modules/${output_code_lib_dir}" "LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib"
                     ${COMPILE_BC_CMD}
            BYPRODUCTS ${output_lto_bc_full_name}
            # The ${target_name}_bc depends on ${target_name} so they will not run simultaneously. <target> and <target>_bc
            # compile the same package, which means they may write the same bc cache file. Running simultaneously
            # may cause IO error on windows in some cases.
            DEPENDS ${CODIRALIB_DEPENDS} ${CODIRALIB_SOURCE_DIR} ${target_name}
            COMMENT "Generating ${target_name}_bc")
    endif()

    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        set(TARGET_AR ar)
        if(CMAKE_CROSSCOMPILING)
            if(IOS)
                set(TARGET_AR ${CODIRA_TARGET_TOOLCHAIN}/ar)
            elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
                set(TARGET_AR ${CODIRA_TARGET_TOOLCHAIN}/llvm-ar)
            else()
                set(TARGET_AR ${CODIRA_TARGET_TOOLCHAIN}/${TRIPLE}-ar)
            endif()
        endif()
        if(CMAKE_HOST_UNIX)
            set(MOVE_CMD mv)
        elseif(CMAKE_HOST_WIN32)
            set(MOVE_CMD move)
        endif()
        add_custom_command(
            TARGET ${target_name}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${target_name} && cd ${target_name}
            COMMAND ${CMAKE_COMMAND} -E remove_directory tmp
            COMMAND ${CMAKE_COMMAND} -E make_directory tmp && cd tmp
            COMMAND ${TARGET_AR} x ${output_full_name}
            COMMAND ${MOVE_CMD} *.o ${output_full_name_prefix}.o
            COMMAND cd ..
            COMMAND ${CMAKE_COMMAND} -E remove_directory tmp
            BYPRODUCTS ${output_full_name_prefix}.o)
    endif()

    # Install
    if(NOT ("${CODIRALIB_MODULE_NAME}" STREQUAL ""))
        set(file_name "${CODIRALIB_MODULE_NAME}.${CODIRALIB_PACKAGE_NAME}")
    else()
        set(file_name "${CODIRALIB_PACKAGE_NAME}")
    endif()
    
    set(install_files "${CMAKE_BINARY_DIR}/${output_dir}/${file_name}.codeo")
    
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    else()
        list(APPEND install_files "${CMAKE_BINARY_DIR}/${output_dir}/${file_name}.bchir")
        list(APPEND install_files "${CMAKE_BINARY_DIR}/${output_dir}/${file_name}.pdba")
    endif()

    if(CODIRA_CODEGEN_CODENATIVE_BACKEND
       AND NOT WIN32
       AND NOT DARWIN)
        list(APPEND install_files ${output_lto_bc_full_name})
    endif()
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        install(FILES ${install_files} DESTINATION "${TARGET_TRIPLE_DIRECTORY_PREFIX}_${BACKEND}/static/stdx")
        install(FILES ${install_files} DESTINATION "${TARGET_TRIPLE_DIRECTORY_PREFIX}_${BACKEND}/dynamic/stdx")
    endif()
endfunction()

set(CODENATIVE_BACKEND "codenative")
# Install cangjie library FFI
function(install_cangjie_library_ffi lib_name)
    # Set install dir
    string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX} output_lib_dir)
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        install(TARGETS ${lib_name} DESTINATION ${output_lib_dir}_${CODENATIVE_BACKEND}/static/stdx)
    endif()
endfunction()

function(install_cangjie_library_ffi_s lib_name)
    # Set install dir
    string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX} output_lib_dir)
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        install(TARGETS ${lib_name} DESTINATION ${output_lib_dir}_${CODENATIVE_BACKEND}/dynamic/stdx)
    endif()
endfunction()
