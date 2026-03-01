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

function(setup_taihe_cmake_test_env)
  if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    return()  # 不是最顶层项目，跳过设置
  endif()
  # 编译器设置
  set(CMAKE_C_COMPILER "clang" CACHE STRING "C compiler" FORCE)
  set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "C++ compiler" FORCE)

  # 编译标准
  set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard" FORCE)
  set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "" FORCE)

  # PIC 与调试构建
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC" CACHE STRING "C flags" FORCE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC" CACHE STRING "C++ flags" FORCE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)

  set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json" FORCE)

  # 用于解决多次配置 python 导致的列表空字符串问题
  if(POLICY CMP0007)
    cmake_policy(SET CMP0007 NEW)
  endif()
  # Python 查找
  if(NOT DEFINED Python3_EXECUTABLE)
    set(Python3_EXECUTABLE "/usr/bin/python3" CACHE FILEPATH "Python3 executable")
  endif()
  find_package(Python3 REQUIRED COMPONENTS Interpreter)

  # Coverage 开关
  option(ENABLE_COVERAGE "Enable coverage run for the Python command" OFF)

  # ASan 可选
  option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
  if(ENABLE_ASAN AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address" CACHE STRING "" FORCE)
  endif()
endfunction()

# taihec 获取配置路径
function(execute_and_set_variable OUTPUT_VAR_NAME)
  set(options "")
  set(oneValueArgs "TAIHEC_PATH")
  set(multiValueArgs "")
  cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(ARGS_TAIHEC_PATH)
    set(TAIHEC_PATH "${ARGS_TAIHEC_PATH}")
  elseif(BIN_TAIHEC_PATH)
    set(TAIHEC_PATH "${BIN_TAIHEC_PATH}")
  else()
    set(TAIHEC_PATH "taihec")
  endif()

  execute_process(
    COMMAND ${TAIHEC_PATH} ${ARGN}
    OUTPUT_VARIABLE _output
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _result
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
  )

  if(_result EQUAL 0)
    string(REPLACE "\\" "/" _output "${_output}")
    set(${OUTPUT_VAR_NAME} "${_output}" CACHE STRING "Set ${OUTPUT_VAR_NAME} from taihec")
  else()
    message(FATAL_ERROR "Failed to execute 'taihec ${COMMAND_ARGS}'. Error code: ${_result}")
  endif()
endfunction()

# taihe 代码生成
function(generate_code_from_idl demo_name idl_files gen_ets_names author_bridge user_bridge taihe_configs gen_include_dir gen_abi_c_files gen_bridge_cpp_files gen_ets_files)
  set(options "")
  set(oneValueArgs "TAIHEC_PATH")
  set(multiValueArgs "")
  cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(ARGS_TAIHEC_PATH)
    set(TAIHEC_PATH "${ARGS_TAIHEC_PATH}")
  elseif(BIN_TAIHEC_PATH)
    set(TAIHEC_PATH "${BIN_TAIHEC_PATH}")
  else()
    set(TAIHEC_PATH "taihec")
  endif()

  set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

  set(GEN_INCLUDE_DIR "${GEN_DIR}/include")

  set(GEN_ABI_C_FILES)
  set(GEN_AUTHOR_CPP_FILES)
  set(GEN_USER_CPP_FILES)

  # config 为字符串时需要拆分为字符串列表
  string(REGEX MATCH " " HAS_SPACE "${taihe_configs}")

  if(HAS_SPACE)
    separate_arguments(TAIHE_CONFIGS_LIST UNIX_COMMAND "${taihe_configs}")
  else()
    set(TAIHE_CONFIGS_LIST ${taihe_configs})
  endif()

  set(IDL_EXTENSION_REGEX "\\.(taihe|ohidl|hmidl)$")

  foreach(TAIHE_FILE ${idl_files})
    # 替换扩展名
    get_filename_component(TAIHE_FILE_NAME ${TAIHE_FILE} NAME)
    # 将修改后的文件名添加到新的列表中
    string(REGEX REPLACE "${IDL_EXTENSION_REGEX}" ".abi.c" GEN_ABI_C_FILE ${TAIHE_FILE_NAME})
    list(APPEND GEN_ABI_C_FILES "${GEN_DIR}/src/${GEN_ABI_C_FILE}")

    # author bridge
    if(author_bridge STREQUAL "kn-author")
      string(REGEX REPLACE "${IDL_EXTENSION_REGEX}" ".knapi.cpp" GEN_AUTHOR_FILE ${TAIHE_FILE_NAME})
      list(APPEND GEN_AUTHOR_CPP_FILES "${GEN_DIR}/src/${GEN_AUTHOR_FILE}")
      # TODO: kn_author config
      # list(APPEND TAIHE_CONFIGS_LIST "-Gkn_author")
    elseif(author_bridge STREQUAL "cpp-author")
      list(APPEND TAIHE_CONFIGS_LIST "-Gcpp-author")
    endif()

    # user bridge
    if(user_bridge STREQUAL "cpp-user")
      list(APPEND TAIHE_CONFIGS_LIST "-Gcpp-user")
    elseif(user_bridge STREQUAL "napi-bridge")
      string(REGEX REPLACE "${IDL_EXTENSION_REGEX}" ".napi.cpp" GEN_USER_FILE ${TAIHE_FILE_NAME})
      list(APPEND GEN_USER_CPP_FILES "${GEN_DIR}/src/${GEN_USER_FILE}")
      list(APPEND TAIHE_CONFIGS_LIST "-Gnapi-bridge")
      # TODO: napi_bridge config
      # list(APPEND TAIHE_CONFIGS_LIST "-Gnapi-bridge")
    elseif(user_bridge STREQUAL "ani-bridge")
      string(REGEX REPLACE "${IDL_EXTENSION_REGEX}" ".ani.cpp" GEN_USER_FILE ${TAIHE_FILE_NAME})
      list(APPEND GEN_USER_CPP_FILES "${GEN_DIR}/src/${GEN_USER_FILE}")
      list(APPEND TAIHE_CONFIGS_LIST "-Gani-bridge")
    endif()
  endforeach()

  set(GEN_BRIDGE_CPP_FILES ${GEN_AUTHOR_CPP_FILES} ${GEN_USER_CPP_FILES})

  if(user_bridge STREQUAL "ani-bridge")
    list(APPEND GEN_ABI_C_FILES "${GEN_DIR}/src/taihe.platform.ani.abi.c")
  endif()

  set_source_files_properties(
    ${GEN_BRIDGE_CPP_FILES}
    PROPERTIES
    LANGUAGE CXX
    COMPILE_FLAGS "-std=c++17"
  )

  set(GEN_ETS_FILES)
  foreach(ETS_NAME ${gen_ets_names})
    set(ETS_FILE "${GEN_DIR}/${ETS_NAME}")
    list(APPEND GEN_ETS_FILES "${ETS_FILE}")
  endforeach()

  if(ENABLE_COVERAGE)
    list(APPEND COMMAND_TO_RUN "coverage" "run" "--parallel-mode" "-m" "taihe.cli.compiler")
  else()
    list(APPEND COMMAND_TO_RUN "${TAIHEC_PATH}")
  endif()

  add_custom_command(
    OUTPUT ${GEN_INCLUDE_DIR} ${GEN_ABI_C_FILES} ${GEN_BRIDGE_CPP_FILES} ${GEN_ETS_FILES}
    COMMAND ${COMMAND_TO_RUN}
    ${idl_files}
    -O${GEN_DIR}
    ${TAIHE_CONFIGS_LIST}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
    DEPENDS ${idl_files}
    COMMENT "Generating Taihe C++ header and source files... ${GEN_DIR}"
    VERBATIM
  )

  set(${gen_ets_files} ${GEN_ETS_FILES} PARENT_SCOPE)
  set(${gen_abi_c_files} ${GEN_ABI_C_FILES} PARENT_SCOPE)
  set(${gen_bridge_cpp_files} ${GEN_BRIDGE_CPP_FILES} PARENT_SCOPE)
  set(${gen_include_dir} ${GEN_INCLUDE_DIR} PARENT_SCOPE)
endfunction()

# 编译 taihe runtime 为 静态库
function(add_taihe_runtime)
  if (NOT TARGET taihe_runtime)
    execute_and_set_variable(TAIHE_RUNTIME_SOURCE_DIR "--print-runtime-source-path")
    execute_and_set_variable(TAIHE_RUNTIME_HEADER_DIR "--print-runtime-header-path")

    set(TAIHE_RUNTIME_SOURCES
      "${TAIHE_RUNTIME_SOURCE_DIR}/string.cpp"
      "${TAIHE_RUNTIME_SOURCE_DIR}/object.cpp"
      "${TAIHE_RUNTIME_SOURCE_DIR}/runtime.cpp"
    )

    add_library(taihe_runtime STATIC
      ${TAIHE_RUNTIME_SOURCES}
    )

    set_target_properties(taihe_runtime PROPERTIES
      ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    )

    target_include_directories(taihe_runtime PUBLIC ${TAIHE_RUNTIME_HEADER_DIR})
  endif()
endfunction()

# 配置 panda sdk
function(setup_panda_sdk)
  # 用户显式提供了 PANDA_HOME, 不执行下载逻辑
  if(DEFINED PANDA_HOME AND IS_DIRECTORY "${PANDA_HOME}")
    message(STATUS "Using user-provided PANDA_HOME: ${PANDA_HOME}")
    set(ENV{PANDA_HOME} "${PANDA_HOME}")
    # 自动下载模式
  else()
    if(NOT DEFINED PANDA_EXTRACT_DIR)
      execute_and_set_variable(PANDA_EXTRACT_DIR "--print-panda-vm-path")
      message(STATUS "PANDA_HOME set to: ${PANDA_EXTRACT_DIR}/linux_host_tools")
    endif()
    set(PANDA_HOME "${PANDA_EXTRACT_DIR}/linux_host_tools")
    set(ENV{PANDA_HOME} "${PANDA_HOME}")
    set(PANDA_HOME "${PANDA_HOME}" PARENT_SCOPE)
  endif()

  # Set ETS compiler path
  list(APPEND CMAKE_PROGRAM_PATH "$ENV{PANDA_HOME}/bin")

  find_program(ETS_COMPILER es2panda)
  find_program(ETS_RUNTIME ark)
  find_program(ETS_DISASM ark_disasm)
  find_program(ETS_LINK ark_link)

  if(NOT ETS_COMPILER)
    message(FATAL_ERROR "ets_compiler not found! Please set ETS_COMPILER_PATH or ensure ets_compiler is in your PATH.")
  endif()
endfunction()

# 生成 arktsconfig.json
function(write_arkts_config arktsconfig ets_files)
  foreach(file IN LISTS ets_files)
    get_filename_component(file_name ${file} NAME)
    string(REGEX REPLACE "\\.ets$" "" stripped_file "${file_name}")
    set(entry "      \"${stripped_file}\": [\"${file}\"]")
    list(APPEND ETS_PATAIHE_ENTRIES "${entry}")
  endforeach()

  string(REPLACE ";" ",\n" ETS_PATAIHE_ENTRIES_JOINED "${ETS_PATAIHE_ENTRIES}")

  file(WRITE "${arktsconfig}"
    "{\n"
    "  \"compilerOptions\": {\n"
    "    \"baseUrl\": \"${PANDA_HOME}\",\n"
    "    \"paths\": {\n"
    "      \"std\": [\"${PANDA_HOME}/../ets/stdlib/std\"],\n"
    "      \"escompat\": [\"${PANDA_HOME}/../ets/stdlib/escompat\"],\n"
    "      \"@ohos\": [\"${PANDA_HOME}/../ets/sdk/sdk/api/@ohos\"],\n"
    "${ETS_PATAIHE_ENTRIES_JOINED}\n"
    "    }\n"
    "  }\n"
    "}\n"
  )
endfunction()

# 自定义 ETS 构建规则
function(build_ets_target ets_file output_dir abc_file dump_file ets_files arktsconfig)
  # 创建输出目录（如果不存在）
  file(MAKE_DIRECTORY ${output_dir})

  add_custom_command(
    OUTPUT ${abc_file} # 输出文件
    COMMAND ${ETS_COMPILER} ${ets_file}
    --output ${abc_file}
    --extension ets
    --arktsconfig ${arktsconfig} # 生成命令
    && ${ETS_DISASM} ${abc_file} ${dump_file}
    DEPENDS ${ets_file} ${ets_files} ${arktsconfig} # 输入文件依赖
    COMMENT "Building ${ets_file} ETS target" # 注释
  )
endfunction()

function(abc_link demo_name main_abc ets_files)
  set(ABC_FILES)
  set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/build")
  set(ARKTSCONFIG "${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json")

  write_arkts_config(${ARKTSCONFIG} "${ets_files}")

  # 为每个 ETS 文件创建编译目标
  foreach(ETS_FILE ${ets_files})
    get_filename_component(ETS_NAME_P ${ETS_FILE} NAME)
    string(REGEX REPLACE "\\.[^.]*$" "" ETS_NAME "${ETS_NAME_P}")

    set(ABC_FILE "${BUILD_DIR}/${ETS_NAME}.ets.abc")
    set(DUMP_FILE "${BUILD_DIR}/${ETS_NAME}.ets.abc.dump")

    build_ets_target(${ETS_FILE} ${BUILD_DIR} ${ABC_FILE} ${DUMP_FILE} "${ets_files}" ${ARKTSCONFIG})
    list(APPEND ABC_FILES "${ABC_FILE}")
  endforeach()

  # 创建链接命令
  add_custom_command(
    OUTPUT ${main_abc}
    COMMAND ${ETS_LINK} --output ${main_abc} -- ${ABC_FILES}
    DEPENDS ${ABC_FILES}
    COMMENT "Linking all ABC files to main.abc"
  )
endfunction()

function(run_abc demo_name main_abc)
  # 设置
  get_filename_component(demo_name ${demo_name} NAME_WE)

  # 创建运行目标，并明确依赖于链接目标
  add_custom_target(${demo_name}_run ALL
    COMMAND # LD_PRELOAD=/usr/lib/llvm-14/lib/clang/14.0.0/lib/linux/libclang_rt.asan-x86_64.so
    LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR} ${ETS_RUNTIME}
    --boot-panda-files=$ENV{PANDA_HOME}/../ets/etsstdlib.abc
    --boot-panda-files=$ENV{PANDA_HOME}/../ets/etssdk.abc
    --load-runtimes=ets ${main_abc} main.ETSGLOBAL::main
    && echo "Run successful" || (echo "Run failed" && exit 1)
    COMMENT "Running ${demo_name}"
    DEPENDS ${main_abc} ${demo_name}
  )
