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

#include "runtime/string_table.h"

#include "runtime/include/runtime.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/include/coretypes/string.h"

namespace ark {

coretypes::String *StringTable::GetOrInternString(const uint8_t *mutf8Data, uint32_t utf16Length,
                                                  const LanguageContext &ctx)
{
    bool canBeCompressed = coretypes::String::CanBeCompressedMUtf8(mutf8Data);
    auto *str = internalTable_.GetString(mutf8Data, utf16Length, canBeCompressed, ctx);
    if (str == nullptr) {
        str = table_.GetOrInternString(mutf8Data, utf16Length, canBeCompressed, ctx);
    }
    return str;
}

coretypes::String *StringTable::GetOrInternString(const uint16_t *utf16Data, uint32_t utf16Length,
                                                  const LanguageContext &ctx)
{
    auto *str = internalTable_.GetString(utf16Data, utf16Length, ctx);
    if (str == nullptr) {
        str = table_.GetOrInternString(utf16Data, utf16Length, ctx);
    }
    return str;
}

coretypes::String *StringTable::GetOrInternString(coretypes::String *string, const LanguageContext &ctx)
{
    auto *str = internalTable_.GetString(string, ctx);
    if (str == nullptr) {
        str = table_.GetOrInternString(string, ctx);
    }
    return str;
}

coretypes::String *StringTable::GetOrInternInternalString(const panda_file::File &pf, panda_file::File::EntityId id,
                                                          const LanguageContext &ctx)
{
    auto data = pf.GetStringData(id);

    coretypes::String *str = table_.GetString(data.data, data.utf16Length, data.isAscii, ctx);
    if (str != nullptr) {
        table_.PreBarrierOnGet(str);
        return str;
    }
    return internalTable_.GetOrInternString(pf, id, ctx);
}

void StringTable::Sweep(const GCObjectVisitor &gcObjectVisitor)
{
    table_.Sweep(gcObjectVisitor);
}

size_t StringTable::Size()
{
    return internalTable_.Size() + table_.Size();
}

void StringTable::Table::VisitStrings(const StringVisitor &visitor)
{
    os::memory::ReadLockHolder holder(tableLock_);
    for (auto entry : table_) {
        visitor(entry.second);
    }
}

coretypes::String *StringTable::Table::GetString(const uint8_t *utf8Data, uint32_t utf16Length, bool canBeCompressed,
                                                 [[maybe_unused]] const LanguageContext &ctx)
{
    uint32_t hashCode = coretypes::String::ComputeHashcodeMutf8(utf8Data, utf16Length, canBeCompressed);
    os::memory::ReadLockHolder holder(tableLock_);
    for (auto it = table_.find(hashCode); it != table_.end(); it++) {
        auto foundString = it->second;
        if (coretypes::String::StringsAreEqualMUtf8(foundString, utf8Data, utf16Length, canBeCompressed)) {
            return foundString;
        }
    }
    return nullptr;
}

coretypes::String *StringTable::Table::GetString(const uint16_t *utf16Data, uint32_t utf16Length,
                                                 [[maybe_unused]] const LanguageContext &ctx)
{
    uint32_t hashCode = coretypes::String::ComputeHashcodeUtf16(utf16Data, utf16Length);
    os::memory::ReadLockHolder holder(tableLock_);
    for (auto it = table_.find(hashCode); it != table_.end(); it++) {
        auto foundString = it->second;
        if (coretypes::String::StringsAreEqualUtf16(foundString, utf16Data, utf16Length)) {
            return foundString;
        }
    }
    return nullptr;
}

coretypes::String *StringTable::Table::GetString(coretypes::String *string, [[maybe_unused]] const LanguageContext &ctx)
{
    ASSERT(string != nullptr);
    os::memory::ReadLockHolder holder(tableLock_);
    auto hash = coretypes::String::Cast(string)->GetHashcode();
    for (auto it = table_.find(hash); it != table_.end(); it++) {
        auto foundString = it->second;
        if (coretypes::String::StringsAreEqual(foundString, string)) {
            return foundString;
        }
    }
    return nullptr;
}

void StringTable::Table::ForceInternString(coretypes::String *string, [[maybe_unused]] const LanguageContext &ctx)
{
    os::memory::WriteLockHolder holder(tableLock_);
    table_.insert(std::pair<uint32_t, coretypes::String *>(coretypes::String::Cast(string)->GetHashcode(), string));
}

coretypes::String *StringTable::Table::InternString(coretypes::String *string,
                                                    [[maybe_unused]] const LanguageContext &ctx)
{
    ASSERT(string != nullptr);
    uint32_t hashCode = coretypes::String::Cast(string)->GetHashcode();
    os::memory::WriteLockHolder holder(tableLock_);
    // Check string is not present before actually creating and inserting
    for (auto it = table_.find(hashCode); it != table_.end(); it++) {
        auto foundString = it->second;
        if (coretypes::String::StringsAreEqual(foundString, string)) {
            return foundString;
        }
    }
    table_.insert(std::pair<uint32_t, coretypes::String *>(hashCode, string));
    return string;
}

void StringTable::Table::PreBarrierOnGet(coretypes::String *str)
{
    // Need pre barrier if string exists in string table, because this string can be got from the
    // string table (like phoenix) and write to a field during concurrent phase and GC does not see it on Remark
    ASSERT_MANAGED_CODE();
    auto *preWrb = Thread::GetCurrent()->GetPreWrbEntrypoint();
    if (preWrb != nullptr) {
        reinterpret_cast<mem::ObjRefProcessFunc>(preWrb)(str);
    }
}

coretypes::String *StringTable::Table::GetOrInternString(const uint8_t *mutf8Data, uint32_t utf16Length,
                                                         bool canBeCompressed, const LanguageContext &ctx)
{
    coretypes::String *result = GetString(mutf8Data, utf16Length, canBeCompressed, ctx);
    if (result != nullptr) {
        PreBarrierOnGet(result);
        return result;
    }

    // Even if this string is not inserted, it should get removed during GC
    result =
        coretypes::String::CreateFromMUtf8(mutf8Data, utf16Length, canBeCompressed, ctx, Thread::GetCurrent()->GetVM());
    if (UNLIKELY(result == nullptr)) {
        return nullptr;
    }
    result = InternString(result, ctx);

    return result;
}

coretypes::String *StringTable::Table::GetOrInternString(const uint16_t *utf16Data, uint32_t utf16Length,
                                                         const LanguageContext &ctx)
{
    coretypes::String *result = GetString(utf16Data, utf16Length, ctx);
    if (result != nullptr) {
        PreBarrierOnGet(result);
        return result;
    }

    // Even if this string is not inserted, it should get removed during GC
    result = coretypes::String::CreateFromUtf16(utf16Data, utf16Length, ctx, Thread::GetCurrent()->GetVM());
    if (UNLIKELY(result == nullptr)) {
        return nullptr;
    }

    result = InternString(result, ctx);

    return result;
}

coretypes::String *StringTable::Table::GetOrInternString(coretypes::String *string, const LanguageContext &ctx)
{
    coretypes::String *result = GetString(string, ctx);
    if (result != nullptr) {
        PreBarrierOnGet(result);
        return result;
    }
    result = InternString(string, ctx);
    return result;
}

void StringTable::Table::UpdateAndSweep(const ReferenceUpdater &updater)
{
    os::memory::WriteLockHolder holder(tableLock_);
    auto it = table_.begin();
    while (it != table_.end()) {
        ObjectHeader *prevObject = it->second;
        if (updater(reinterpret_cast<ObjectHeader **>(&it->second)) == ObjectStatus::ALIVE_OBJECT) {
            if (it->second != prevObject) {
                LOG(DEBUG, GC) << "StringTable: forwarded " << prevObject << " -> " << it->second;
            }
            ++it;
        } else {
            table_.erase(it++);
            LOG(DEBUG, GC) << "StringTable: delete " << prevObject;
        }
    }
}

// NOTE(alovkov): make parallel
void StringTable::Table::Sweep(const GCObjectVisitor &gcObjectVisitor)
{
    os::memory::WriteLockHolder holder(tableLock_);
    LOG(DEBUG, GC) << "=== StringTable Sweep. BEGIN ===";
    LOG(DEBUG, GC) << "StringTable iterate over: " << table_.size() << " elements in string table";
    for (auto it = table_.begin(), end = table_.end(); it != end;) {
        auto *object = it->second;
        if (gcObjectVisitor(object) == ObjectStatus::ALIVE_OBJECT) {
            // All references in the string table must be updated before.
            ASSERT(!object->IsForwarded());
            ++it;
        } else if (gcObjectVisitor(object) == ObjectStatus::DEAD_OBJECT) {
            LOG(DEBUG, GC) << "StringTable: delete string " << std::hex << object
                           << ", val = " << ConvertToString(object);
            table_.erase(it++);
        }
    }
    LOG(DEBUG, GC) << "StringTable size after sweep = " << table_.size();
    LOG(DEBUG, GC) << "=== StringTable Sweep. END ===";
}

size_t StringTable::Table::Size()
{
    os::memory::ReadLockHolder holder(tableLock_);
    return table_.size();
}

coretypes::String *StringTable::InternalTable::GetOrInternString(const uint8_t *mutf8Data, uint32_t utf16Length,
                                                                 bool canBeCompressed, const LanguageContext &ctx)
{
    coretypes::String *result = GetString(mutf8Data, utf16Length, canBeCompressed, ctx);
    if (result != nullptr) {
        return result;
    }

    result = coretypes::String::CreateFromMUtf8(mutf8Data, utf16Length, canBeCompressed, ctx,
                                                Thread::GetCurrent()->GetVM(), false);
    if (UNLIKELY(result == nullptr)) {
        return nullptr;
    }
    result = InternStringNonMovable(result, ctx);
    return result;
}

coretypes::String *StringTable::InternalTable::GetOrInternString(const uint16_t *utf16Data, uint32_t utf16Length,
                                                                 const LanguageContext &ctx)
{
    coretypes::String *result = GetString(utf16Data, utf16Length, ctx);
    if (result != nullptr) {
        return result;
    }

    result = coretypes::String::CreateFromUtf16(utf16Data, utf16Length, ctx, Thread::GetCurrent()->GetVM(), false);
    if (UNLIKELY(result == nullptr)) {
        return nullptr;
    }
    result = InternStringNonMovable(result, ctx);
    return result;
}

coretypes::String *StringTable::InternalTable::GetOrInternString(coretypes::String *string, const LanguageContext &ctx)
{
    coretypes::String *result = GetString(string, ctx);
    if (result != nullptr) {
        return result;
    }
    result = InternString(string, ctx);
    return result;
}

coretypes::String *StringTable::InternalTable::GetOrInternString(const panda_file::File &pf,
                                                                 panda_file::File::EntityId id,
                                                                 const LanguageContext &ctx)
{
    auto data = pf.GetStringData(id);
    coretypes::String *result = GetString(data.data, data.utf16Length, data.isAscii, ctx);
    if (result != nullptr) {
        return result;
    }
    result = coretypes::String::CreateFromMUtf8(data.data, data.utf16Length, data.isAscii, ctx,
                                                Thread::GetCurrent()->GetVM(), false);
    if (UNLIKELY(result == nullptr)) {
        return nullptr;
    }

    result = InternStringNonMovable(result, ctx);

    // Update cache.
    os::memory::WriteLockHolder lock(mapsLock_);
    auto it = maps_.find(&pf);
    if (it != maps_.end()) {
        (it->second)[id] = result;
    } else {
        PandaUnorderedMap<panda_file::File::EntityId, coretypes::String *, EntityIdEqual> map;
        map[id] = result;
        maps_[&pf] = std::move(map);
    }
    return result;
}

coretypes::String *StringTable::InternalTable::GetStringFast(const panda_file::File &pf, panda_file::File::EntityId id)
{
    os::memory::ReadLockHolder lock(mapsLock_);
    auto it = maps_.find(&pf);
    if (it != maps_.end()) {
        auto idIt = it->second.find(id);
        if (idIt != it->second.end()) {
            return idIt->second;
        }
    }
    return nullptr;
}

void StringTable::InternalTable::VisitRoots(const GCRootVisitor &visitor, mem::VisitGCRootFlags flags)
{
    ASSERT(BitCount(flags & (mem::VisitGCRootFlags::ACCESS_ROOT_ALL | mem::VisitGCRootFlags::ACCESS_ROOT_ONLY_NEW)) ==
           1);

    ASSERT(BitCount(flags & (mem::VisitGCRootFlags::START_RECORDING_NEW_ROOT |
                             mem::VisitGCRootFlags::END_RECORDING_NEW_ROOT)) <= 1);
    // need to set flags before we iterate, because concurrent allocation should be in proper table
    if ((flags & mem::VisitGCRootFlags::START_RECORDING_NEW_ROOT) != 0) {
        os::memory::WriteLockHolder holder(tableLock_);
        recordNewString_ = true;
    } else if ((flags & mem::VisitGCRootFlags::END_RECORDING_NEW_ROOT) != 0) {
        os::memory::WriteLockHolder holder(tableLock_);
        recordNewString_ = false;
    }

    if ((flags & mem::VisitGCRootFlags::ACCESS_ROOT_ALL) != 0) {
        os::memory::ReadLockHolder lock(tableLock_);
        for (auto &v : table_) {
            visitor({mem::RootType::STRING_TABLE, reinterpret_cast<ObjectHeader **>(&v.second)});
        }
    } else if ((flags & mem::VisitGCRootFlags::ACCESS_ROOT_ONLY_NEW) != 0) {
        os::memory::ReadLockHolder lock(tableLock_);
        for (auto &str : newStringTable_) {
            visitor({mem::RootType::STRING_TABLE, reinterpret_cast<ObjectHeader **>(&str)});
        }
    } else {
        LOG(FATAL, RUNTIME) << "Unknown VisitGCRootFlags: " << static_cast<uint32_t>(flags);
    }
    if ((flags & mem::VisitGCRootFlags::END_RECORDING_NEW_ROOT) != 0) {
        os::memory::WriteLockHolder holder(tableLock_);
        newStringTable_.clear();
    }
}

coretypes::String *StringTable::InternalTable::InternStringNonMovable(coretypes::String *string,
                                                                      const LanguageContext &ctx)
{
    auto *result = InternString(string, ctx);
    os::memory::WriteLockHolder holder(tableLock_);
    if (recordNewString_) {
        newStringTable_.push_back(result);
    }
    return result;
}

}  // namespace ark
