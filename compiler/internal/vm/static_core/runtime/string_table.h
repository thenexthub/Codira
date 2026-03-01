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
#ifndef PANDA_RUNTIME_STRING_TABLE_H_
#define PANDA_RUNTIME_STRING_TABLE_H_

#include <cstdint>

#include "libarkbase/mem/mem.h"
#include "libarkbase/os/mutex.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/language_context.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark {

namespace mem::test {
class MultithreadedInternStringTableTest;
}  // namespace mem::test

class PANDA_PUBLIC_API StringTable {
public:
    explicit StringTable(mem::InternalAllocatorPtr allocator) : internalTable_(allocator), table_(allocator) {}
    StringTable() = default;
    virtual ~StringTable() = default;

    virtual coretypes::String *GetOrInternString(const uint8_t *mutf8Data, uint32_t utf16Length,
                                                 const LanguageContext &ctx);
    virtual coretypes::String *GetOrInternString(const uint16_t *utf16Data, uint32_t utf16Length,
                                                 const LanguageContext &ctx);
    coretypes::String *GetOrInternString(coretypes::String *string, const LanguageContext &ctx);

    coretypes::String *GetOrInternInternalString(const panda_file::File &pf, panda_file::File::EntityId id,
                                                 const LanguageContext &ctx);

    coretypes::String *GetInternalStringFast(const panda_file::File &pf, panda_file::File::EntityId id)
    {
        return internalTable_.GetStringFast(pf, id);
    }

    using StringVisitor = std::function<void(ObjectHeader *)>;

    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    void VisitRoots(GCRootVisitor visitor, mem::VisitGCRootFlags flags = mem::VisitGCRootFlags::ACCESS_ROOT_ALL)
    {
        internalTable_.VisitRoots(visitor, flags);
    }

    void VisitStrings(const StringVisitor &visitor)
    {
        table_.VisitStrings(visitor);
    }

    virtual void Sweep(const GCObjectVisitor &gcObjectVisitor);

    void UpdateAndSweep(const ReferenceUpdater &updater)
    {
        return table_.UpdateAndSweep(updater);
    }

    size_t Size();

protected:
    class PANDA_PUBLIC_API Table {
    public:
        explicit Table(mem::InternalAllocatorPtr allocator) : table_(allocator->Adapter()) {}
        Table() = default;
        virtual ~Table() = default;

        void PreBarrierOnGet(coretypes::String *str);

        virtual coretypes::String *GetOrInternString(const uint8_t *mutf8Data, uint32_t utf16Length,
                                                     bool canBeCompressed, const LanguageContext &ctx);
        virtual coretypes::String *GetOrInternString(const uint16_t *utf16Data, uint32_t utf16Length,
                                                     const LanguageContext &ctx);
        virtual coretypes::String *GetOrInternString(coretypes::String *string, const LanguageContext &ctx);
        virtual void Sweep(const GCObjectVisitor &gcObjectVisitor);

        void UpdateAndSweep(const ReferenceUpdater &updater);

        void VisitStrings(const StringVisitor &visitor);

        size_t Size();

        coretypes::String *GetString(const uint8_t *utf8Data, uint32_t utf16Length, bool canBeCompressed,
                                     const LanguageContext &ctx);
        coretypes::String *GetString(const uint16_t *utf16Data, uint32_t utf16Length, const LanguageContext &ctx);
        coretypes::String *GetString(coretypes::String *string, const LanguageContext &ctx);

        coretypes::String *InternString(coretypes::String *string, const LanguageContext &ctx);
        void ForceInternString(coretypes::String *string, const LanguageContext &ctx);

    protected:
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        PandaUnorderedMultiMap<uint32_t, coretypes::String *> table_ GUARDED_BY(tableLock_) {};
        os::memory::RWLock tableLock_;  // NOLINT(misc-non-private-member-variables-in-classes)

    private:
        NO_COPY_SEMANTIC(Table);
        NO_MOVE_SEMANTIC(Table);

        // Required to clear intern string in test
        friend class mem::test::MultithreadedInternStringTableTest;
    };

    class PANDA_PUBLIC_API InternalTable : public Table {
    public:
        InternalTable() = default;
        explicit InternalTable(mem::InternalAllocatorPtr allocator)
            : Table(allocator), newStringTable_(allocator->Adapter())
        {
        }
        ~InternalTable() override = default;

        void PreBarrierOnGet(coretypes::String *str) = delete;

        coretypes::String *GetOrInternString(const uint8_t *mutf8Data, uint32_t utf16Length, bool canBeCompressed,
                                             const LanguageContext &ctx) override;

        coretypes::String *GetOrInternString(const uint16_t *utf16Data, uint32_t utf16Length,
                                             const LanguageContext &ctx) override;

        coretypes::String *GetOrInternString(coretypes::String *string, const LanguageContext &ctx) override;

        coretypes::String *GetOrInternString(const panda_file::File &pf, panda_file::File::EntityId id,
                                             const LanguageContext &ctx);

        coretypes::String *GetStringFast(const panda_file::File &pf, panda_file::File::EntityId id);

        void VisitRoots(const GCRootVisitor &visitor,
                        mem::VisitGCRootFlags flags = mem::VisitGCRootFlags::ACCESS_ROOT_ALL);

    protected:
        coretypes::String *InternStringNonMovable(coretypes::String *string, const LanguageContext &ctx);

    private:
        bool recordNewString_ {false};
        PandaVector<coretypes::String *> newStringTable_ GUARDED_BY(tableLock_) {};
        class EntityIdEqual {
        public:
            uint32_t operator()(const panda_file::File::EntityId &id) const
            {
                return id.GetOffset();
            }
        };
        PandaUnorderedMap<const panda_file::File *,
                          PandaUnorderedMap<panda_file::File::EntityId, coretypes::String *, EntityIdEqual>>
            maps_ GUARDED_BY(mapsLock_);

        os::memory::RWLock mapsLock_;

        NO_COPY_SEMANTIC(InternalTable);
        NO_MOVE_SEMANTIC(InternalTable);
    };

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    InternalTable internalTable_;  // Used for string in panda file.
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    Table table_;

private:
    NO_COPY_SEMANTIC(StringTable);
    NO_MOVE_SEMANTIC(StringTable);

    // Required to clear intern string in test
    friend class mem::test::MultithreadedInternStringTableTest;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_STRING_TABLE_H_
