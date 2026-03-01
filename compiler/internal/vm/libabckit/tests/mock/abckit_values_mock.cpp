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

#include "src/mock/mock_values.h"

#include <cstdint>
#include <cstring>

const char *DEFAULT_PATH = "abckit.abc";
const size_t DEFAULT_PATH_SIZE = std::strlen(DEFAULT_PATH) + 1;
const char *DEFAULT_CONST_CHAR = "abckit default const char*";
const size_t DEFAULT_CONST_CHAR_SIZE = std::strlen(DEFAULT_CONST_CHAR) + 1;
const bool DEFAULT_BOOL = true;
const uint8_t DEFAULT_U8 = 0x11;
const uint16_t DEFAULT_U16 = 0x1111;
const uint32_t DEFAULT_U32 = 0x11111111;
const int32_t DEFAULT_I32 = 0x11111112;
const uint64_t DEFAULT_U64 = 0x1111111122222222;
const int64_t DEFAULT_I64 = 0x1111111122222223;
const size_t DEFAULT_SIZE_T = 0x1111111133333333;
