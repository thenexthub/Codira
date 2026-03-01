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
#ifndef PANDA_RUNTIME_FLATTENED_STRING_CACHE_H
#define PANDA_RUNTIME_FLATTENED_STRING_CACHE_H

#include "runtime/include/coretypes/string.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"

namespace ark {
class FlattenedStringCache {
public:
    FlattenedStringCache() = delete;

    static coretypes::Array *Create(PandaVM *vm)
    {
        auto *linker = Runtime::GetCurrent()->GetClassLinker();
        auto *ext = linker->GetExtension(vm->GetLanguageContext());
        auto *klass = ext->GetClassRoot(ClassRoot::ARRAY_STRING);
        return coretypes::Array::Create(klass, ENTRY_SIZE * SIZE);
    }

    static coretypes::String *Get(coretypes::Array *cache, coretypes::String *treeStr)
    {
        if (cache == nullptr) {
            ASSERT(PandaVM::GetCurrent()->GetLanguageContext().GetLanguage() == SourceLanguage::PANDA_ASSEMBLY);
            return nullptr;
        }
        auto index = GetIndex(treeStr);
        auto *key = GetKey(cache, index);
        if (key != treeStr) {
            return nullptr;
        }
        return GetValue(cache, index);
    }

    static void Update(coretypes::Array *cache, coretypes::String *treeStr, coretypes::String *flatStr)
    {
        if (cache == nullptr) {
            ASSERT(PandaVM::GetCurrent()->GetLanguageContext().GetLanguage() == SourceLanguage::PANDA_ASSEMBLY);
            return;
        }
        ASSERT(cache != nullptr);
        ASSERT(treeStr != nullptr);
        ASSERT(flatStr != nullptr);
        auto index = GetIndex(treeStr);
        SetKey(cache, index, treeStr);
        SetValue(cache, index, flatStr);
    }

    static constexpr size_t GetAddressShift()
    {
        return SHIFT;
    }

    static constexpr size_t GetAddressMask()
    {
        return MASK;
    }

    static constexpr size_t GetEntrySize()
    {
        return ENTRY_SIZE;
    }

    static constexpr size_t GetValueOffset()
    {
        return VALUE_OFFSET;
    }

private:
    static constexpr size_t SHIFT = 3;
    static constexpr size_t MASK = 0x3f;
    static constexpr size_t SIZE = MASK + 1;
    static constexpr size_t ENTRY_SIZE = 2;
    static constexpr size_t VALUE_OFFSET = 1;

    static size_t GetIndex(coretypes::String *treeStr)
    {
        return ((ToUintPtr(treeStr) >> SHIFT) & MASK) * ENTRY_SIZE;
    }

    static coretypes::String *GetKey(coretypes::Array *cache, size_t index)
    {
        return cache->Get<coretypes::String *>(index);
    }

    static void SetKey(coretypes::Array *cache, size_t index, coretypes::String *key)
    {
        cache->Set(index, key);
    }

    static coretypes::String *GetValue(coretypes::Array *cache, size_t index)
    {
        return cache->Get<coretypes::String *>(index + VALUE_OFFSET);
    }

    static void SetValue(coretypes::Array *cache, size_t index, coretypes::String *value)
    {
        cache->Set(index + VALUE_OFFSET, value);
    }
};
}  // namespace ark

#endif  // PANDA_RUNTIME_FLATTENED_STRING_CACHE_H
