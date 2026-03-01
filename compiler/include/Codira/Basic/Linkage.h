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

/**
 * @file
 *
 * This file declares some common utility functions.
 */

#ifndef CODIRA_BASIC_LINKAGE_H
#define CODIRA_BASIC_LINKAGE_H

#include <cstdint>
namespace Codira {
/**
 * Represent the linkage of the Decl.
 */
enum class Linkage : uint8_t {
    WEAK_ODR,      /**< weak_odr linkage. */
    EXTERNAL,      /**< External linkage. */
    INTERNAL,      /**< Internal linkage. */
    LINKONCE_ODR,  /**< linkonce_odr linkage. */
    EXTERNAL_WEAK, /**< external_weak linkage */
};
} // namespace Codira

#endif // CODIRA_BASIC_LINKAGE_H
