/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef JSVM_VERSION_H
#define JSVM_VERSION_H

// JSVM version
#define JSVM_MAJOR_VERSION 1
#define JSVM_MINOR_VERSION 0
#define JSVM_PATCH_VERSION 0

#ifndef DEBUG
#define JSVM_COMPILE_STATUS "Debug"
#else
#define JSVM_COMPILE_STATUS "Release"
#endif

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

#define JSVM_VERSION_STRING                                                            \
    "v" STRINGIFY(JSVM_MAJOR_VERSION) "." STRINGIFY(JSVM_MINOR_VERSION) "." STRINGIFY( \
        JSVM_PATCH_VERSION) "(" JSVM_COMPILE_STATUS ")"

// JSVM_API_VERSION
#define JSVM_API_VERSION 9

#endif
