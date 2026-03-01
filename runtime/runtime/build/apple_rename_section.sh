#!/bin/bash

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
set -e

platform=$1
c_compiler=$2
shift 2

if [ "${platform}" == "macos_cangjie" ] || [ "${platform}" == "mac_x86_64_cangjie" ] || [ "${platform}" == "mac_aarch64_cangjie" ]; then
  mac_sdk_path=$(xcrun --show-sdk-path)
  for param in "$@"; do
    IFS=';' read -ra target_objects <<< "$param"
    for obj in "${target_objects[@]}"; do
      # The no_eh_labels: tell ld64 not to produces .eh labels on all FDEs,
      # as it will lead to incompatibility with ld64.lld. 
      # The -no_eh_labels option is remove in macOS 15, to fix build error, remove the option temporarily.
      $c_compiler \
        -isysroot ${mac_sdk_path} \
        -Wl,-r,-rename_section,__TEXT,__text,__TEXT,__codert_text \
        $obj \
        -o $obj;
    done
  done
else
  if [ "${platform}" == "ios_simulator_aarch64_cangjie" ]; then
    XCODE_PATH=$(xcode-select -p)
    CMAKE_IOS_DEVELOPER_ROOT=${XCODE_PATH}/Platforms/iPhoneSimulator.platform/Developer
    CMAKE_IOS_SDK_ROOT=${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/iPhoneSimulator17.5.sdk
    TARGET=arm64-apple-ios11-simulator
  elif [ "${platform}" == "ios_simulator_x86_64_cangjie" ]; then
    XCODE_PATH=$(xcode-select -p)
    CMAKE_IOS_DEVELOPER_ROOT=${XCODE_PATH}/Platforms/iPhoneSimulator.platform/Developer
    CMAKE_IOS_SDK_ROOT=${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/iPhoneSimulator17.5.sdk
    TARGET=x86_64-apple-ios11-simulator
  else
    XCODE_PATH=$(xcode-select -p)
    CMAKE_IOS_DEVELOPER_ROOT=${XCODE_PATH}/Platforms/iPhoneOS.platform/Developer
    CMAKE_IOS_SDK_ROOT=${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/iPhoneOS17.5.sdk
    TARGET=arm64-apple-ios11
  fi
  for param in "$@"; do
    IFS=';' read -ra target_objects <<< "$param"
    for obj in "${target_objects[@]}"; do
      $c_compiler \
        -target ${TARGET} \
        -isysroot ${CMAKE_IOS_SDK_ROOT} \
        -Wl,-r,-rename_section,__TEXT,__text,__TEXT,__codert_text \
        -Wl,-no_eh_labels \
        $obj \
        -o $obj;
    done
  done
fi
