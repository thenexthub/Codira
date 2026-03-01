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
#ifndef PANDA_PLUGINS_ETS_COMPILER_CONSTANTS_INL_H
#define PANDA_PLUGINS_ETS_COMPILER_CONSTANTS_INL_H

// The measured runtime for Array.reverse of 1100 elements is well below the 20ms threshold
// If array len more than MAGIC_NUM_FOR_SHORT_ARRAY_THRESHOLD, go to the slow path
static constexpr size_t MAGIC_NUM_FOR_SHORT_ARRAY_THRESHOLD = 1100;

#endif  // PANDA_PLUGINS_ETS_COMPILER_CONSTANTS_INL_H
