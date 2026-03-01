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

#include "libarkbase/mem/mem_config.h"

namespace ark::mem {

bool MemConfig::isInitialized_ = false;
size_t MemConfig::initialHeapSizeLimit_ = 0;
size_t MemConfig::heapSizeLimit_ = 0;
size_t MemConfig::internalMemorySizeLimit_ = 0;
size_t MemConfig::codeCacheSizeLimit_ = 0;
size_t MemConfig::compilerMemorySizeLimit_ = 0;
size_t MemConfig::framesMemorySizeLimit_ = 0;
size_t MemConfig::nativeStacksMemorySizeLimit_ = 0;
}  // namespace ark::mem
