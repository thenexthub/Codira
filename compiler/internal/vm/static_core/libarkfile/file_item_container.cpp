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

#include "libarkfile/file_item_container.h"
#include <cstdint>
#include <type_traits>
#include "libarkbase/macros.h"
#include <libarkfile/include/file_format_version.h>
#include "libarkfile/pgo.h"

namespace ark::panda_file {

class ItemDeduper {
public:
    template <class T>
    T *Deduplicate(T *item)
    {
        static_assert(std::is_base_of_v<BaseItem, T>);

        if (auto iter = alreadyDedupedItems_.find(item); iter != alreadyDedupedItems_.end()) {
            ASSERT(item->GetItemType() == iter->second->GetItemType());
            return static_cast<T *>(iter->second);
        }

        ItemData itemData(item);
        auto it = items_.find(itemData);
        if (it == items_.cend()) {
            items_.insert(itemData);
            return item;
        }

        auto resultItem = it->GetItem();
        ASSERT(item->GetItemType() == resultItem->GetItemType());
        auto result = static_cast<T *>(resultItem);
        if (item != result) {
            alreadyDedupedItems_.emplace(item, result);
            if constexpr (!std::is_same_v<T, LineNumberProgramItem>) {
                item->SetNeedsEmit(false);
            }
        }

        return result;
    }

    size_t GetUniqueCount() const
    {
        return items_.size();
    }

private:
    class ItemWriter : public Writer {
    public:
        ItemWriter(std::vector<uint8_t> *buf, size_t offset) : buf_(buf), offset_(offset) {}
        ~ItemWriter() override = default;

        NO_COPY_SEMANTIC(ItemWriter);
        NO_MOVE_SEMANTIC(ItemWriter);

        bool WriteByte(uint8_t byte) override
        {
            buf_->push_back(byte);
            ++offset_;
            return true;
        }

        bool WriteBytes(const std::vector<uint8_t> &bytes) override
        {
            buf_->insert(buf_->end(), bytes.cbegin(), bytes.cend());
            offset_ += bytes.size();
            return true;
        }

        size_t GetOffset() const override
        {
            return offset_;
        }

    private:
        std::vector<uint8_t> *buf_;
        size_t offset_;
    };

    class ItemData {
    public:
        explicit ItemData(BaseItem *item) : item_(item)
        {
            Initialize();
        }

        ~ItemData() = default;

        DEFAULT_COPY_SEMANTIC(ItemData);
        NO_MOVE_SEMANTIC(ItemData);

        BaseItem *GetItem() const
        {
            return item_;
        }

        uint32_t GetHash() const
        {
            ASSERT(IsInitialized());
            return hash_;
        }

        bool operator==(const ItemData &itemData) const noexcept
        {
            ASSERT(IsInitialized());
            return data_ == itemData.data_;
        }

    private:
        bool IsInitialized() const
        {
            return !data_.empty();
        }

        void Initialize()
        {
            ASSERT(item_->NeedsEmit());

            ItemWriter writer(&data_, item_->GetOffset());
            [[maybe_unused]] auto res = item_->Write(&writer);
            ASSERT(res);
            ASSERT(data_.size() == item_->GetSize());

            hash_ = GetHash32(data_.data(), data_.size());
        }

        BaseItem *item_;
        uint32_t hash_ {0};
        std::vector<uint8_t> data_;
    };

    struct ItemHash {
        size_t operator()(const ItemData &itemData) const noexcept
        {
            return itemData.GetHash();
        }
    };