endfunction()

# 配置 ani 头文件
function(setup_ani_header)
  if(NOT DEFINED ANI_HEADER)
    set(ANI_HEADER $ENV{PANDA_HOME}/../ohos_arm64/include/plugins/ets/runtime/ani CACHE PATH "ANI include path")
    message(STATUS "Panda include directory: ${ANI_HEADER}")
  endif()

  if(EXISTS "${ANI_HEADER}")
    include_directories("${ANI_HEADER}")
  else()
    message(FATAL_ERROR "Cannot find the path: ${ANI_HEADER}")
  endif()
endfunction()

# 将生成文件编译为静态库
function(compile_gen_lib gen_demo_name gen_include_dir gen_abi_c_files gen_ani_cpp_files)
  add_library(${gen_demo_name} STATIC ${gen_abi_c_files} ${gen_ani_cpp_files})

  target_compile_options(${gen_demo_name} PRIVATE "-Wno-attributes")
  set_target_properties(${gen_demo_name} PROPERTIES LINKER_LANGUAGE CXX)
  target_link_libraries(${gen_demo_name} PRIVATE taihe_runtime)
  target_link_options(${gen_demo_name} PRIVATE "-Wl,--no-undefined")
  target_include_directories(${gen_demo_name} PUBLIC ${gen_include_dir} ${TAIHE_RUNTIME_HEADER_DIR})
