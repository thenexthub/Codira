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
#ifndef RUNTIME_INCLUDE_TAIHE_OPTIONAL_ABI_H_
#define RUNTIME_INCLUDE_TAIHE_OPTIONAL_ABI_H_

#include <taihe/common.h>

// TOptional
// Represents an optional value structure containing a pointer to the data.
// # Members
// - `m_data`: A pointer to the data in the optional value.
struct TOptional {
    void const *m_data;
};
#endif  // RUNTIME_INCLUDE_TAIHE_OPTIONAL_ABI_H_