    std::unordered_set<ItemData, ItemHash> items_;
    std::unordered_map<BaseItem *, BaseItem *> alreadyDedupedItems_;
};

// NOTE(nsizov): make method for items deletion
template <class T, class C, class I, class P, class E, class... Args>
// CC-OFFNXT(G.FUN.01-CPP) solid logic
[[nodiscard]] static T *GetOrInsert(C &map, I &items, const P &pos, const E &key, bool isForeign, Args &&...args)
{
    auto it = map.find(key);
    if (it != map.cend()) {
        auto *item = it->second;
        if (item->IsForeign() == isForeign) {
            return static_cast<T *>(item);
        }

        return nullptr;
    }

    auto ii = items.insert(pos, std::make_unique<T>(std::forward<Args>(args)...));
    auto *item = static_cast<T *>(ii->get());

    [[maybe_unused]] auto res = map.insert({key, item});
    ASSERT(res.second);
    return item;
}

ItemContainer::ItemContainer()
{
    itemsEnd_ = items_.insert(items_.end(), std::make_unique<EndItem>());
    annotationItemsEnd_ = items_.insert(items_.end(), std::make_unique<EndItem>());
    codeItemsEnd_ = items_.insert(items_.end(), std::make_unique<EndItem>());
    debugItemsEnd_ = items_.insert(items_.end(), std::make_unique<EndItem>());
    end_ = debugItemsEnd_->get();
}

ClassItem *ItemContainer::GetOrCreateClassItem(const std::string &str)
{
    [[maybe_unused]] auto item = GetOrInsert<ClassItem>(classMap_, items_, itemsEnd_, str, false, str);
    ASSERT(item != nullptr);
    return item;
}

ForeignClassItem *ItemContainer::GetOrCreateForeignClassItem(const std::string &str)
{
    return GetOrInsert<ForeignClassItem>(classMap_, foreignItems_, foreignItems_.end(), str, true, str);
}

StringItem *ItemContainer::GetOrCreateStringItem(const std::string &str)
{
    auto it = classMap_.find(str);
    if (it != classMap_.cend()) {
        return it->second->GetNameItem();
    }

    [[maybe_unused]] auto item = GetOrInsert<StringItem>(stringMap_, items_, itemsEnd_, str, false, str);
    ASSERT(item != nullptr);
    return item;
}

LiteralArrayItem *ItemContainer::GetOrCreateLiteralArrayItem(const std::string &id)
{
    [[maybe_unused]] auto item = GetOrInsert<LiteralArrayItem>(literalarrayMap_, items_, itemsEnd_, id, false);
    ASSERT(item != nullptr);
    return item;
}

ScalarValueItem *ItemContainer::GetOrCreateIntegerValueItem(uint32_t v)
{
    [[maybe_unused]] auto item = GetOrInsert<ScalarValueItem>(intValueMap_, items_, itemsEnd_, v, false, v);
    ASSERT(item != nullptr);
    return item;
}

ScalarValueItem *ItemContainer::GetOrCreateLongValueItem(uint64_t v)
{
    [[maybe_unused]] auto item = GetOrInsert<ScalarValueItem>(longValueMap_, items_, itemsEnd_, v, false, v);
    ASSERT(item != nullptr);
    return item;
}

ScalarValueItem *ItemContainer::GetOrCreateFloatValueItem(float v)
{
    [[maybe_unused]] auto item =
        GetOrInsert<ScalarValueItem>(floatValueMap_, items_, itemsEnd_, bit_cast<uint32_t>(v), false, v);
    ASSERT(item != nullptr);
    return item;
}

ScalarValueItem *ItemContainer::GetOrCreateDoubleValueItem(double v)
{
    [[maybe_unused]] auto item =
        GetOrInsert<ScalarValueItem>(doubleValueMap_, items_, itemsEnd_, bit_cast<uint64_t>(v), false, v);
    ASSERT(item != nullptr);
    return item;
}

ScalarValueItem *ItemContainer::GetOrCreateIdValueItem(BaseItem *v)
{
    [[maybe_unused]] auto item = GetOrInsert<ScalarValueItem>(idValueMap_, items_, itemsEnd_, v, false, v);
    ASSERT(item != nullptr);
    return item;
}

ProtoItem *ItemContainer::GetOrCreateProtoItem(TypeItem *retType, const std::vector<MethodParamItem> &params)
{
    ProtoKey key(retType, params);
    [[maybe_unused]] auto item = GetOrInsert<ProtoItem>(protoMap_, items_, itemsEnd_, key, false, retType, params);
    ASSERT(item != nullptr);
    return item;
}

PrimitiveTypeItem *ItemContainer::GetOrCreatePrimitiveTypeItem(Type type)
{
    return GetOrCreatePrimitiveTypeItem(type.GetId());
}

PrimitiveTypeItem *ItemContainer::GetOrCreatePrimitiveTypeItem(Type::TypeId type)
{
    [[maybe_unused]] auto item =
        GetOrInsert<PrimitiveTypeItem>(primitiveTypeMap_, items_, itemsEnd_, type, false, type);
    ASSERT(item != nullptr);
    return item;
}

LineNumberProgramItem *ItemContainer::CreateLineNumberProgramItem()
{
    auto it = items_.insert(debugItemsEnd_, std::make_unique<LineNumberProgramItem>());
    auto *item = static_cast<LineNumberProgramItem *>(it->get());
    [[maybe_unused]] auto res = lineNumberProgramIndexItem_.Add(item);
    ASSERT(res);
    return item;
}

void ItemContainer::IncRefLineNumberProgramItem(LineNumberProgramItem *it)
{
    lineNumberProgramIndexItem_.IncRefCount(it);
}

void ItemContainer::DeduplicateLineNumberProgram(DebugInfoItem *item, ItemDeduper *deduper)
{
    auto *lineNumberProgram = item->GetLineNumberProgram();
    auto *deduplicated = deduper->Deduplicate(lineNumberProgram);
    if (deduplicated != lineNumberProgram) {
        item->SetLineNumberProgram(deduplicated);
        lineNumberProgramIndexItem_.IncRefCount(deduplicated);
        lineNumberProgramIndexItem_.DecRefCount(lineNumberProgram);
    }
}

void ItemContainer::DeduplicateDebugInfo(MethodItem *method, ItemDeduper *debugInfoDeduper,
                                         ItemDeduper *lineNumberProgramDeduper)
{
    auto *debugItem = method->GetDebugInfo();
    if (debugItem == nullptr) {
        return;
    }

    DeduplicateLineNumberProgram(debugItem, lineNumberProgramDeduper);

    auto *deduplicated = debugInfoDeduper->Deduplicate(debugItem);
    if (deduplicated != debugItem) {
        method->SetDebugInfo(deduplicated);
        lineNumberProgramIndexItem_.DecRefCount(debugItem->GetLineNumberProgram());
    }
}

static void DeduplicateCode(MethodItem *method, ItemDeduper *codeDeduper)
{
    auto *codeItem = method->GetCode();
    if (codeItem == nullptr) {
        return;
    }

    auto *deduplicated = codeDeduper->Deduplicate(codeItem);
    if (deduplicated != codeItem) {
        method->SetCode(deduplicated);
        deduplicated->AddMethod(method);  // we need it for Profile-Guided optimization
    }
}

void ItemContainer::DeduplicateCodeAndDebugInfo()
{
    ItemDeduper lineNumberProgramDeduper;
    ItemDeduper debugDeduper;
    ItemDeduper codeDeduper;

    for (auto &p : classMap_) {
        auto *item = p.second;
        if (item->IsForeign()) {
            continue;
        }

        auto *classItem = static_cast<ClassItem *>(item);

        classItem->VisitMethods([this, &debugDeduper, &lineNumberProgramDeduper, &codeDeduper](BaseItem *paramItem) {
            auto *methodItem = static_cast<MethodItem *>(paramItem);
            DeduplicateDebugInfo(methodItem, &debugDeduper, &lineNumberProgramDeduper);
            DeduplicateCode(methodItem, &codeDeduper);
            return true;
        });
    }
}

static void DeduplicateAnnotationValue(AnnotationItem *annotationItem, ItemDeduper *deduper)
{
    auto *elems = annotationItem->GetElements();
    const auto &tags = annotationItem->GetTags();

    for (size_t i = 0; i < elems->size(); i++) {
        auto tag = tags[i];

        // try to dedupe only ArrayValueItems
        switch (tag.GetItem()) {
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'R':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y':
            case 'Z':
            case '@':
                break;
            default:
                continue;
        }

        auto &elem = (*elems)[i];
        auto *value = elem.GetValue();
        auto *deduplicated = deduper->Deduplicate(value);
        if (deduplicated != value) {
            elem.SetValue(deduplicated);
        }
    }
}

static void DeduplicateAnnotations(std::vector<AnnotationItem *> *items, ItemDeduper *annotationDeduper,
                                   ItemDeduper *valueDeduper)
{
    for (auto &item : *items) {
        DeduplicateAnnotationValue(item, valueDeduper);
        auto *deduplicated = annotationDeduper->Deduplicate(item);
        if (deduplicated != item) {
            item = deduplicated;
        }
    }
}

void ItemContainer::DeduplicateAnnotations()
{
    ItemDeduper valueDeduper;
    ItemDeduper annotationDeduper;

    for (auto &p : classMap_) {
        auto *item = p.second;
        if (item->IsForeign()) {
            continue;
        }

        auto *classItem = static_cast<ClassItem *>(item);

        panda_file::DeduplicateAnnotations(classItem->GetRuntimeAnnotations(), &annotationDeduper, &valueDeduper);
        panda_file::DeduplicateAnnotations(classItem->GetAnnotations(), &annotationDeduper, &valueDeduper);
        panda_file::DeduplicateAnnotations(classItem->GetRuntimeTypeAnnotations(), &annotationDeduper, &valueDeduper);
        panda_file::DeduplicateAnnotations(classItem->GetTypeAnnotations(), &annotationDeduper, &valueDeduper);

        classItem->VisitMethods([&annotationDeduper, &valueDeduper](BaseItem *paramItem) {
            auto *methodItem = static_cast<MethodItem *>(paramItem);
            panda_file::DeduplicateAnnotations(methodItem->GetRuntimeAnnotations(), &annotationDeduper, &valueDeduper);
            panda_file::DeduplicateAnnotations(methodItem->GetAnnotations(), &annotationDeduper, &valueDeduper);
            panda_file::DeduplicateAnnotations(methodItem->GetRuntimeTypeAnnotations(), &annotationDeduper,
                                               &valueDeduper);
            panda_file::DeduplicateAnnotations(methodItem->GetTypeAnnotations(), &annotationDeduper, &valueDeduper);
            return true;
        });

        classItem->VisitFields([&annotationDeduper, &valueDeduper](BaseItem *paramItem) {
            auto *fieldItem = static_cast<FieldItem *>(paramItem);
            panda_file::DeduplicateAnnotations(fieldItem->GetRuntimeAnnotations(), &annotationDeduper, &valueDeduper);
            panda_file::DeduplicateAnnotations(fieldItem->GetAnnotations(), &annotationDeduper, &valueDeduper);
            panda_file::DeduplicateAnnotations(fieldItem->GetRuntimeTypeAnnotations(), &annotationDeduper,
                                               &valueDeduper);
            panda_file::DeduplicateAnnotations(fieldItem->GetTypeAnnotations(), &annotationDeduper, &valueDeduper);
            return true;
        });
    }
}

void ItemContainer::DeduplicateItems(bool computeLayout)
{
    if (computeLayout) {
        ComputeLayout();
    }
    DeduplicateCodeAndDebugInfo();
    DeduplicateAnnotations();
}

void ItemContainer::MarkLiteralarrayMap()
{
    for (auto &entry : literalarrayMap_) {
        entry.second->SetDependencyMark();
    }
}

void ItemContainer::CleanupArrayValueItems(ValueItem *value)
{
    auto arrayValue = value->GetAsArray();
    auto *vecItems = arrayValue->GetMutableItems();
    auto newEnd = std::partition(vecItems->begin(), vecItems->end(), [](const ScalarValueItem &valueItem) {
        if (valueItem.GetType() == ValueItem::Type::ID) {
            auto realitem = valueItem.GetIdItem();
            return realitem->GetDependencyMark();
        }
        return true;
    });
    vecItems->erase(newEnd, vecItems->end());
}

void ItemContainer::DeleteReferenceFromAnno(AnnotationItem *annoItem)
{
    auto &elements = *annoItem->GetElements();
    for (auto element = elements.begin(); element != elements.end();) {
        bool nodelete = true;
        auto *value = element->GetValue();
        if (value->GetType() == ValueItem::Type::ARRAY) {
            CleanupArrayValueItems(value);
        } else if (value->GetType() == ValueItem::Type::ID) {
            auto scalarValue = value->GetAsScalar();
            nodelete = scalarValue->GetIdItem()->GetDependencyMark();
        }
        if (nodelete) {
            ++element;
        } else {
            element = elements.erase(element);
        }
    }
}

uint32_t ItemContainer::DeleteItems()
{
    // ANNOTATION_ITEM reference other item by
    // AnnotationItem->std::vector<Elem>
    // Elem->ValueItem
    // ValueItem->BaseItem if ValueItem::Type == Type::ID
    // BaseItem should be remove from AnnotationItem before delete
    auto shouldRemoveAnnotationItem = [this](const std::unique_ptr<BaseItem> &item) -> bool {
        if (item->GetItemType() != ItemTypes::ANNOTATION_ITEM) {
            return false;
        }
        auto annoItem = static_cast<AnnotationItem *>(item.get());
        DeleteReferenceFromAnno(annoItem);
        return !item->GetDependencyMark() && item->NeedsEmit();
    };
    items_.remove_if(shouldRemoveAnnotationItem);
    items_.remove_if([this](const std::unique_ptr<BaseItem> &item) {
        if (!item->GetDependencyMark()) {
            // lineNumberProgramIndexItem_ reference LineNumberProgramItem.
            // remove from lnp before item deleted if item not necessary.
            if (item->GetItemType() == ItemTypes::LINE_NUMBER_PROGRAM_ITEM) {
                auto lnpitem = static_cast<LineNumberProgramItem *>(item.get());
                lineNumberProgramIndexItem_.Remove(lnpitem);
            }
        }
        return !item->GetDependencyMark() && item->NeedsEmit();
    });
    return 0;
}

uint32_t ItemContainer::DeleteForeignItems()
{
    auto newEnd = std::partition(foreignItems_.begin(), foreignItems_.end(), [](const std::unique_ptr<BaseItem> &item) {
        return item->GetDependencyMark() || !item->NeedsEmit();
    });
    foreignItems_.erase(newEnd, foreignItems_.end());
    return 0;
}

void ItemContainer::FillExportMap()
{
    for (auto &it : classMap_) {
        if (!it.second->IsForeign() && it.first.rfind("ETSGLOBAL") != std::string::npos) {
            exportMap_.insert({it.first, it.second});
        }
    }
}

uint32_t ItemContainer::ComputeLayout()
{
    FillExportMap();

    uint32_t numClasses = classMap_.size();
    uint32_t numLiteralarrays = literalarrayMap_.size();
    uint32_t numExportClasses = exportMap_.size();
    uint32_t classIdxOffset = sizeof(File::Header);
    uint32_t exportIdxOffset = classIdxOffset + numClasses * ID_SIZE;  // immediately after class table
    uint32_t curOffset = exportIdxOffset + (numExportClasses + numLiteralarrays) * ID_SIZE;

    UpdateOrderIndexes();
    UpdateLiteralIndexes();

    RebuildRegionSection();
    RebuildLineNumberProgramIndex();

    regionSectionItem_.SetOffset(curOffset);
    regionSectionItem_.ComputeLayout();
    curOffset += regionSectionItem_.GetSize();

    for (auto &item : foreignItems_) {
        curOffset = RoundUp(curOffset, item->Alignment());
        item->SetOffset(curOffset);
        item->ComputeLayout();
        curOffset += item->GetSize();
    }

    for (auto &item : items_) {
        if (!item->NeedsEmit()) {
            continue;
        }

        curOffset = RoundUp(curOffset, item->Alignment());
        item->SetOffset(curOffset);
        item->ComputeLayout();
        curOffset += item->GetSize();
    }

    // Line number program should be last because it's size is known only after deduplication
    curOffset = RoundUp(curOffset, lineNumberProgramIndexItem_.Alignment());
    lineNumberProgramIndexItem_.SetOffset(curOffset);
    lineNumberProgramIndexItem_.ComputeLayout();
    curOffset += lineNumberProgramIndexItem_.GetSize();

    end_->SetOffset(curOffset);

    return curOffset;
}

void ItemContainer::RebuildLineNumberProgramIndex()
{
    lineNumberProgramIndexItem_.Reset();
    lineNumberProgramIndexItem_.UpdateItems(nullptr, nullptr);
}

void ItemContainer::RebuildRegionSection()
{
    regionSectionItem_.Reset();

    for (auto &item : foreignItems_) {
        ProcessIndexDependecies(item.get());
    }

    for (auto &item : items_) {
        if (!item->NeedsEmit()) {
            continue;
        }

        ProcessIndexDependecies(item.get());
    }

    if (!regionSectionItem_.IsEmpty()) {
        regionSectionItem_.GetCurrentHeader()->SetEnd(end_);
    }

    regionSectionItem_.UpdateItems();
}

void ItemContainer::UpdateOrderIndexes()
{
    size_t idx = 0;

    for (auto &item : foreignItems_) {
        item->SetOrderIndex(idx++);
        item->Visit([&idx](BaseItem *paramItem) {
            paramItem->SetOrderIndex(idx++);
            return true;
        });
    }

    for (auto &item : items_) {
        if (!item->NeedsEmit()) {
            continue;
        }

        item->SetOrderIndex(idx++);
        item->Visit([&idx](BaseItem *paramItem) {
            paramItem->SetOrderIndex(idx++);
            return true;
        });
    }

    end_->SetOrderIndex(idx++);
}

void ItemContainer::UpdateLiteralIndexes()
{
    size_t idx = 0;

    for (auto &it : literalarrayMap_) {
        it.second->SetIndex(idx++);
    }
}

void ItemContainer::ReorderItems(ark::panda_file::pgo::ProfileOptimizer *profileOpt)
{
    profileOpt->ProfileGuidedRelayout(items_);
}

void ItemContainer::ProcessIndexDependecies(BaseItem *item)
{
    auto deps = item->GetIndexDependencies();

    item->Visit([&deps](BaseItem *paramItem) {
        const auto &itemDeps = paramItem->GetIndexDependencies();
        deps.insert(deps.end(), itemDeps.cbegin(), itemDeps.cend());
        return true;
    });

    if (regionSectionItem_.IsEmpty()) {
        regionSectionItem_.AddHeader();
        regionSectionItem_.GetCurrentHeader()->SetStart(item);
    }

    if (regionSectionItem_.GetCurrentHeader()->Add(deps)) {
        return;
    }

    regionSectionItem_.GetCurrentHeader()->SetEnd(item);
    regionSectionItem_.AddHeader();
    regionSectionItem_.GetCurrentHeader()->SetStart(item);

    if (!regionSectionItem_.GetCurrentHeader()->Add(deps)) {
        LOG(FATAL, PANDAFILE) << "Cannot add " << deps.size() << " items to index";
    }
}

bool ItemContainer::WriteHeaderIndexInfo(Writer *writer)
{
    if (!writer->Write<uint32_t>(classMap_.size())) {
        return false;
    }

    if (!writer->Write<uint32_t>(sizeof(File::Header))) {
        return false;
    }

    if (!writer->Write<uint32_t>(exportMap_.size())) {
        return false;
    }

    uint32_t exportMapOffset = sizeof(File::Header) + classMap_.size() * ID_SIZE;
    if (!writer->Write<uint32_t>(exportMapOffset)) {
        return false;
    }

    if (!writer->Write<uint32_t>(lineNumberProgramIndexItem_.GetNumItems())) {
        return false;
    }

    if (!writer->Write<uint32_t>(lineNumberProgramIndexItem_.GetOffset())) {
        return false;
    }

    if (!writer->Write<uint32_t>(literalarrayMap_.size())) {
        return false;
    }

    uint32_t literalarrayIdxOffset = sizeof(File::Header) + (classMap_.size() + exportMap_.size()) * ID_SIZE;
    if (!writer->Write<uint32_t>(literalarrayIdxOffset)) {
        return false;
    }

    if (!writer->Write<uint32_t>(regionSectionItem_.GetNumHeaders())) {
        return false;
    }

    size_t indexSectionOff = literalarrayIdxOffset + literalarrayMap_.size() * ID_SIZE;
    return writer->Write<uint32_t>(indexSectionOff);
}

bool ItemContainer::WriteHeader(Writer *writer, ssize_t *checksumOffset)
{
    uint32_t fileSize = ComputeLayout();
    writer->ReserveBufferCapacity(fileSize);

    std::vector<uint8_t> magic;
    magic.assign(File::MAGIC.cbegin(), File::MAGIC.cend());
    if (!writer->WriteBytes(magic)) {
        return false;
    }

    *checksumOffset = static_cast<ssize_t>(writer->GetOffset());
    uint32_t checksum = 0;
    if (!writer->Write(checksum)) {
        return false;
    }
    writer->CountChecksum(true);

    std::vector<uint8_t> versionVec(std::begin(VERSION), std::end(VERSION));
    if (!writer->WriteBytes(versionVec)) {
        return false;
    }

    if (!writer->Write(fileSize)) {
        return false;
    }

    uint32_t foreignOffset = GetForeignOffset();
    if (!writer->Write(foreignOffset)) {
        return false;
    }

    uint32_t foreignSize = GetForeignSize();
    if (!writer->Write(foreignSize)) {
        return false;
    }

    if (!writer->Write<uint32_t>(static_cast<uint32_t>(isQuickened_))) {
        return false;
    }

    return WriteHeaderIndexInfo(writer);
}

bool ItemContainer::Write(Writer *writer, bool deduplicateItems, bool computeLayout)
{
    if (deduplicateItems) {
        DeduplicateItems(computeLayout);
    }

    ssize_t checksumOffset = -1;
    if (!WriteHeader(writer, &checksumOffset)) {
        return false;
    }
    ASSERT(checksumOffset != -1);

    // Write class idx
    for (auto &entry : classMap_) {
        if (!writer->Write(entry.second->GetOffset())) {
            return false;
        }
    }

    // Write export class idx
    for (auto &entry : exportMap_) {
        if (!writer->Write(entry.second->GetOffset())) {
            return false;
        }
    }

    // Write literalArray idx
    for (auto &entry : literalarrayMap_) {
        if (!writer->Write(entry.second->GetOffset())) {
            return false;
        }
    }

    // Write index section

    if (!regionSectionItem_.Write(writer)) {
        return false;
    }

    for (auto &item : foreignItems_) {
        if (!writer->Align(item->Alignment()) || !item->Write(writer)) {
            return false;
        }
    }

    for (auto &item : items_) {
        if (!item->NeedsEmit()) {
            continue;
        }

        if (!writer->Align(item->Alignment()) || !item->Write(writer)) {
            return false;
        }
    }

    if (!writer->Align(lineNumberProgramIndexItem_.Alignment())) {
        return false;
    }

    // Write line number program idx

    if (!lineNumberProgramIndexItem_.Write(writer)) {
        return false;
    }

    writer->CountChecksum(false);
    writer->WriteChecksum(checksumOffset);

    return writer->FinishWrite();
}

std::map<std::string, size_t> ItemContainer::GetStat()
{
    std::map<std::string, size_t> stat;

    DeduplicateItems();
    ComputeLayout();

    stat["header_item"] = sizeof(File::Header);
    stat["class_idx_item"] = classMap_.size() * ID_SIZE;
    stat["export_table_idx_item"] = exportMap_.size() * ID_SIZE;
    stat["line_number_program_idx_item"] = lineNumberProgramIndexItem_.GetNumItems() * ID_SIZE;
    stat["literalarray_idx"] = literalarrayMap_.size() * ID_SIZE;

    stat["region_section_item"] = regionSectionItem_.GetSize();
    stat["foreign_item"] = GetForeignSize();

    size_t numIns = 0;
    size_t codesize = 0;
    for (auto &item : items_) {
        if (!item->NeedsEmit()) {
            continue;
        }

        const auto &name = item->GetName();
        size_t size = item->GetSize();
        auto it = stat.find(name);
        if (it != stat.cend()) {
            stat[name] += size;
        } else if (size != 0) {
            stat[name] = size;
        }
        if (name == "code_item") {
            numIns += static_cast<CodeItem *>(item.get())->GetNumInstructions();
            codesize += static_cast<CodeItem *>(item.get())->GetCodeSize();
        }
    }
    stat["instructions_number"] = numIns;
    stat["codesize"] = codesize;

    return stat;
}

void ItemContainer::DumpItemsStat(std::ostream &os) const
{
    struct Stat {
        size_t n;
        size_t totalSize;
    };

    std::map<std::string, Stat> stat;

    auto collectStat = [&stat](auto &items) {
        for (auto &item : items) {
            if (!item->NeedsEmit()) {
                continue;
            }

            const auto &name = item->GetName();
            size_t size = item->GetSize();
            auto it = stat.find(name);
            if (it != stat.cend()) {
                stat[name].n += 1;
                stat[name].totalSize += size;
            } else if (size != 0) {
                stat[name] = {1, size};
            }
        }
    };

    collectStat(foreignItems_);
    collectStat(items_);

    for (auto &[name, elem] : stat) {
        os << name << ":" << std::endl;
        os << "    n          = " << elem.n << std::endl;
        os << "    total size = " << elem.totalSize << std::endl;
    }
}

size_t ItemContainer::GetForeignOffset() const
{
    if (foreignItems_.empty()) {
        return 0;
    }

    return foreignItems_.front()->GetOffset();
}

size_t ItemContainer::GetForeignSize() const
{
    if (foreignItems_.empty()) {
        return 0;
    }

    size_t begin = foreignItems_.front()->GetOffset();
    size_t end = foreignItems_.back()->GetOffset() + foreignItems_.back()->GetSize();

    return end - begin;
}

bool ItemContainer::RegionHeaderItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());
    ASSERT(start_ != nullptr);
    ASSERT(start_->GetOffset() != 0);
    ASSERT(end_ != nullptr);
    ASSERT(end_->GetOffset() != 0);

    if (!writer->Write<uint32_t>(start_->GetOffset())) {
        return false;
    }

    if (!writer->Write<uint32_t>(end_->GetOffset())) {
        return false;
    }

    for (auto *indexItem : indexes_) {
        if (!writer->Write<uint32_t>(indexItem->GetNumItems())) {
            return false;
        }

        ASSERT(indexItem->GetOffset() != 0);
        if (!writer->Write<uint32_t>(indexItem->GetOffset())) {
            return false;
        }
    }

    return true;
}

