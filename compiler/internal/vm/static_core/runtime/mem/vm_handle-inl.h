/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef RUNTIME_MEM_OBJECT_VMHANDLE_INL_H
#define RUNTIME_MEM_OBJECT_VMHANDLE_INL_H

#include "runtime/mem/vm_handle.h"
#include "runtime/mem/local_object_handle.h"
#include "runtime/mem/refstorage/global_object_storage.h"

namespace ark {

template <typename T>
template <typename P, typename>
inline VMHandle<T>::VMHandle(const LocalObjectHandle<P> &other) : HandleBase(other.GetAddress())
{
}

template <typename T>
inline VMHandle<T>::VMHandle(mem::GlobalObjectStorage *globalStorage, mem::Reference *reference)
    : HandleBase(globalStorage->GetAddressForRef(reference))
{
    ASSERT(reference != nullptr);
}

}  // namespace ark

#endif  // RUNTIME_MEM_OBJECT_VMHANDLE_INL_H
