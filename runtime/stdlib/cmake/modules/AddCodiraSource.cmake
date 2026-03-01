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

set(CODIRA_NATIVE_CODIRA_TOOLS_PATH ${CMAKE_BINARY_DIR}/bin)
set(CODIRA_LIB_DIR "modules")
set(CODIRA_EXECUTABLE_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)
set(CODENATIVE_BACKEND "codenative")

# Compile cangjie files into an object file(as a library or executable)
# Usage:
# add_cangjie_library(
#     target_name                 # The target name you want to generate
#     IS_STDLIB                   # This is a kind of stdlib
#     IS_PACKAGE                  # This is a package
#     IS_PRELUDE                  # This option will append a option to codec: --no-prelude
#
#     [BACKEND]                   # Specify the backend, default is codenative
#     [OUTPUT_NAME]               # Specify the output file name
#     [OUTPUT_DIR]                # Specify the lib name, and will install to the directory with this name
#     [PACKAGE_NAME]              # Package name of target cangjie source file
#     [MODULE_NAME]               # Module name of target cangjie source file and we will generate product to a directory with this name
#     [BACKEND_OPTS]              # This argument is backend-options passed to backend
#     [WHEN_CONFIG_NAME_OPTS]     # This argument is other options passed to codec
#     [WHEN_CONFIG_VALUES_OPTS]   # This argument is other options passed to codec
#     [CONFIG_OPTS]               # This argument is other options passed to codec
#     [SOURCE_DIR]                # Input source directory
#     [DISABLE_REFLECTION]        # Disable reflection for specific package
#
#     [SOURCES]                   # Input source files
#     [DEPENDS]                   # This target should be dependent on these targets
#     [NO_SUB_PKG]                # This package doesn't have sub-packages
#     )
function(add_cangjie_library target_name)
    set(options
        IS_PRELUDE
        IS_PACKAGE
        IS_STDLIB
        IS_CODENATIVE_BACKEND
        IS_JS_BACKEND
        DISABLE_REFLECTION
        NO_SUB_PKG
        NO_SANCOV)
    set(one_value_args
        OUTPUT_NAME
        OUTPUT_DIR
        PACKAGE_NAME
        MODULE_NAME
        WHEN_CONFIG_NAME_OPTS
        WHEN_CONFIG_VALUES_OPTS
        CONFIG_OPTS
        SOURCE_DIR)
    set(multi_value_args SOURCES BACKEND_OPTS DEPENDS FFI)
    cmake_parse_arguments(
        CODIRALIB
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN})

    # pre-process source files
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
    # setting output directory
    set(output_dir)
    set(output_bc_dir)
    if(CODIRALIB_IS_STDLIB)
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
            # -g will enable aggressive-parallel-compile, so we need limit --apc to 1 to disable it forcibly.
            list(APPEND cangjie_compile_flags "--apc=1")
        endif()
    else()
       if(NOT CODIRA_BUILD_STDLIB_WITH_COVERAGE)
            list(APPEND cangjie_compile_flags "--trimpath")
            list(APPEND cangjie_compile_flags "${CMAKE_SOURCE_DIR}/libs/")
        endif()
    endif()

    if (CODIRALIB_IS_CODENATIVE_BACKEND)
        if (CODIRA_ASAN_SUPPORT)
            list(APPEND cangjie_compile_flags "--sanitize=address")
        elseif (CODIRA_TSAN_SUPPORT)
            list(APPEND cangjie_compile_flags "--sanitize=thread")
        elseif (CODIRA_HWASAN_SUPPORT)
            list(APPEND cangjie_compile_flags "--sanitize=hwaddress")
        endif()
    endif()

    set(MKDIR_TEMP_FILES_CMD)
    # append backend-options
    list(LENGTH CODIRALIB_OPTS options_length)
    if(NOT (options_length EQUAL 0))
        list(APPEND cangjie_compile_flags ${CODIRALIB_OPTS})
    endif()
    # append config options
    if(NOT ("${CODIRALIB_CONFIG_OPTS}" STREQUAL ""))
        list(APPEND cangjie_compile_flags "--cfg=\"${CODIRALIB_CONFIG_OPTS}\"")
    endif()

    if(NOT ("${CODIRALIB_MODULE_NAME}" STREQUAL ""))
        set(output_full_name "${CMAKE_BINARY_DIR}/${output_dir}/${CODIRALIB_MODULE_NAME}.${CODIRALIB_PACKAGE_NAME}")
    else()
        set(output_full_name "${CMAKE_BINARY_DIR}/${output_dir}/${CODIRALIB_PACKAGE_NAME}")
    endif()
    
    set(output_full_name_prefix "${CMAKE_BINARY_DIR}/${output_dir}/${CODIRALIB_PACKAGE_NAME}")
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        set(output_full_name "${output_full_name}.a") # set output path and output name
        if(NOT ("${CODIRALIB_MODULE_NAME}" STREQUAL ""))
            set(output_lto_bc_full_name "${CMAKE_BINARY_DIR}/${output_bc_dir}/lib${CODIRALIB_MODULE_NAME}.${CODIRALIB_PACKAGE_NAME}")
        else()
            set(output_lto_bc_full_name "${CMAKE_BINARY_DIR}/${output_bc_dir}/lib${CODIRALIB_PACKAGE_NAME}")
        endif()        
        
        set(output_lto_bc_full_name "${output_lto_bc_full_name}.bc") # set output path and output name
    endif()

    foreach(build_args ${CODIRA_BUILD_ARGS})
        list(APPEND cangjie_compile_flags "${build_args}")
    endforeach()

    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        list(APPEND cangjie_compile_flags "--output-type=staticlib")
    endif()
    if(TRIPLE STREQUAL "arm-linux-ohos")
        list(APPEND cangjie_compile_flags "--disable-reflection")
    endif()

    # set compiler path
    if(CMAKE_CROSSCOMPILING)
        set(CODIRA_NATIVE_CODIRA_TOOLS_PATH ${CMAKE_BINARY_DIR}/../build/bin)
    endif()
    # Do not use ${CMAKE_EXECUTABLE_SUFFIX} here, because its value is determined by the target platform, not the host.
    # Determine the suffix according to the host instead.
    set(cangjie_compiler_tool "codec$<$<BOOL:${CMAKE_HOST_WIN32}>:.exe>")

    # create building task
    if(CODIRALIB_IS_PRELUDE)
        set(no_prelude "--no-prelude") # not to import prelude libraries while compiling core library
    endif()
    # no-sub-pkg
    if(CODIRALIB_NO_SUB_PKG)
        set(no_sub_pkg "--no-sub-pkg")
    endif()

    set(output_argument "--output") # output argument to specify the output file dir and name
    set(module_name_argument) # module name argument to specify which module the project belongs to
    set(CODENATIVE_PATH)
    if(CMAKE_CROSSCOMPILING)
        # When cross-compiling stdlib, use the installed llvm tools,
        # in case the backend is compiled from source in previous native-building step
        set(CODENATIVE_PATH $ENV{CODIRA_HOME}/third_party/llvm/bin)
        # $ENV{CODIRA_HOME}
    else()
        set(CODENATIVE_PATH $ENV{CODIRA_HOME}/third_party/llvm/bin)
    endif()
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

    set(COMPILE_BC_CMD
        ${COMPILE_CMD}
        --lto=full
        ${output_argument}
        ${output_lto_bc_full_name})
    set(COMPILE_CMD ${COMPILE_CMD} ${output_argument} ${output_full_name})

    if(NOT ("${CODIRALIB_MODULE_NAME}" STREQUAL ""))
        set(temp_files_dir "${CMAKE_BINARY_DIR}/${output_dir}/${CODIRALIB_MODULE_NAME}.${CODIRALIB_PACKAGE_NAME}-temp-files")
    else()
        set(temp_files_dir "${CMAKE_BINARY_DIR}/${output_dir}/${CODIRALIB_PACKAGE_NAME}-temp-files")
    endif()
    
    set(COMPILE_CMD ${COMPILE_CMD} "-j1")
    set(COMPILE_CMD ${COMPILE_CMD} "--save-temps=${temp_files_dir}")
    set(MKDIR_TEMP_FILES_CMD COMMAND ${CMAKE_COMMAND} -E make_directory ${temp_files_dir})

    if(CODIRA_BUILD_STDLIB_WITH_COVERAGE)
        list(APPEND COMPILE_CMD "--coverage")
    endif()

    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        if(TRIPLE STREQUAL "arm-linux-ohos")
            list(APPEND COMPILE_CMD "$<IF:$<CONFIG:MinSizeRel>,-Os,-O0>")
            # .bc files is for LTO mode and LTO mode does not support -Os and -Oz.
            list(APPEND COMPILE_BC_CMD "-O0")
        else()
            list(APPEND COMPILE_CMD "$<IF:$<CONFIG:MinSizeRel>,-Os,-O2>")
            # .bc files is for LTO mode and LTO mode does not support -Os and -Oz.
            list(APPEND COMPILE_BC_CMD "-O2")
        endif()
    endif()

    set(ENV{LD_LIBRARY_PATH} $ENV{LD_LIBRARY_PATH}:${CMAKE_BINARY_DIR}/lib)
    string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX}_${BACKEND} output_code_lib_dir)
    add_custom_target(
        ${target_name} ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${output_dir}
        ${MKDIR_TEMP_FILES_CMD}
        COMMAND ${CMAKE_COMMAND} -E env "CODIRA_PATH=${CMAKE_BINARY_DIR}/modules/${output_code_lib_dir}"  "LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib"
                ${COMPILE_CMD}
        BYPRODUCTS ${output_full_name}
        DEPENDS ${CODIRALIB_DEPENDS} ${CODIRALIB_SOURCE_DIR}
        COMMENT "Generating ${target_name}")
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND
       AND NOT WIN32
       AND NOT CODIRA_SANITIZER_SUPPORT_ENABLED
       AND NOT DARWIN)
        add_custom_target(
            ${target_name}_bc ALL
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${output_bc_dir}
            COMMAND ${CMAKE_COMMAND} -E env "CODIRA_PATH=${CMAKE_BINARY_DIR}/modules/${output_code_lib_dir}" "LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib"
                     ${COMPILE_BC_CMD}
            BYPRODUCTS ${output_lto_bc_full_name}
            # ${target_name}_bc depends on ${target_name} so they will not run simultaneously. <target> and <target>_bc
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

    # install
    # sanitizer version only needs library files, codeo, bchir, pdba files are not needed
    if (CODIRA_SANITIZER_SUPPORT_ENABLED)
        return()
    endif()
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
        install(FILES ${install_files} DESTINATION ${output_dir})
    endif()
endfunction()

set(CODENATIVE_BACKEND "codenative")
# Install cangjie library FFI
function(install_cangjie_library_ffi lib_name)
    # set install dir
    string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX} output_lib_dir)
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        install(TARGETS ${lib_name} DESTINATION lib/${output_lib_dir}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH})
    endif()
endfunction()

function(install_cangjie_library_ffi_s lib_name)
    # set install dir
    string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX} output_lib_dir)
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        install(TARGETS ${lib_name} DESTINATION runtime/lib/${output_lib_dir}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH})
    endif()
endfunction()

function(install_ast_library_ffi lib_name)
    # set install dir
    string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX} output_lib_dir)
    if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
        install(FILES ${lib_name} DESTINATION lib/${output_lib_dir}_${CODENATIVE_BACKEND}${SANITIZER_SUBPATH})
    endif()
endfunction()