bool ItemContainer::RegionHeaderItem::Add(const std::list<IndexedItem *> &items)
{
    std::list<IndexedItem *> addedItems;

    for (auto *item : items) {
        auto type = item->GetIndexType();
        ASSERT(type != IndexType::NONE);

        auto *indexItem = GetIndexByType(type);

        if (indexItem->Has(item)) {
            continue;
        }

        if (!indexItem->Add(item)) {
            Remove(addedItems);
            return false;
        }

        addedItems.push_back(item);
    }

    return true;
}

void ItemContainer::RegionHeaderItem::Remove(const std::list<IndexedItem *> &items)
{
    for (auto *item : items) {
        auto type = item->GetIndexType();
        ASSERT(type != IndexType::NONE);

        auto *indexItem = GetIndexByType(type);
        indexItem->Remove(item);
    }
}

bool ItemContainer::IndexItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    for (auto *item : index_) {
        if (!writer->Write<uint32_t>(item->GetOffset())) {
            return false;
        }
    }

    return true;
}

ItemTypes ItemContainer::IndexItem::GetItemType() const
{
    switch (type_) {
        case IndexType::CLASS:
            return ItemTypes::CLASS_INDEX_ITEM;
        case IndexType::METHOD:
            return ItemTypes::METHOD_INDEX_ITEM;
        case IndexType::FIELD:
            return ItemTypes::FIELD_INDEX_ITEM;
        case IndexType::PROTO:
            return ItemTypes::PROTO_INDEX_ITEM;
        case IndexType::LINE_NUMBER_PROG:
            return ItemTypes::LINE_NUMBER_PROGRAM_INDEX_ITEM;
        default:
            break;
    }

    UNREACHABLE();
}