endfunction()

# 将用户文件编译为动态库
function(compile_dylib demo_name user_include_dir user_cpp_files gen_include_dir link_gen_lib)
  if (user_cpp_files)
    add_library(${demo_name} SHARED ${user_cpp_files})
  else()
    set(dummy_cpp "${CMAKE_CURRENT_BINARY_DIR}/${demo_name}_dummy.cpp")
    file(WRITE "${dummy_cpp}" "extern \"C\" void __taihe_dummy_symbol__() {}\n")
    add_library(${demo_name} SHARED "${dummy_cpp}")
  endif()
  target_compile_options(${demo_name} PRIVATE "-Wno-attributes")
  set_target_properties(${demo_name} PROPERTIES LINKER_LANGUAGE CXX)
  target_link_libraries(${demo_name} PRIVATE taihe_runtime ${link_gen_lib})
  target_link_options(${demo_name} PRIVATE "-Wl,--no-undefined")
  target_include_directories(${demo_name} PUBLIC ${user_include_dir} ${gen_include_dir} ${TAIHE_RUNTIME_HEADER_DIR})
endfunction()

function(add_taihe_library target_name idl_files)
  set(options "")
  set(oneValueArgs "AUTHOR_BRIDGE" "USER_BRIDGE" "TAIHE_CONFIGS" "TAIHEC_PATH")
  set(multiValueArgs "")
  cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if(NOT ARGS_AUTHOR_BRIDGE)
    set(ARGS_AUTHOR_BRIDGE "cpp-author")
  endif()
  if(NOT ARGS_USER_BRIDGE)
    set(ARGS_USER_BRIDGE "ani-bridge")
  endif()
  if(NOT ARGS_TAIHE_CONFIGS)
    set(ARGS_TAIHE_CONFIGS "")
  endif()

  if(ARGS_TAIHEC_PATH)
    set(TAIHEC_PATH "${ARGS_TAIHEC_PATH}")
  elseif(BIN_TAIHEC_PATH)
    set(TAIHEC_PATH "${BIN_TAIHEC_PATH}")
  else()
    set(TAIHEC_PATH "taihec")
  endif()

  execute_and_set_variable(TAIHE_RUNTIME_SOURCE_DIR "--print-runtime-source-path" TAIHEC_PATH ${TAIHEC_PATH})
  execute_and_set_variable(TAIHE_RUNTIME_HEADER_DIR "--print-runtime-header-path" TAIHEC_PATH ${TAIHEC_PATH})
  set(TAIHE_RUNTIME_SOURCES
    "${TAIHE_RUNTIME_SOURCE_DIR}/string.cpp"
    "${TAIHE_RUNTIME_SOURCE_DIR}/object.cpp"
    "${TAIHE_RUNTIME_SOURCE_DIR}/runtime.cpp"
  )
  set_source_files_properties(
    ${TAIHE_RUNTIME_SOURCES}
    PROPERTIES
    LANGUAGE CXX
    COMPILE_FLAGS "-std=c++17"
  )

  generate_code_from_idl(
    ${target_name}
    "${idl_files}"
    ""
    "${ARGS_AUTHOR_BRIDGE}"
    "${ARGS_USER_BRIDGE}"
    "${ARGS_TAIHE_CONFIGS}"
    GEN_INCLUDE_DIR
    GEN_ABI_C_FILES
    GEN_BRIDGE_CPP_FILES
    GEN_ETS_FILES
    TAIHEC_PATH ${TAIHEC_PATH}
  )

  # compile static library
  add_library(${target_name} STATIC ${TAIHE_RUNTIME_SOURCES} ${GEN_ABI_C_FILES} ${GEN_BRIDGE_CPP_FILES} ${GEN_STDLIB_ABI_C_FILES})
  target_compile_options(${target_name} PRIVATE "-Wno-attributes")
  set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE CXX)
  target_link_options(${target_name} PRIVATE "-Wl,--no-undefined")
  target_include_directories(${target_name} PUBLIC ${GEN_INCLUDE_DIR} ${TAIHE_RUNTIME_HEADER_DIR})
