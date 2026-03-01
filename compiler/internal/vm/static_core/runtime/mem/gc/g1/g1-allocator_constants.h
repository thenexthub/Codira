/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RUNTIME_MEM_GC_G1_G1_ALLOCATOR_CONSTANTS_H
#define RUNTIME_MEM_GC_G1_G1_ALLOCATOR_CONSTANTS_H

#include "runtime/mem/region_allocator.h"

namespace ark::mem {
constexpr size_t G1_REGION_SIZE = ark::mem::RegionAllocator<ark::mem::ObjectAllocConfig>::REGION_SIZE;
}  // namespace ark::mem

#endif  // RUNTIME_MEM_GC_G1_G1_ALLOCATOR_CONSTANTS_H