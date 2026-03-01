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

#ifndef ABCKIT_WRAPPER_FULLNAME_UTIL_H
#define ABCKIT_WRAPPER_FULLNAME_UTIL_H

#include <string>

namespace abckit_wrapper {

/**
 * @brief fullname seperator
 * example: a.b.c.d
 */
constexpr std::string_view FULL_NAME_SEP = ".";

/**
 * @brief FullNameUtil
 */
class FullNameUtil {
public:
    /**
     * Get object full name
     * @tparam T object type
     * @param instance abckit::core object
     * @return object full name
     */
    template <typename T>
    static std::string GetFullName(const T &instance)
    {
        if (instance.GetParentNamespace()) {
            return GetFullName(instance.GetParentNamespace()) + FULL_NAME_SEP.data() + instance.GetName();
        }

        return instance.GetModule().GetName() + FULL_NAME_SEP.data() + instance.GetName();
    }
};
}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_FULLNAME_UTIL_H
