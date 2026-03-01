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

#include "string_util.h"

std::vector<std::string> abckit_wrapper::StringUtil::Split(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> tokens;
    auto start = str.find_first_not_of(delimiter, 0);
    auto pos = str.find_first_of(delimiter, 0);
    while (start != std::string::npos) {
        if (pos > start) {
            tokens.push_back(str.substr(start, pos - start));
        }
        start = str.find_first_not_of(delimiter, pos);
        pos = str.find_first_of(delimiter, start + 1);
    }

    return tokens;
}