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

#ifndef LIBPANDAFILE_FILE_ITEM_CONTAINER_H_
#define LIBPANDAFILE_FILE_ITEM_CONTAINER_H_

#include "libarkfile/file_items.h"
#include "libarkfile/file_writer.h"
#include "libarkfile/pgo.h"

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace ark::panda_file {

class ItemDeduper;

class ItemContainer {
public:
    PANDA_PUBLIC_API ItemContainer();
    ~ItemContainer() = default;
    NO_COPY_SEMANTIC(ItemContainer);
    NO_MOVE_SEMANTIC(ItemContainer);

    constexpr static std::string_view GetGlobalClassName()
    {
        return "L_GLOBAL;";
    }

    PANDA_PUBLIC_API StringItem *GetOrCreateStringItem(const std::string &str);

    PANDA_PUBLIC_API LiteralArrayItem *GetOrCreateLiteralArrayItem(const std::string &id);

    PANDA_PUBLIC_API ClassItem *GetOrCreateClassItem(const std::string &str);

    PANDA_PUBLIC_API ForeignClassItem *GetOrCreateForeignClassItem(const std::string &str);

    PANDA_PUBLIC_API ScalarValueItem *GetOrCreateIntegerValueItem(uint32_t v);

    PANDA_PUBLIC_API ScalarValueItem *GetOrCreateLongValueItem(uint64_t v);

    PANDA_PUBLIC_API ScalarValueItem *GetOrCreateFloatValueItem(float v);

    PANDA_PUBLIC_API ScalarValueItem *GetOrCreateDoubleValueItem(double v);

    ScalarValueItem *GetOrCreateIdValueItem(BaseItem *v);

    ClassItem *GetOrCreateGlobalClassItem()
    {
        return GetOrCreateClassItem(std::string(GetGlobalClassName()));
    }

    PANDA_PUBLIC_API ProtoItem *GetOrCreateProtoItem(TypeItem *retType, const std::vector<MethodParamItem> &params);

    PrimitiveTypeItem *GetOrCreatePrimitiveTypeItem(Type type);

    PANDA_PUBLIC_API PrimitiveTypeItem *GetOrCreatePrimitiveTypeItem(Type::TypeId type);

    PANDA_PUBLIC_API LineNumberProgramItem *CreateLineNumberProgramItem();

    void IncRefLineNumberProgramItem(LineNumberProgramItem *it);

    void SetQuickened()
    {
        isQuickened_ = true;
    }

    bool IsQuickened() const
    {
        return isQuickened_;
    }

    template <class T, class... Args>
    T *CreateItem(Args &&...args)
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

        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        auto ret = ptr.get();
        if (ptr->IsForeign()) {
            foreignItems_.emplace_back(std::move(ptr));
        } else {
            items_.insert(GetInsertPosition<T>(), std::move(ptr));
        }
        return ret;
    }

    PANDA_PUBLIC_API void FillExportMap();
    PANDA_PUBLIC_API uint32_t ComputeLayout();
    PANDA_PUBLIC_API void MarkLiteralarrayMap();
    PANDA_PUBLIC_API void DeleteReferenceFromAnno(AnnotationItem *annoItem);
    PANDA_PUBLIC_API void CleanupArrayValueItems(ValueItem *value);
    PANDA_PUBLIC_API uint32_t DeleteItems();
    PANDA_PUBLIC_API uint32_t DeleteForeignItems();

    PANDA_PUBLIC_API bool Write(Writer *writer, bool deduplicateItems = true, bool computeLayout = true);

    PANDA_PUBLIC_API std::map<std::string, size_t> GetStat();

    void DumpItemsStat(std::ostream &os) const;

    std::unordered_map<std::string, StringItem *> *GetStringMap()
    {
        return &stringMap_;
    }

    LiteralArrayItem *GetLiteralArrayItem(const std::string &key)
    {
        return literalarrayMap_.at(key);
    }

    std::map<std::string, BaseClassItem *> *GetClassMap()
    {
        return &classMap_;
    }

    std::map<std::string, BaseClassItem *> *GetExportMap()
    {
        return &exportMap_;
    }

    std::unordered_map<uint32_t, ValueItem *> *GetIntValueMap()
    {
        return &intValueMap_;
    }

    std::unordered_map<uint64_t, ValueItem *> *GetLongValueMap()
    {
        return &longValueMap_;
    }

    std::unordered_map<uint32_t, ValueItem *> *GetFloatValueMap()
    {
        return &floatValueMap_;
    }

    std::unordered_map<uint64_t, ValueItem *> *GetDoubleValueMap()
    {
        return &doubleValueMap_;
    }

    std::unordered_map<BaseItem *, ValueItem *> *GetScalarValueMap()
    {
        return &idValueMap_;
    }

    ProtoItem *GetProtoItem(TypeItem *retType, const std::vector<MethodParamItem> &params)
    {
        return protoMap_.at(ProtoKey {retType, params});
    }

    std::unordered_map<Type::TypeId, PrimitiveTypeItem *> *GetPrimitiveTypeMap()
    {
        return &primitiveTypeMap_;
    }

    const std::list<std::unique_ptr<BaseItem>> &GetItems() const
    {
        return items_;
    }

    const std::vector<std::unique_ptr<BaseItem>> &GetForeignItems()
    {
        return foreignItems_;
    }

    BaseItem *GetEndItem()
    {
        return end_;
    }

    PANDA_PUBLIC_API void ReorderItems(ark::panda_file::pgo::ProfileOptimizer *profileOpt);

    void DeduplicateItems(bool computeLayout = true);

    void DeduplicateCodeAndDebugInfo();

    void DeduplicateAnnotations();

    void DeduplicateLineNumberProgram(DebugInfoItem *item, ItemDeduper *deduper);

    void DeduplicateDebugInfo(MethodItem *method, ItemDeduper *debugInfoDeduper, ItemDeduper *lineNumberProgramDeduper);

private:
    template <class T>
    auto GetInsertPosition()
    {
        if (std::is_same_v<T, CodeItem>) {
            return codeItemsEnd_;
        }

        if (std::is_same_v<T, DebugInfoItem>) {
            return debugItemsEnd_;
        }

        if (std::is_same_v<T, AnnotationItem> || std::is_base_of_v<ValueItem, T>) {
            return annotationItemsEnd_;
        }

        return itemsEnd_;
    }

    class PANDA_PUBLIC_API IndexItem : public BaseItem {
    public:
        IndexItem(IndexType type, size_t maxIndex) : type_(type), maxIndex_(maxIndex)
        {
            ASSERT(type_ != IndexType::NONE);
        }

        ~IndexItem() override = default;

        DEFAULT_COPY_SEMANTIC(IndexItem);
        NO_MOVE_SEMANTIC(IndexItem);

        size_t Alignment() override
        {
            return sizeof(uint32_t);
        }

        PANDA_PUBLIC_API bool Write(Writer *writer) override;

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
            return index_.size() * ID_SIZE;
        }

    private:
        struct Comparator {
            bool operator()(IndexedItem *item1, IndexedItem *item2) const noexcept
            {
                auto indexType = item1->GetIndexType();
                if (indexType == IndexType::CLASS) {
                    auto typeItem1 = static_cast<TypeItem *>(item1);
                    auto typeItem2 = static_cast<TypeItem *>(item2);

                    auto typeId1 = static_cast<size_t>(typeItem1->GetType().GetId());
                    auto typeId2 = static_cast<size_t>(typeItem2->GetType().GetId());
                    if (typeId1 != typeId2) {
                        return typeId1 < typeId2;
                    }
                }

                if (indexType == IndexType::LINE_NUMBER_PROG) {
                    auto refCount1 = item1->GetRefCount();
                    auto refCount2 = item2->GetRefCount();
                    if (refCount1 != refCount2) {
                        return refCount1 > refCount2;
                    }
                }

                return item1->GetItemAllocId() < item2->GetItemAllocId();
            }
        };

        IndexType type_;
        size_t maxIndex_;
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
            ASSERT(item->GetRefCount() > 0);
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
            if (item->GetRefCount() == 0) {
                item->SetNeedsEmit(false);
            } else {
                Add(item);
            }
        }
    };

    class RegionHeaderItem : public BaseItem {
    public:
        explicit RegionHeaderItem(std::vector<IndexItem *> indexes) : indexes_(std::move(indexes))
        {
            ASSERT(indexes_.size() == INDEX_COUNT_16);
        }

        ~RegionHeaderItem() override = default;

        DEFAULT_COPY_SEMANTIC(RegionHeaderItem);
        NO_MOVE_SEMANTIC(RegionHeaderItem);

        size_t Alignment() override
        {
            return ID_SIZE;
        }

        bool Write(Writer *writer) override;

        ItemTypes GetItemType() const override
        {
            return ItemTypes::REGION_HEADER;
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
            return sizeof(File::RegionHeader);
        }

    private:
        IndexItem *GetIndexByType(IndexType type) const
        {
            auto i = static_cast<size_t>(type);
            return indexes_[i];
        }

        BaseItem *start_ {nullptr};
        BaseItem *end_ {nullptr};
        std::vector<IndexItem *> indexes_;
    };

    class PANDA_PUBLIC_API RegionSectionItem : public BaseItem {
    public:
        size_t Alignment() override
        {
            return ID_SIZE;
        }

        bool Write(Writer *writer) override;

        ItemTypes GetItemType() const override
        {
            return ItemTypes::REGION_SECTION;
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

        RegionHeaderItem *GetCurrentHeader()
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
        std::list<RegionHeaderItem> headers_;
        std::list<IndexItem> indexes_;
    };

    class ProtoKey {
    public:
        ProtoKey(TypeItem *retType, const std::vector<MethodParamItem> &params);

        ~ProtoKey() = default;

        DEFAULT_COPY_SEMANTIC(ProtoKey);
        NO_MOVE_SEMANTIC(ProtoKey);

        size_t GetHash() const
        {
            return hash_;
        }

        bool operator==(const ProtoKey &key) const
        {
            return shorty_ == key.shorty_ && refTypes_ == key.refTypes_;
        }

    private:
        void Add(TypeItem *item);

        size_t hash_;
        std::string shorty_;
        std::vector<TypeItem *> refTypes_;
    };

    struct ProtoKeyHash {
        size_t operator()(const ProtoKey &key) const noexcept
        {
            return key.GetHash();
        };
    };

    struct LiteralArrayCompare {
        bool operator()(const std::string &lhs, const std::string &rhs) const
        {
            return lhs.length() < rhs.length() || (lhs.length() == rhs.length() && lhs < rhs);
        }
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

    bool WriteHeader(Writer *writer, ssize_t *checksumOffset);

    bool WriteHeaderIndexInfo(Writer *writer);

    void RebuildRegionSection();

    void RebuildLineNumberProgramIndex();

    void UpdateOrderIndexes();

    void UpdateLiteralIndexes();

    void ProcessIndexDependecies(BaseItem *item);

    size_t GetForeignOffset() const;

    size_t GetForeignSize() const;

    std::unordered_map<std::string, StringItem *> stringMap_;
    std::map<std::string, LiteralArrayItem *, LiteralArrayCompare> literalarrayMap_;

    std::map<std::string, BaseClassItem *> classMap_;
    std::map<std::string, BaseClassItem *> exportMap_;

    std::unordered_map<uint32_t, ValueItem *> intValueMap_;
    std::unordered_map<uint64_t, ValueItem *> longValueMap_;
    // NB! For f32 and f64 value maps we use integral keys
    // (in fact, bit patterns of corresponding values) to
    // workaround 0.0 == -0.0 semantics.
    std::unordered_map<uint32_t, ValueItem *> floatValueMap_;
    std::unordered_map<uint64_t, ValueItem *> doubleValueMap_;
    std::unordered_map<BaseItem *, ValueItem *> idValueMap_;
    std::unordered_map<ProtoKey, ProtoItem *, ProtoKeyHash> protoMap_;
    std::unordered_map<Type::TypeId, PrimitiveTypeItem *> primitiveTypeMap_;

    std::list<std::unique_ptr<BaseItem>> items_;

    std::vector<std::unique_ptr<BaseItem>> foreignItems_;

    RegionSectionItem regionSectionItem_;

    LineNumberProgramIndexItem lineNumberProgramIndexItem_;

    std::list<std::unique_ptr<BaseItem>>::iterator itemsEnd_;
    std::list<std::unique_ptr<BaseItem>>::iterator annotationItemsEnd_;
    std::list<std::unique_ptr<BaseItem>>::iterator codeItemsEnd_;
    std::list<std::unique_ptr<BaseItem>>::iterator debugItemsEnd_;

    BaseItem *end_;

    bool isQuickened_ = false;
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_FILE_ITEM_CONTAINER_H_
