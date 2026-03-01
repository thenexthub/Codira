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

#ifndef GUARD_NAME_CACHE_NAME_CACHE_CONSTANTS_H
#define GUARD_NAME_CACHE_NAME_CACHE_CONSTANTS_H

#include <memory>
#include <string>
#include <unordered_map>

namespace ark::guard {
struct ObjectNameCache;
using ObjectNameCacheTable = std::unordered_map<std::string, std::shared_ptr<ObjectNameCache>>;

/**
 * @brief ObjectNameCache
 * This structure stores object name cache information.
 */
struct ObjectNameCache {
    // obfuscated name
    std::string obfName;
    // this field stores the name cache information of annotation and field
    std::unordered_map<std::string, std::string> fields;
    // this field stores the name cache information of function
    std::unordered_map<std::string, std::string> methods;
    // this field stores the name cache information of class, enum and namespace
    ObjectNameCacheTable objects;
};

namespace ObfuscationToolConstants {
constexpr std::string_view NAME = "ArkGuardStaticVersion";
constexpr std::string_view VERSION = "1.0.0.300";
}  // namespace ObfuscationToolConstants

namespace NameCacheConstants {
constexpr std::string_view OBF_NAME = "#obfName";
}  // namespace NameCacheConstants

}  // namespace ark::guard

#endif  // GUARD_NAME_CACHE_NAME_CACHE_CONSTANTS_H