bool ItemContainer::IndexItem::Add(IndexedItem *item)
{
    auto size = index_.size();
    ASSERT(size <= maxIndex_);

    if (size == maxIndex_) {
        return false;
    }

    auto res = index_.insert(item);
    ASSERT(res.second);

    return res.second;
}

void ItemContainer::RegionSectionItem::AddHeader()
{
    std::vector<IndexItem *> indexItems;
    for (size_t i = 0; i < INDEX_COUNT_16; i++) {
        auto type = static_cast<IndexType>(i);
        indexes_.emplace_back(type, MAX_INDEX_16);
        indexItems.push_back(&indexes_.back());
    }
    headers_.emplace_back(indexItems);
}

size_t ItemContainer::RegionSectionItem::CalculateSize() const
{
    size_t size = headers_.size() * sizeof(File::RegionHeader);
    for (auto &indexItem : indexes_) {
        size += indexItem.GetSize();
    }
    return size;
}

void ItemContainer::RegionSectionItem::ComputeLayout()
{
    size_t offset = GetOffset();

    for (auto &header : headers_) {
        header.SetOffset(offset);
        header.ComputeLayout();
        offset += header.GetSize();
    }

    for (auto &index : indexes_) {
        index.SetOffset(offset);
        index.ComputeLayout();
        offset += index.GetSize();
    }
}

bool ItemContainer::RegionSectionItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    for (auto &header : headers_) {
        if (!header.Write(writer)) {
            return false;
        }
    }

    for (auto &index : indexes_) {
        if (!index.Write(writer)) {
            return false;
        }
    }

    return true;
}

ItemContainer::ProtoKey::ProtoKey(TypeItem *retType, const std::vector<MethodParamItem> &params)
{
    Add(retType);
    for (const auto &param : params) {
        Add(param.GetType());
    }
    size_t shortyHash = std::hash<std::string>()(shorty_);
    size_t retTypeHash = std::hash<TypeItem *>()(retType);
    // combine hashes of shorty and ref_types
    hash_ = ark::MergeHashes(shortyHash, retTypeHash);
    // combine hashes of all param types
    for (const auto &item : params) {
        size_t paramTypeHash = std::hash<TypeItem *>()(item.GetType());
        hash_ = ark::MergeHashes(hash_, paramTypeHash);
    }
}

void ItemContainer::ProtoKey::Add(TypeItem *item)
{
    auto type = item->GetType();
    shorty_.append(Type::GetSignatureByTypeId(type));
    if (type.IsReference()) {
        refTypes_.push_back(item);
    }
}

}  // namespace ark::panda_file
