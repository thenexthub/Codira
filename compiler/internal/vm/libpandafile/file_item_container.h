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

#ifndef LIBPANDAFILE_FILE_ITEM_CONTAINER_H
#define LIBPANDAFILE_FILE_ITEM_CONTAINER_H

#include "file_items.h"
#include "file_writer.h"
#include "pgo.h"

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace panda::panda_file {

class ItemDeduper;

class ItemContainer {
public:
    explicit ItemContainer();
    ~ItemContainer() = default;
    NO_COPY_SEMANTIC(ItemContainer);
    NO_MOVE_SEMANTIC(ItemContainer);

    StringItem *GetStringItem(const std::string &str) const;

    StringItem *GetOrCreateStringItem(const std::string &str);

    LiteralArrayItem *GetOrCreateLiteralArrayItem(const std::string &id);

    ClassItem *GetOrCreateClassItem(const std::string &str);

    ForeignClassItem *GetOrCreateForeignClassItem(const std::string &str);

    ScalarValueItem *GetOrCreateIntegerValueItem(uint32_t v);

    ScalarValueItem *GetOrCreateLongValueItem(uint64_t v);

    ScalarValueItem *GetOrCreateFloatValueItem(float v);

    ScalarValueItem *GetOrCreateDoubleValueItem(double v);

    ScalarValueItem *GetOrCreateIdValueItem(BaseItem *v);

    ClassItem *GetOrCreateGlobalClassItem()
    {
        return GetOrCreateClassItem("L_GLOBAL;");
    }

    ProtoItem *GetOrCreateProtoItem(TypeItem *ret_type, const std::vector<MethodParamItem> &params);

    PrimitiveTypeItem *GetOrCreatePrimitiveTypeItem(Type type);

    PrimitiveTypeItem *GetOrCreatePrimitiveTypeItem(Type::TypeId type);

    LineNumberProgramItem *CreateLineNumberProgramItem();

    template <class T, class... Args>
    T *CreateItem(Args &&... args)
    {
        static_assert(!std::is_same_v<T, StringItem>, "Use GetOrCreateStringItem to create StringItem");
        static_assert(!std::is_same_v<T, ClassItem>, "Use GetOrCreateClassItem to create ClassItem");
        static_assert(!std::is_same_v<T, ForeignClassItem>,
                      "Use GetOrCreateForeignClassItem to create ForeignClassItem");
        static_assert(!std::is_same_v<T, ValueItem>, "Use GetOrCreateValueItem functions to create ValueItem");
        static_assert(!std::is_same_v<T, ProtoItem>, "Use GetOrCreateProtoItem to create ValueItem");
        static_assert(!std::is_same_v<T, LineNumberProgramItem>,
                      "Use CreateLineNumberProgramItem to create LineNumberProgramItem");
        static_assert(!std::is_same_v<T, PrimitiveTypeItem>,
                      "Use GetOrCreatePrimitiveTypeItem to create PrimitiveTypeItem");
        static_assert(!std::is_same_v<T, MethodItem>, "Use ClassItem instance to create MethodItem");
        static_assert(!std::is_same_v<T, FieldItem>, "Use ClassItem instance to create FieldItem");

        std::unique_ptr<T> ptr = nullptr;
        if constexpr (std::is_same_v<T, ForeignFieldItem> || std::is_same_v<T, ForeignMethodItem> ||
            std::is_same_v<T, ScalarValueItem> || std::is_same_v<T, ArrayValueItem> ||
            std::is_same_v<T, LiteralArrayItem>) {
            ptr = std::make_unique<T>(std::forward<Args>(args)..., this);
        } else {
            ptr = std::make_unique<T>(std::forward<Args>(args)...);
        }

        auto ret = ptr.get();
        if (ptr->IsForeign()) {
            foreign_items_.emplace_back(std::move(ptr));
        } else {
            items_.insert(GetInsertPosition<T>(), std::move(ptr));
        }
        return ret;
    }

    void ReLayout();
    uint32_t ComputeLayout();
    bool Write(Writer *writer);

    std::map<std::string, size_t> GetStat();

    void DumpItemsStat(std::ostream &os) const;

    uint32_t CalculateRoundUpSize(uint32_t before, uint32_t after)
    {
        return after - before;
    }

    std::unordered_map<std::string, StringItem *> *GetStringMap()
    {
        return &string_map_;
    }

    LiteralArrayItem *GetLiteralArrayItem(const std::string &key)
    {
        return literalarray_map_.at(key);
    }

    std::map<std::string, BaseClassItem *> *GetClassMap()
    {
        return &class_map_;
    }

    std::unordered_map<uint32_t, ValueItem *> *GetIntValueMap()
    {
        return &int_value_map_;
    }

    std::unordered_map<uint64_t, ValueItem *> *GetLongValueMap()
    {
        return &long_value_map_;
    }

    std::unordered_map<uint32_t, ValueItem *> *GetFloatValueMap()
    {
        return &float_value_map_;
    }

    std::unordered_map<uint64_t, ValueItem *> *GetDoubleValueMap()
    {
        return &double_value_map_;
    }

    std::unordered_map<BaseItem *, ValueItem *> *GetScalarValueMap()
    {
        return &id_value_map_;
    }

    ProtoItem *GetProtoItem(TypeItem *retType, const std::vector<MethodParamItem> &params)
    {
        return proto_map_.at(ProtoKey {retType, params});
    }

    std::unordered_map<Type::TypeId, PrimitiveTypeItem *> *GetPrimitiveTypeMap()
    {
        return &primitive_type_map_;
    }

    std::list<std::unique_ptr<BaseItem>> &GetItems()
    {
        return items_;
    }

    const std::list<std::unique_ptr<BaseItem>> &GetItems() const
    {
        return items_;
    }

    const std::vector<std::unique_ptr<BaseItem>> &GetForeigtems()
    {
        return foreign_items_;
    }

    BaseItem *GetEndItem()
    {
        return end_;
    }

    void ReorderItems(panda::panda_file::pgo::ProfileOptimizer *profile_opt);

    void DeduplicateItems(bool computeLayout = true);

    void DeduplicateCodeAndDebugInfo();

    void DeduplicateAnnotations();

    void DeduplicateLineNumberProgram(DebugInfoItem *item, ItemDeduper *deduper);

    void DeduplicateDebugInfo(MethodItem *method, ItemDeduper *debug_info_deduper,
                              ItemDeduper *line_number_program_deduper);

    size_t GetIndexedItemCount() const
    {
        return indexed_item_count_;
    }

    void IncIndexedItemCount()
    {
        indexed_item_count_++;
    }

    static void SetApi(uint8_t api)
    {
        ItemContainer::apiVersion = api;
    }

    static void SetSubApi(std::string subApi)
    {
        ItemContainer::subApiVersion = subApi;
    }

    static std::string GetSubApi()
    {
        return ItemContainer::subApiVersion;
    }

    static uint8_t GetApi()
    {
        return ItemContainer::apiVersion;
    }

    static uint8_t apiVersion;
    static std::string subApiVersion;

private:
    template <class T>
    auto GetInsertPosition()
    {
        if (std::is_same_v<T, CodeItem>) {
            return code_items_end_;
        }

        if (std::is_same_v<T, DebugInfoItem>) {
            return debug_items_end_;
        }

        return items_end_;
    }

    class IndexItem : public BaseItem {
    public:
        IndexItem(IndexType type, size_t max_index) : type_(type), max_index_(max_index)
        {
            ASSERT(type_ != IndexType::NONE);

            const auto bc_version = GetVersionByApi(ItemContainer::GetApi(), ItemContainer::GetSubApi());
            if (bc_version.value().front() >= API_12 && (type == IndexType::FIELD || type == IndexType::PROTO)) {
                SetNeedsEmit(false);
            }
        }

        ~IndexItem() override = default;

        DEFAULT_COPY_SEMANTIC(IndexItem);
        NO_MOVE_SEMANTIC(IndexItem);

        size_t Alignment() override
        {
            return sizeof(uint32_t);
        }

        bool Write(Writer *writer) override;

        ItemTypes GetItemType() const override;

        bool Add(IndexedItem *item);

        bool Has(IndexedItem *item) const
        {
            auto res = index_.find(item);
            return res != index_.cend();
        }

        void Remove(IndexedItem *item)
        {
            index_.erase(item);
        }

        size_t GetNumItems() const
        {
            return index_.size();
        }

        void UpdateItems(BaseItem *start, BaseItem *end)
        {
            size_t i = 0;
            for (auto *item : index_) {
                item->SetIndex(start, end, i++);
            }
        }

        void Reset()
        {
            for (auto *item : index_) {
                item->ClearIndexes();
            }
        }

    protected:
        size_t CalculateSize() const override
        {
            if (NeedsEmit()) {
                return index_.size() * ID_SIZE;
            }
            return 0;
        }

    private:
        struct Comparator {
            bool operator()(IndexedItem *item1, IndexedItem *item2) const noexcept
            {
                auto index_type = item1->GetIndexType();
                if (index_type == IndexType::CLASS) {
                    auto type_item1 = static_cast<TypeItem *>(item1);
                    auto type_item2 = static_cast<TypeItem *>(item2);
                    auto type_id1 = static_cast<size_t>(type_item1->GetType().GetId());
                    auto type_id2 = static_cast<size_t>(type_item2->GetType().GetId());
                    if (type_id1 != type_id2) {
                        return type_id1 < type_id2;
                    }
                }

                if (index_type == IndexType::LINE_NUMBER_PROG) {
                    auto ref_count1 = item1->GetRefCount();
                    auto ref_count2 = item2->GetRefCount();
                    if (ref_count1 != ref_count2) {
                        return ref_count1 > ref_count2;
                    }
                }

                return item1->GetIndexedItemCount() < item2->GetIndexedItemCount();
            }
        };

        IndexType type_;
        size_t max_index_;
        std::set<IndexedItem *, Comparator> index_;
    };

    class LineNumberProgramIndexItem : public IndexItem {
    public:
        LineNumberProgramIndexItem() : IndexItem(IndexType::LINE_NUMBER_PROG, MAX_INDEX_32) {}
        ~LineNumberProgramIndexItem() override = default;
        DEFAULT_COPY_SEMANTIC(LineNumberProgramIndexItem);
        NO_MOVE_SEMANTIC(LineNumberProgramIndexItem);

        void IncRefCount(LineNumberProgramItem *item)
        {
            ASSERT(Has(item));
            Remove(item);
            item->IncRefCount();
            Add(item);
        }

        void DecRefCount(LineNumberProgramItem *item)
        {
            ASSERT(Has(item));
            Remove(item);
            item->DecRefCount();
            Add(item);
        }
    };

    class IndexHeaderItem : public BaseItem {
    public:
        explicit IndexHeaderItem(std::vector<IndexItem *> indexes) : indexes_(std::move(indexes))
        {
            ASSERT(indexes_.size() == INDEX_COUNT_16);
        }

        ~IndexHeaderItem() override = default;

        DEFAULT_COPY_SEMANTIC(IndexHeaderItem);
        NO_MOVE_SEMANTIC(IndexHeaderItem);

        size_t Alignment() override
        {
            return ID_SIZE;
        }

        bool Write(Writer *writer) override;

        ItemTypes GetItemType() const override
        {
            return ItemTypes::INDEX_HEADER;
        }

        bool Add(const std::list<IndexedItem *> &items);

        void Remove(const std::list<IndexedItem *> &items);

        void SetStart(BaseItem *item)
        {
            start_ = item;
        }

        void SetEnd(BaseItem *item)
        {
            end_ = item;
        }

        void UpdateItems()
        {
            for (auto *index : indexes_) {
                index->UpdateItems(start_, end_);
            }
        }

    protected:
        size_t CalculateSize() const override
        {
            return sizeof(File::IndexHeader);
        }

    private:
        IndexItem *IndexGetIndexByType(IndexType type) const
        {
            auto i = static_cast<size_t>(type);
            return indexes_[i];
        }

        BaseItem *start_ {nullptr};
        BaseItem *end_ {nullptr};
        std::vector<IndexItem *> indexes_;
    };

    class IndexSectionItem : public BaseItem {
    public:
        size_t Alignment() override
        {
            return ID_SIZE;
        }

        bool Write(Writer *writer) override;

        ItemTypes GetItemType() const override
        {
            return ItemTypes::INDEX_SECTION;
        }

        void Reset()
        {
            headers_.clear();

            for (auto &index : indexes_) {
                index.Reset();
            }

            indexes_.clear();
        }

        void AddHeader();

        IndexHeaderItem *GetCurrentHeader()
        {
            return &headers_.back();
        }

        bool IsEmpty() const
        {
            return headers_.empty();
        }

        size_t GetNumHeaders() const
        {
            return headers_.size();
        }

        void ComputeLayout() override;

        void UpdateItems()
        {
            for (auto &header : headers_) {
                header.UpdateItems();
            }
        }

    protected:
        size_t CalculateSize() const override;

    private:
        std::list<IndexHeaderItem> headers_;
        std::list<IndexItem> indexes_;
    };

    class ProtoKey {
    public:
        ProtoKey(TypeItem *ret_type, const std::vector<MethodParamItem> &params);

        ~ProtoKey() = default;

        DEFAULT_COPY_SEMANTIC(ProtoKey);
        NO_MOVE_SEMANTIC(ProtoKey);

        size_t GetHash() const
        {
            return hash_;
        }

        bool operator==(const ProtoKey &key) const
        {
            return shorty_ == key.shorty_ && ref_types_ == key.ref_types_;
        }

    private:
        void Add(TypeItem *item);

        size_t hash_;
        std::string shorty_;
        std::vector<TypeItem *> ref_types_;
    };

    struct ProtoKeyHash {
        size_t operator()(const ProtoKey &key) const noexcept
        {
            return key.GetHash();
        };
    };

    class EndItem : public BaseItem {
    public:
        EndItem()
        {
            SetNeedsEmit(false);
        }

        ~EndItem() override = default;

        DEFAULT_COPY_SEMANTIC(EndItem);
        NO_MOVE_SEMANTIC(EndItem);

        size_t CalculateSize() const override
        {
            return 0;
        }

        bool Write([[maybe_unused]] Writer *writer) override
        {
            return true;
        }

        ItemTypes GetItemType() const override
        {
            return ItemTypes::END_ITEM;
        }
    };

    bool WriteHeader(Writer *writer, ssize_t *checksum_offset);

    bool WriteItems(Writer *writer);

    bool WriteHeaderIndexInfo(Writer *writer);

    void RebuildIndexSection();

    void RebuildLineNumberProgramIndex();

    void UpdateOrderIndexes();

    void AddIndexDependecies(BaseItem *item);

    void ProcessIndexDependecies(BaseItem *item);

    size_t GetForeignOffset() const;

    size_t GetForeignSize() const;

    std::unordered_map<std::string, StringItem *> string_map_;
    std::unordered_map<std::string, LiteralArrayItem *> literalarray_map_;

    std::map<std::string, BaseClassItem *> class_map_;

    std::unordered_map<uint32_t, ValueItem *> int_value_map_;
    std::unordered_map<uint64_t, ValueItem *> long_value_map_;
    // NB! For f32 and f64 value maps we use integral keys
    // (in fact, bit patterns of corresponding values) to
    // workaround 0.0 == -0.0 semantics.
    std::unordered_map<uint32_t, ValueItem *> float_value_map_;
    std::unordered_map<uint64_t, ValueItem *> double_value_map_;
    std::unordered_map<BaseItem *, ValueItem *> id_value_map_;
    std::unordered_map<ProtoKey, ProtoItem *, ProtoKeyHash> proto_map_;
    std::unordered_map<Type::TypeId, PrimitiveTypeItem *> primitive_type_map_;

    std::list<std::unique_ptr<BaseItem>> items_;

    std::vector<std::unique_ptr<BaseItem>> foreign_items_;

    IndexSectionItem index_section_item_;

    LineNumberProgramIndexItem line_number_program_index_item_;

    std::list<std::unique_ptr<BaseItem>>::iterator items_end_;
    std::list<std::unique_ptr<BaseItem>>::iterator code_items_end_;
    std::list<std::unique_ptr<BaseItem>>::iterator debug_items_end_;

    // Get Roundup Alignment Size
    std::unordered_map<std::string, size_t> items_round_up_size_;
    uint32_t foreign_item_roundup_size_ {0};
    uint32_t line_number_item_roundup_size_ {0};

    BaseItem *end_;
    size_t indexed_item_count_ {0};
};

}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_FILE_ITEM_CONTAINER_H
