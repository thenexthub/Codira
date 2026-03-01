/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "runtime/include/coretypes/string_flatten.h"
#include "runtime/include/object_header.h"
#include "common_interfaces/objects/base_object.h"
#include "common_interfaces/objects/string/base_string-inl.h"

#include "libarkbase/utils/utf.h"
#include "libarkbase/utils/hash.h"
#include "libarkbase/utils/span.h"
#include "runtime/arch/memory_helpers.h"
#include "runtime/include/runtime.h"
#include "runtime/handle_base-inl.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/flattened_string_cache.h"

namespace ark::coretypes {

/* static */
String *FlatStringInfo::SlowFlatten(VMHandle<String> &str, const LanguageContext &ctx)
{
    ASSERT(str->IsTreeString());
    PandaVM *vm = Runtime::GetCurrent()->GetPandaVM();

    uint32_t length = str->GetLength();
    bool compressed = str->IsUtf8();
    auto thread = ManagedThread::GetCurrent();

    String *result = String::AllocLineStringObject(thread, length, compressed, ctx, vm);
    if (result == nullptr) {
        return nullptr;
    }

    VMHandle<String> resultHandle(thread, result);
    auto readBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
    };

    if (compressed) {
        common::BaseString::WriteToFlat(std::move(readBarrier), str->ToString(), resultHandle->GetDataUtf8Writable(),
                                        length);
    } else {
        common::BaseString::WriteToFlat(std::move(readBarrier), str->ToString(), resultHandle->GetDataUtf16Writable(),
                                        length);
    }
    return resultHandle.GetPtr();
}

/* static */
String *FlatStringInfo::SlowFlattenWithNativeMemory(VMHandle<String> &str, const LanguageContext &ctx)
{
    ASSERT(str->IsTreeString());
    uint32_t length = str->GetLength();
    bool compressed = str->IsUtf8();
    auto stringClass = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::LINE_STRING);
    size_t size =
        compressed ? common::LineString::ComputeSizeUtf8(length) : common::LineString::ComputeSizeUtf16(length);
    uint8_t *p =
        Runtime::GetCurrent()->GetInternalAllocator()->New<uint8_t[]>(size);  // NOLINT(modernize-avoid-c-arrays)
    if (p == nullptr) {
        return nullptr;
    }

    auto pStr = reinterpret_cast<common::BaseString *>(p);
    // After setting length we should have a full barrier, so this write should happens-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    pStr->InitLengthAndFlags(length, compressed);
    pStr->SetMixHashcode(0);
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // Witout full memory barrier it is possible that architectures with weak memory order can try fetching string
    // legth before it's set
    arch::FullMemoryBarrier();

    reinterpret_cast<ObjectHeader *>(pStr)->SetClass(stringClass);
    String *lineStr = String::Cast(pStr);
    auto readBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
    };

    if (compressed) {
        common::BaseString::WriteToFlat(std::move(readBarrier), str->ToString(), lineStr->GetDataUtf8Writable(),
                                        length);
    } else {
        common::BaseString::WriteToFlat(std::move(readBarrier), str->ToString(), lineStr->GetDataUtf16Writable(),
                                        length);
    }
    return lineStr;
}

/* static */
FlatStringInfo FlatStringInfo::FlattenTreeString(VMHandle<String> &treeStr, const LanguageContext &ctx,
                                                 bool withNativeMemory)
{
    auto *thread = ManagedThread::GetCurrent();
    VMHandle<coretypes::Array> cache(thread, coretypes::Array::Cast(thread->GetFlattenedStringCache()));
    auto *cachedFlatStr = FlattenedStringCache::Get(cache.GetPtr(), treeStr.GetPtr());
    // cache hitted
    if (cachedFlatStr != nullptr) {
        return FlatStringInfo(cachedFlatStr, 0, cachedFlatStr->GetLength());
    }

    common::TreeString *treeString = treeStr->ToTreeString();
    auto readBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
    };
    if (treeString->IsFlat(std::move(readBarrier))) {
        auto readBarrierLeft = [](void *obj, size_t offset) {
            return reinterpret_cast<common::BaseString *>(
                ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
        };
        auto *first = String::Cast(treeString->GetLeftSubString<common::BaseString *>(std::move(readBarrierLeft)));
        FlattenedStringCache::Update(cache.GetPtr(), treeStr.GetPtr(), first);
        return FlatStringInfo(first, 0, treeString->GetLength());
    }

    // cached missed , no need to cache native memory string
    if (withNativeMemory) {
        String *s = SlowFlattenWithNativeMemory(treeStr, ctx);
        return FlatStringInfo(s, 0, s->GetLength(), true);
    }

    // cache it
    String *s = SlowFlatten(treeStr, ctx);
    // check for OOME
    if (s != nullptr) {
        FlattenedStringCache::Update(cache.GetPtr(), treeStr.GetPtr(), s);
    }
    return FlatStringInfo(s, 0, treeStr->GetLength());
}

/* static */
FlatStringInfo FlatStringInfo::FlattenSlicedString(VMHandle<String> &slicedStr)
{
    const common::SlicedString *slicedString = slicedStr->ToSlicedString();
    auto readBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<common::BaseString *>(ObjectAccessor::GetObject(const_cast<const void *>(obj), offset));
    };
    // NOLINTNEXTLINE(modernize-use-auto)
    common::BaseString *parent = slicedString->GetParent<common::BaseString *>(std::move(readBarrier));
    return FlatStringInfo(String::Cast(parent), slicedString->GetStartIndex(), slicedString->GetLength());
}

/* static */
FlatStringInfo FlatStringInfo::FlattenAllString(VMHandle<String> &str, const LanguageContext &ctx,
                                                bool withNativeMemory)
{
    String *string = str.GetPtr();
    // 1. LineString return directly
    if (string->IsLineString()) {
        return FlatStringInfo(string, 0, string->GetLength());
    }
    // 2. SlicedString
    if (string->IsSlicedString()) {
        return FlattenSlicedString(str);
    }
    // 3. TreeString
    if (string->IsTreeString()) {
        return FlattenTreeString(str, ctx, withNativeMemory);
    }
    UNREACHABLE();
}

FlatStringInfo::FlatStringInfo(FlatStringInfo &&info) noexcept
    : string_(info.string_),
      startIndex_(info.startIndex_),
      length_(info.length_),
      withNativeMemory_(info.withNativeMemory_)
{
    info.string_ = nullptr;
    info.startIndex_ = 0;
    info.length_ = 0;
    info.withNativeMemory_ = false;
}

FlatStringInfo &FlatStringInfo::operator=(FlatStringInfo &&info) noexcept
{
    if (this == &info) {
        return *this;
    }

    this->string_ = info.string_;
    this->startIndex_ = info.startIndex_;
    this->length_ = info.length_;
    this->withNativeMemory_ = info.withNativeMemory_;

    info.string_ = nullptr;
    info.startIndex_ = 0;
    info.length_ = 0;
    info.withNativeMemory_ = false;

    return *this;
}

FlatStringInfo::~FlatStringInfo()
{
    if (withNativeMemory_) {
        Runtime::GetCurrent()->GetInternalAllocator()->DeleteArray(reinterpret_cast<uint8_t *>(string_));
    }
}

}  // namespace ark::coretypes