endfunction()

function(add_ani_demo demo_name idl_files taihe_configs gen_ets_names user_ets_files user_include_dir user_cpp_files)
  if (NOT DEFINED TAIHE_STDLIB_DIR)
    execute_and_set_variable(TAIHE_STDLIB_DIR "--print-stdlib-path")
  endif()

  set(MAIN_ABC "${CMAKE_CURRENT_BINARY_DIR}/main.abc")

  # panda sdk 环境配置
  setup_panda_sdk()
  # ani 头文件
  setup_ani_header()
  # 编译 taihe 运行时
  add_taihe_runtime()
  # ani 代码生成相关配置
  set(taihe_configs "-Gpretty-print ${taihe_configs}")
  # 生成代码
  generate_code_from_idl(${demo_name} "${idl_files}" "${gen_ets_names}" "cpp-author" "ani-bridge" "${taihe_configs}" GEN_INCLUDE_DIR GEN_ABI_C_FILES GEN_ANI_CPP_FILES GEN_ETS_FILES)
  # 生成代码静态库编译
  set(ALL_GEN_ABI_C_FILES ${GEN_STDLIB_ABI_C_FILES} ${GEN_ABI_C_FILES})
  compile_gen_lib("taihe_gen_${demo_name}" "${GEN_INCLUDE_DIR}" "${ALL_GEN_ABI_C_FILES}" "${GEN_ANI_CPP_FILES}")
  # 动态库编译
  compile_dylib(${demo_name} "${user_include_dir}" "${user_cpp_files}" "${GEN_INCLUDE_DIR}" "taihe_gen_${demo_name}")
  # 链接为 main.abc
  set(ALL_ETS_FILES ${user_ets_files} ${GEN_ETS_FILES})
  abc_link(${demo_name} ${MAIN_ABC} "${ALL_ETS_FILES}")
  # 运行
  run_abc(${demo_name} ${MAIN_ABC})
endfunction()
