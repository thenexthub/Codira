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

include(${CMAKE_CURRENT_LIST_DIR}/TaiheUtils.cmake)

function(add_taihe_ani_library target_name idl_files)
  set(options "")
  set(oneValueArgs "TAIHE_CONFIGS" "TAIHEC_PATH")
  set(multiValueArgs "")
  cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
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

  add_taihe_library(${target_name} "${idl_files}"
    AUTHOR_BRIDGE "cpp-author"
    USER_BRIDGE "ani-bridge"
    TAIHE_CONFIGS "${ARGS_TAIHE_CONFIGS}"
    TAIHEC_PATH ${TAIHEC_PATH}
  )
endfunction()

function(add_taihe_cpp_library target_name idl_files)
  set(options "")
  set(oneValueArgs "TAIHE_CONFIGS" "TAIHEC_PATH")
  set(multiValueArgs "")
  cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
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

  add_taihe_library(${target_name} "${idl_files}"
    AUTHOR_BRIDGE "cpp-author"
    USER_BRIDGE "cpp-user"
    TAIHE_CONFIGS "${ARGS_TAIHE_CONFIGS}"
    TAIHEC_PATH ${TAIHEC_PATH}
  )
endfunction()
