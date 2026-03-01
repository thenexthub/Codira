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

#ifndef ABCKIT_WRAPPER_STRING_UTIL_H
#define ABCKIT_WRAPPER_STRING_UTIL_H

#include <string>
#include <vector>

namespace abckit_wrapper {
class StringUtil {
public:
    /**
     * Split str by delimiter
     * @param str split str
     * @param delimiter split delimiter
     * @return split tokens, if split failed return empty vector
     * e.g. Split("##a#", "#"); => ["a"]
     */
    static std::vector<std::string> Split(const std::string &str, const std::string &delimiter);
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_STRING_UTIL_H
