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

#ifndef PANDA_RUNTIME_MEM_GC_GC_ROOT_INL_H
#define PANDA_RUNTIME_MEM_GC_GC_ROOT_INL_H

#include "runtime/mem/gc/gc_root.h"

namespace ark::mem {

template <class LanguageConfig>
std::ostream &operator<<(std::ostream &os, const RootManager<LanguageConfig> &rootManager)
{
    os << "Dump of roots " << std::endl;
    rootManager.VisitNonHeapRoots([&os](const GCRoot &gcRoot) { os << gcRoot; });
    return os;
}

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_ROOT_INL_H
