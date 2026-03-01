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

#include <cstddef>
#include <cstdint>

#include "libarkfile/annotation_data_accessor.h"
#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/code_data_accessor.h"
#include "libarkfile/field_data_accessor.h"
#include "libarkfile/file.h"
#include "libarkfile/debug_info_updater-inl.h"
#include "libarkfile/file_items.h"
#include "libarkfile/file_reader.h"
#include "libarkfile/bytecode_instruction-inl.h"
#include "libarkfile/line_number_program.h"
#include "libarkfile/literal_data_accessor-inl.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/debug_data_accessor-inl.h"
#include "libarkfile/field_data_accessor-inl.h"
#include "libarkfile/method_data_accessor-inl.h"

#include "libarkbase/utils/utf.h"
#include "libarkfile/proto_data_accessor.h"

namespace ark::panda_file {

namespace {
class FileReaderDebugInfoUpdater : public DebugInfoUpdater<FileReaderDebugInfoUpdater> {
public:
    using Super = DebugInfoUpdater<FileReaderDebugInfoUpdater>;

    FileReaderDebugInfoUpdater(const File *file, ItemContainer *cont) : Super(file), cont_(cont) {}

    StringItem *GetOrCreateStringItem(const std::string &s)
    {
        return cont_->GetOrCreateStringItem(s);
    }

    BaseClassItem *GetType(File::EntityId typeId, const std::string &typeName)
    {
        if (GetFile()->IsExternal(typeId)) {
            return cont_->GetOrCreateForeignClassItem(typeName);
        }
        return cont_->GetOrCreateClassItem(typeName);
    }

private:
    ItemContainer *cont_;
};
}  // namespace

bool FileReader::ReadContainer(bool shouldRebuildIndices)
{
    const File::Header *header = file_->GetHeader();
    LOG_IF(header->quickenedFlag, FATAL, PANDAFILE) << "File " << file_->GetFullFileName() << " is already quickened";

    if (!ReadClasses()) {
        return false;
    }
    if (!ReadLiteralArrayItems()) {
        return false;
    }
    if (!ReadRegionHeaders()) {
        return false;
    }

    if (shouldRebuildIndices) {
        ComputeLayoutAndUpdateIndices();
    }

    return true;
}

template <typename T>
static void EmplaceLiteralArray(const panda_file::LiteralDataAccessor::LiteralValue &value,
                                std::vector<panda_file::LiteralItem> &literalArray, std::unique_ptr<const File> &file)
{
    File::EntityId id(std::get<uint32_t>(value));
    auto sp = file->GetSpanFromId(id);
    auto len = helpers::Read<sizeof(uint32_t)>(&sp);
    literalArray.emplace_back(len);
    for (size_t i = 0; i < len; i++) {
        auto v = helpers::Read<sizeof(T)>(&sp);
        literalArray.emplace_back(v);
    }
}

static void EmplaceLiteralString(const panda_file::LiteralDataAccessor::LiteralValue &value,
                                 std::vector<panda_file::LiteralItem> &literalArray, std::unique_ptr<const File> &file,
                                 ItemContainer &container)
{
    File::EntityId id(std::get<uint32_t>(value));
    auto data = file->GetStringData(id);
    std::string itemStr(utf::Mutf8AsCString(data.data));
    auto *stringItem = container.GetOrCreateStringItem(itemStr);
    literalArray.emplace_back(stringItem);
}

static void EmplaceLiteralArrayString(const panda_file::LiteralDataAccessor::LiteralValue &value,
                                      std::vector<panda_file::LiteralItem> &literalArray,
                                      std::unique_ptr<const File> &file, ItemContainer &container)
{
    File::EntityId id(std::get<uint32_t>(value));
    auto sp = file->GetSpanFromId(id);
    auto len = helpers::Read<sizeof(uint32_t)>(&sp);
    literalArray.emplace_back(len);
    for (size_t i = 0; i < len; i++) {
        File::EntityId strId(helpers::Read<sizeof(uint32_t)>(&sp));
        auto data = file->GetStringData(strId);
        std::string itemStr(utf::Mutf8AsCString(data.data));
        auto *stringItem = container.GetOrCreateStringItem(itemStr);
        literalArray.emplace_back(stringItem);
    }
}

// CC-OFFNXT(G.FUN.01-CPP, huge_method[C++]) big switch case
void FileReader::EmplaceLiteralVals(std::vector<panda_file::LiteralItem> &literalArray,
                                    const panda_file::LiteralDataAccessor::LiteralValue &value,
                                    const panda_file::LiteralTag &tag)
{
    literalArray.emplace_back(static_cast<uint8_t>(tag));
    switch (tag) {
        case panda_file::LiteralTag::BOOL: {
            literalArray.emplace_back(static_cast<uint8_t>(std::get<bool>(value)));
            break;
        }
        case panda_file::LiteralTag::TAGVALUE:
        case panda_file::LiteralTag::ACCESSOR:
        case panda_file::LiteralTag::NULLVALUE: {
            literalArray.emplace_back(std::get<uint8_t>(value));
            break;
        }
        case panda_file::LiteralTag::ARRAY_U1:
        case panda_file::LiteralTag::ARRAY_I8:
        case panda_file::LiteralTag::ARRAY_U8: {
            EmplaceLiteralArray<uint8_t>(value, literalArray, file_);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I16:
        case panda_file::LiteralTag::ARRAY_U16: {
            EmplaceLiteralArray<uint16_t>(value, literalArray, file_);
            break;
        }
        case panda_file::LiteralTag::INTEGER: {
            literalArray.emplace_back(std::get<uint32_t>(value));
            break;
        }
        case panda_file::LiteralTag::ARRAY_I32:
        case panda_file::LiteralTag::ARRAY_U32:
        case panda_file::LiteralTag::ARRAY_F32: {
            EmplaceLiteralArray<uint32_t>(value, literalArray, file_);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I64:
        case panda_file::LiteralTag::ARRAY_U64:
        case panda_file::LiteralTag::ARRAY_F64: {
            EmplaceLiteralArray<uint64_t>(value, literalArray, file_);
            break;
        }
        case panda_file::LiteralTag::FLOAT: {
            literalArray.emplace_back(bit_cast<uint32_t>(std::get<float>(value)));
            break;
        }
        case panda_file::LiteralTag::DOUBLE: {
            literalArray.emplace_back(bit_cast<uint64_t>(std::get<double>(value)));
            break;
        }
        case panda_file::LiteralTag::STRING: {
            EmplaceLiteralString(value, literalArray, file_, container_);
            break;
        }
        case panda_file::LiteralTag::ARRAY_STRING: {
            EmplaceLiteralArrayString(value, literalArray, file_, container_);
            break;
        }
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD: {
            File::EntityId methodId(std::get<uint32_t>(value));
            MethodDataAccessor methodAcc(*file_, methodId);
            File::EntityId classId(methodAcc.GetClassId());
            auto *classItem = CreateClassItem(classId);
            literalArray.emplace_back(CreateMethodItem(classItem, methodId));
            break;
        }
        default:
            UNREACHABLE();
    }
}

/* static */
bool FileReader::CreateLiteralArrayItem(LiteralDataAccessor *litArrayAccessor, File::EntityId arrayId, uint32_t index)
{
    auto it = itemsDone_.find(arrayId);
    if (it != itemsDone_.end()) {
        return true;
    }

    LiteralArrayItem *item = container_.GetOrCreateLiteralArrayItem(std::to_string(index));
    itemsDone_.insert({arrayId, static_cast<BaseItem *>(item)});

    std::vector<panda_file::LiteralItem> literalArray;

    litArrayAccessor->EnumerateLiteralVals(
        arrayId, [&literalArray, this](const panda_file::LiteralDataAccessor::LiteralValue &value,
                                       const panda_file::LiteralTag &tag) {
            this->EmplaceLiteralVals(literalArray, value, tag);
        });

    item->AddItems(literalArray);

    return true;
}

template <typename T>
static ValueItem *GeneratePrimitiveItemLesserInt32(AnnotationDataAccessor::Elem &annElem, ItemContainer &container,
                                                   ark::panda_file::Type::TypeId typeId)
{
    auto array = annElem.GetArrayValue();
    std::vector<ScalarValueItem> items;
    for (size_t j = 0; j < array.GetCount(); j++) {
        ScalarValueItem scalar(static_cast<uint32_t>(array.Get<T>(j)));
        items.emplace_back(std::move(scalar));
    }
    return static_cast<ValueItem *>(container.CreateItem<ArrayValueItem>(Type(typeId), std::move(items)));
}

template <typename T>
static ValueItem *GeneratePrimitiveItem(AnnotationDataAccessor::Elem &annElem, ItemContainer &container,
                                        ark::panda_file::Type::TypeId typeId)
{
    auto array = annElem.GetArrayValue();
    std::vector<ScalarValueItem> items;
    for (size_t j = 0; j < array.GetCount(); j++) {
        ScalarValueItem scalar(array.Get<T>(j));
        items.emplace_back(std::move(scalar));
    }
    return static_cast<ValueItem *>(container.CreateItem<ArrayValueItem>(Type(typeId), std::move(items)));
}

// CC-OFFNXT(G.FUN.01-CPP, huge_cyclomatic_complexity[C++], huge_method[C++]) big switch case
// NOLINTNEXTLINE(readability-function-size)
ValueItem *FileReader::SetElemValueItem(AnnotationDataAccessor::Tag &annTag, AnnotationDataAccessor::Elem &annElem)
{
    switch (annTag.GetItem()) {
        case '1':
        case '2':
        case '3': {
            auto scalar = annElem.GetScalarValue();
            return container_.GetOrCreateIntegerValueItem(scalar.Get<uint8_t>());
        }
        case '4':
        case '5': {
            auto scalar = annElem.GetScalarValue();
            return container_.GetOrCreateIntegerValueItem(scalar.Get<uint16_t>());
        }
        case '6':
        case '7': {
            auto scalar = annElem.GetScalarValue();
            return container_.GetOrCreateIntegerValueItem(scalar.Get<uint32_t>());
        }
        case '8':
        case '9': {
            auto scalar = annElem.GetScalarValue();
            return container_.GetOrCreateLongValueItem(scalar.Get<uint64_t>());
        }
        case 'A': {
            auto scalar = annElem.GetScalarValue();
            return container_.GetOrCreateFloatValueItem(scalar.Get<float>());
        }
        case 'B': {
            auto scalar = annElem.GetScalarValue();
            return container_.GetOrCreateDoubleValueItem(scalar.Get<double>());
        }
        case 'C': {
            auto scalar = annElem.GetScalarValue();
            const File::EntityId strId(scalar.Get<uint32_t>());
            auto data = file_->GetStringData(strId);
            std::string itemStr(utf::Mutf8AsCString(data.data));
            auto *strItem = container_.GetOrCreateStringItem(itemStr);
            return container_.GetOrCreateIdValueItem(strItem);
        }
        case 'D': {
            auto scalar = annElem.GetScalarValue();
            const File::EntityId classId {scalar.Get<uint32_t>()};
            return container_.GetOrCreateIdValueItem(CreateGenericClassItem(classId));
        }
        case 'E': {
            auto scalar = annElem.GetScalarValue();
            const File::EntityId methodId {scalar.Get<uint32_t>()};
            MethodDataAccessor methodAcc(*file_, methodId);
            auto *clsItem = CreateGenericClassItem(methodAcc.GetClassId());
            return container_.GetOrCreateIdValueItem(CreateGenericMethodItem(clsItem, methodId));
        }
        case 'F': {
            auto scalar = annElem.GetScalarValue();
            const File::EntityId fieldId {scalar.Get<uint32_t>()};
            FieldDataAccessor fieldAcc(*file_, fieldId);
            auto *clsItem = CreateGenericClassItem(fieldAcc.GetClassId());
            return container_.GetOrCreateIdValueItem(CreateGenericFieldItem(clsItem, fieldId));
        }
        case 'G': {
            auto scalar = annElem.GetScalarValue();
            const File::EntityId annItemId {scalar.Get<uint32_t>()};
            return container_.GetOrCreateIdValueItem(CreateAnnotationItem(annItemId));
        }
        case 'J': {
            LOG(FATAL, PANDAFILE) << "MethodHandle is not supported so far";
            break;
        }
        case '*': {
            return container_.GetOrCreateIntegerValueItem(0);
        }
        case 'K': {
            return GeneratePrimitiveItemLesserInt32<uint8_t>(annElem, container_, Type::TypeId::U1);
        }
        case 'L': {
            return GeneratePrimitiveItemLesserInt32<uint8_t>(annElem, container_, Type::TypeId::I8);
        }
        case 'M': {
            return GeneratePrimitiveItemLesserInt32<uint8_t>(annElem, container_, Type::TypeId::U8);
        }
        case 'N': {
            return GeneratePrimitiveItemLesserInt32<uint16_t>(annElem, container_, Type::TypeId::I16);
        }
        case 'O': {
            return GeneratePrimitiveItemLesserInt32<uint16_t>(annElem, container_, Type::TypeId::U16);
        }
        case 'P': {
            return GeneratePrimitiveItem<uint32_t>(annElem, container_, Type::TypeId::I32);
        }
        case 'Q': {
            return GeneratePrimitiveItem<uint32_t>(annElem, container_, Type::TypeId::U32);
        }
        case 'R': {
            return GeneratePrimitiveItem<uint64_t>(annElem, container_, Type::TypeId::I64);
        }
        case 'S': {
            return GeneratePrimitiveItem<uint64_t>(annElem, container_, Type::TypeId::U64);
        }
        case 'T': {
            return GeneratePrimitiveItem<float>(annElem, container_, Type::TypeId::F32);
        }
        case 'U': {
            return GeneratePrimitiveItem<double>(annElem, container_, Type::TypeId::F64);
        }
        case 'V': {
            auto array = annElem.GetArrayValue();
            std::vector<ScalarValueItem> items;
            for (size_t j = 0; j < array.GetCount(); j++) {
                const File::EntityId strId(array.Get<uint32_t>(j));
                auto data = file_->GetStringData(strId);
                std::string itemStr(utf::Mutf8AsCString(data.data));
                items.emplace_back(ScalarValueItem(container_.GetOrCreateStringItem(itemStr)));
            }
            return static_cast<ValueItem *>(
                container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::REFERENCE), std::move(items)));
        }
        case 'W': {
            auto array = annElem.GetArrayValue();
            std::vector<ScalarValueItem> items;
            for (size_t j = 0; j < array.GetCount(); j++) {
                const File::EntityId classId {array.Get<uint32_t>(j)};
                BaseClassItem *clsItem = nullptr;
                if (file_->IsExternal(classId)) {
                    clsItem = CreateForeignClassItem(classId);
                } else {
                    clsItem = CreateClassItem(classId);
                }
                ASSERT(clsItem != nullptr);
                items.emplace_back(ScalarValueItem(clsItem));
            }
            return static_cast<ValueItem *>(
                container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::REFERENCE), std::move(items)));
        }
        case 'X': {
            auto array = annElem.GetArrayValue();
            std::vector<ScalarValueItem> items;
            for (size_t j = 0; j < array.GetCount(); j++) {
                const File::EntityId methodId {array.Get<uint32_t>(j)};
                MethodDataAccessor methodAcc(*file_, methodId);
                auto *clsItem = CreateGenericClassItem(methodAcc.GetClassId());
                items.emplace_back(ScalarValueItem(CreateGenericMethodItem(clsItem, methodId)));
            }
            return static_cast<ValueItem *>(
                container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::REFERENCE), std::move(items)));
        }
        case 'Y': {
            auto array = annElem.GetArrayValue();
            std::vector<ScalarValueItem> items;
            for (size_t j = 0; j < array.GetCount(); j++) {
                const File::EntityId fieldId {array.Get<uint32_t>(j)};
                FieldDataAccessor fieldAcc(*file_, fieldId);
                auto *clsItem = CreateGenericClassItem(fieldAcc.GetClassId());
                items.emplace_back(ScalarValueItem(CreateGenericFieldItem(clsItem, fieldId)));
            }
            return static_cast<ValueItem *>(
                container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::REFERENCE), std::move(items)));
        }
        case 'H': {
            // ARRAY can appear for empty arrays only
            ASSERT(annElem.GetArrayValue().GetCount() == 0);
            return static_cast<ValueItem *>(
                container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::VOID), std::vector<ScalarValueItem>()));
        }
        case 'Z': {
            auto array = annElem.GetArrayValue();
            std::vector<ScalarValueItem> items;
            for (size_t j = 0; j < array.GetCount(); j++) {
                const File::EntityId annItemId {array.Get<uint32_t>(j)};
                items.emplace_back(CreateAnnotationItem(annItemId));
            }
            return static_cast<ValueItem *>(
                container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::REFERENCE), std::move(items)));
        }
        case '@': {
            // NOTE(nsizov): support it
            LOG(FATAL, PANDAFILE) << "MethodHandle is not supported so far";
            break;
        }
            // array
        case 'I':
            // VOID(I) and ARRAY(H) value should not appear
        default:
            UNREACHABLE();
    }
    return nullptr;
}

AnnotationItem *FileReader::CreateAnnotationItem(File::EntityId annId)
{
    auto it = itemsDone_.find(annId);
    if (it != itemsDone_.end()) {
        return static_cast<AnnotationItem *>(it->second);
    }

    AnnotationDataAccessor annAcc(*file_, annId);
    File::EntityId annClassId {annAcc.GetClassId()};
    AnnotationItem *annItem = nullptr;

    if (!file_->IsExternal(annClassId)) {
        auto *annClassItem = CreateClassItem(annClassId);
        annItem = container_.CreateItem<AnnotationItem>(annClassItem, std::vector<AnnotationItem::Elem>(),
                                                        std::vector<AnnotationItem::Tag>());
    } else {
        auto *annClassItem = CreateForeignClassItem(annClassId);
        annItem = container_.CreateItem<AnnotationItem>(annClassItem, std::vector<AnnotationItem::Elem>(),
                                                        std::vector<AnnotationItem::Tag>());
    }

    ASSERT(annItem != nullptr);

    itemsDone_.insert({annId, static_cast<BaseItem *>(annItem)});

    std::vector<AnnotationItem::Elem> itemElements;
    std::vector<AnnotationItem::Tag> tagElements;

    for (size_t i = 0; i < annAcc.GetCount(); i++) {
        AnnotationDataAccessor::Tag annTag = annAcc.GetTag(i);
        AnnotationDataAccessor::Elem annElem = annAcc.GetElement(i);
        ValueItem *elemValueItem = SetElemValueItem(annTag, annElem);

        ASSERT(elemValueItem != nullptr);

        tagElements.emplace_back(AnnotationItem::Tag(static_cast<char>(annTag.GetItem())));
        File::EntityId nameId(annElem.GetNameId());
        std::string annotNameStr(utf::Mutf8AsCString(file_->GetStringData(nameId).data));
        auto elemNameItem = container_.GetOrCreateStringItem(annotNameStr);
        itemElements.emplace_back(AnnotationItem::Elem(elemNameItem, elemValueItem));
    }

    annItem->SetElements(std::move(itemElements));
    annItem->SetTags(std::move(tagElements));

    return annItem;
}

TypeItem *FileReader::CreateParamTypeItem(ProtoDataAccessor *protoAcc, size_t paramNum, size_t referenceNum)
{
    Type paramType = protoAcc->GetArgType(paramNum);
    TypeItem *paramTypeItem = nullptr;
    if (paramType.IsPrimitive()) {
        paramTypeItem = container_.GetOrCreatePrimitiveTypeItem(paramType);
    } else {
        const File::EntityId typeClsId = protoAcc->GetReferenceType(referenceNum);
        if (file_->IsExternal(typeClsId)) {
            paramTypeItem = CreateForeignClassItem(typeClsId);
        } else {
            paramTypeItem = CreateClassItem(typeClsId);
        }
    }

    ASSERT(paramTypeItem != nullptr);

    return paramTypeItem;
}

std::vector<MethodParamItem> FileReader::CreateMethodParamItems(ProtoDataAccessor *protoAcc,
                                                                MethodDataAccessor *methodAcc, size_t referenceNum)
{
    std::vector<MethodParamItem> paramItems;

    for (size_t i = 0; i < protoAcc->GetNumArgs(); i++) {
        TypeItem *paramTypeItem = CreateParamTypeItem(protoAcc, i, referenceNum);
        if (paramTypeItem->GetType().IsReference()) {
            referenceNum++;
        }
        paramItems.emplace_back(MethodParamItem(paramTypeItem));
    }

    auto paramAnnId = methodAcc->GetParamAnnotationId();
    if (paramAnnId) {
        ParamAnnotationsDataAccessor paramAcc(*file_, paramAnnId.value());
        for (size_t i = 0; i < protoAcc->GetNumArgs(); i++) {
            ParamAnnotationsDataAccessor::AnnotationArray annArr = paramAcc.GetAnnotationArray(i);
            annArr.EnumerateAnnotations([this, &paramItems, &i](File::EntityId annId) {
                auto annItem = CreateAnnotationItem(annId);
                paramItems[i].AddAnnotation(annItem);
            });
        }
    }

    auto runtimeParamAnnId = methodAcc->GetRuntimeParamAnnotationId();
    if (runtimeParamAnnId) {
        ParamAnnotationsDataAccessor paramAcc(*file_, runtimeParamAnnId.value());
        for (size_t i = 0; i < protoAcc->GetNumArgs(); i++) {
            ParamAnnotationsDataAccessor::AnnotationArray annArr = paramAcc.GetAnnotationArray(i);
            annArr.EnumerateAnnotations([this, &paramItems, &i](File::EntityId annId) {
                auto annItem = CreateAnnotationItem(annId);
                paramItems[i].AddRuntimeAnnotation(annItem);
            });
        }
    }

    return paramItems;
}

DebugInfoItem *FileReader::CreateDebugInfoItem(File::EntityId debugInfoId)
{
    auto it = itemsDone_.find(debugInfoId);
    if (it != itemsDone_.end()) {
        auto itemType = it->second->GetBaseItemType();
        if (itemType != ItemTypes::DEBUG_INFO_ITEM) {
            LOG(FATAL, PANDAFILE) << "ItemType Error " << static_cast<int>(itemType);
        }
        return static_cast<DebugInfoItem *>(it->second);
    }

    panda_file::DebugInfoDataAccessor debugAcc(*file_, debugInfoId);

    const auto lnpId = file_->GetIdFromPointer(debugAcc.GetLineNumberProgram());

    LineNumberProgramItem *lnpItem;

    if (auto oldLnp = itemsDone_.find(lnpId); oldLnp != itemsDone_.end()) {
        ASSERT(oldLnp->second->GetItemType() == ItemTypes::LINE_NUMBER_PROGRAM_ITEM);
        lnpItem = static_cast<LineNumberProgramItem *>(oldLnp->second);
        container_.IncRefLineNumberProgramItem(lnpItem);
    } else {
        lnpItem = container_.CreateLineNumberProgramItem();
        itemsDone_.emplace(lnpId, lnpItem);
    }

    auto *debugInfoItem = container_.CreateItem<DebugInfoItem>(lnpItem);
    itemsDone_.insert({debugInfoId, static_cast<BaseItem *>(debugInfoItem)});

    debugInfoItem->SetLineNumber(debugAcc.GetLineStart());
    debugAcc.EnumerateParameters([this, &debugInfoItem](File::EntityId paramId) {
        auto data = file_->GetStringData(paramId);
        std::string itemStr(utf::Mutf8AsCString(data.data));
        auto *stringItem = container_.GetOrCreateStringItem(itemStr);
        debugInfoItem->AddParameter(stringItem);
    });
    debugInfoItem->SetBaseItemType(ItemTypes::DEBUG_INFO_ITEM);
    return debugInfoItem;
}

BaseClassItem *FileReader::GetCatchTypeItem(CodeDataAccessor::CatchBlock &catchBlock, File::EntityId methodId,
                                            MethodItem *methodItem)
{
    BaseClassItem *catchTypeItem = nullptr;
    auto typeIdx = catchBlock.GetTypeIdx();
    if (typeIdx != panda_file::INVALID_INDEX) {
        File::EntityId catchClsId = file_->ResolveClassIndex(methodId, catchBlock.GetTypeIdx());
        if (file_->IsExternal(catchClsId)) {
            catchTypeItem = CreateForeignClassItem(catchClsId);
        } else {
            catchTypeItem = CreateClassItem(catchClsId);
        }
        methodItem->AddIndexDependency(catchTypeItem);
    }
    return catchTypeItem;
}

void FileReader::SetMethodCodeIfPresent(std::optional<File::EntityId> &codeId, MethodItem *methodItem,
                                        File::EntityId &methodId)
{
    CodeDataAccessor codeAcc(*file_, codeId.value());
    std::vector<uint8_t> instructions(codeAcc.GetCodeSize());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    instructions.assign(codeAcc.GetInstructions(), codeAcc.GetInstructions() + codeAcc.GetCodeSize());
    auto *codeItem =
        container_.CreateItem<CodeItem>(codeAcc.GetNumVregs(), codeAcc.GetNumArgs(), std::move(instructions));

    codeAcc.EnumerateTryBlocks([this, &methodItem, &methodId, &codeItem](CodeDataAccessor::TryBlock &tryBlock) {
        std::vector<CodeItem::CatchBlock> catchBlocks;
        tryBlock.EnumerateCatchBlocks(
            [this, &methodItem, &methodId, &catchBlocks](CodeDataAccessor::CatchBlock &catchBlock) {
                BaseClassItem *catchTypeItem = this->GetCatchTypeItem(catchBlock, methodId, methodItem);
                catchBlocks.emplace_back(CodeItem::CatchBlock(methodItem, catchTypeItem, catchBlock.GetHandlerPc(),
                                                              catchBlock.GetCodeSize()));
                return true;
            });
        codeItem->AddTryBlock(CodeItem::TryBlock(tryBlock.GetStartPc(), tryBlock.GetLength(), std::move(catchBlocks)));
        return true;
    });

    methodItem->SetCode(codeItem);
}

TypeItem *FileReader::SetRetType(ProtoDataAccessor &protoAcc, size_t &referenceNum)

{
    Type retType = protoAcc.GetReturnType();
    TypeItem *retTypeItem = nullptr;
    if (retType.IsPrimitive()) {
        retTypeItem = container_.GetOrCreatePrimitiveTypeItem(retType);
    } else {
        const File::EntityId typeClsId = protoAcc.GetReferenceType(referenceNum);
        if (file_->IsExternal(typeClsId)) {
            retTypeItem = CreateForeignClassItem(typeClsId);
        } else {
            retTypeItem = CreateClassItem(typeClsId);
        }
        referenceNum++;
    }
    return retTypeItem;
}

MethodItem *FileReader::CreateMethodItem(ClassItem *cls, File::EntityId methodId)
{
    auto it = itemsDone_.find(methodId);
    if (it != itemsDone_.end()) {
        return static_cast<MethodItem *>(it->second);
    }

    MethodDataAccessor methodAcc(*file_, methodId);
    auto data = file_->GetStringData(methodAcc.GetNameId());
    std::string methodName(utf::Mutf8AsCString(data.data));
    auto *methodStrItem = container_.GetOrCreateStringItem(methodName);

    ProtoDataAccessor protoAcc(*file_, methodAcc.GetProtoId());
    size_t referenceNum = 0;
    TypeItem *retTypeItem = SetRetType(protoAcc, referenceNum);
    ASSERT(retTypeItem != nullptr);
    auto paramItems = CreateMethodParamItems(&protoAcc, &methodAcc, referenceNum);
    // Double check if we done this method while computing params
    auto itCheck = itemsDone_.find(methodId);
    if (itCheck != itemsDone_.end()) {
        return static_cast<MethodItem *>(itCheck->second);
    }
    auto *protoItem = container_.GetOrCreateProtoItem(retTypeItem, paramItems);

    auto *methodItem = cls->AddMethod(methodStrItem, protoItem, methodAcc.GetAccessFlags(), std::move(paramItems));

    if (methodItem->HasRuntimeParamAnnotations()) {
        container_.CreateItem<ParamAnnotationsItem>(methodItem, true);
    }

    if (methodItem->HasParamAnnotations()) {
        container_.CreateItem<ParamAnnotationsItem>(methodItem, false);
    }

    itemsDone_.insert({methodId, static_cast<BaseItem *>(methodItem)});

    methodAcc.EnumerateAnnotations(
        [this, &methodItem](File::EntityId annId) { methodItem->AddAnnotation(CreateAnnotationItem(annId)); });

    methodAcc.EnumerateRuntimeAnnotations(
        [this, &methodItem](File::EntityId annId) { methodItem->AddRuntimeAnnotation(CreateAnnotationItem(annId)); });

    methodAcc.EnumerateTypeAnnotations(
        [this, &methodItem](File::EntityId annId) { methodItem->AddTypeAnnotation(CreateAnnotationItem(annId)); });

    methodAcc.EnumerateRuntimeTypeAnnotations([this, &methodItem](File::EntityId annId) {
        methodItem->AddRuntimeTypeAnnotation(CreateAnnotationItem(annId));
    });

    auto codeId = methodAcc.GetCodeId();
    if (codeId) {
        SetMethodCodeIfPresent(codeId, methodItem, methodId);
    }

    auto debugInfoId = methodAcc.GetDebugInfoId();
    if (debugInfoId) {
        methodItem->SetDebugInfo(CreateDebugInfoItem(debugInfoId.value()));
    }

    auto sourceLang = methodAcc.GetSourceLang();
    if (sourceLang) {
        methodItem->SetSourceLang(sourceLang.value());
    }

    return methodItem;
}

MethodHandleItem *FileReader::CreateMethodHandleItem(File::EntityId mhId)
{
    (void)mhId;
    ASSERT(false);
    return nullptr;  // STUB
}

void FileReader::SetFieldValue(FieldItem *fieldItem, Type fieldType, FieldDataAccessor &fieldAcc)
{
    switch (fieldType.GetId()) {
        case Type::TypeId::U1:
        case Type::TypeId::I8:
        case Type::TypeId::U8:
            SetIntegerFieldValue<uint8_t>(&fieldAcc, fieldItem);
            break;
        case Type::TypeId::I16:
        case Type::TypeId::U16:
            SetIntegerFieldValue<uint16_t>(&fieldAcc, fieldItem);
            break;
        case Type::TypeId::I32:
        case Type::TypeId::U32:
            SetIntegerFieldValue<uint32_t>(&fieldAcc, fieldItem);
            break;
        case Type::TypeId::I64:
        case Type::TypeId::U64:
            SetIntegerFieldValue<uint64_t>(&fieldAcc, fieldItem);
            break;
        case Type::TypeId::F32:
            SetFloatFieldValue<float>(&fieldAcc, fieldItem);
            break;
        case Type::TypeId::F64:
            SetFloatFieldValue<double>(&fieldAcc, fieldItem);
            break;
        case Type::TypeId::REFERENCE:
            SetStringFieldValue(&fieldAcc, fieldItem);
            break;
        case Type::TypeId::TAGGED:
        default:
            UNREACHABLE();
            break;
    }
}

FieldItem *FileReader::CreateFieldItem(ClassItem *cls, File::EntityId fieldId)
{
    auto it = itemsDone_.find(fieldId);
    if (it != itemsDone_.end()) {
        return static_cast<FieldItem *>(it->second);
    }

    FieldDataAccessor fieldAcc(*file_, fieldId);

    auto data = file_->GetStringData(fieldAcc.GetNameId());
    std::string stringName(utf::Mutf8AsCString(data.data));
    auto *fieldName = container_.GetOrCreateStringItem(stringName);
    Type fieldType = Type::GetTypeFromFieldEncoding(fieldAcc.GetType());

    TypeItem *fieldTypeItem = nullptr;
    if (fieldType.IsReference()) {
        File::EntityId typeId(fieldAcc.GetType());
        if (file_->IsExternal(typeId)) {
            fieldTypeItem = CreateForeignClassItem(typeId);
        } else {
            fieldTypeItem = CreateClassItem(typeId);
            // Double check if we done this field while generated class item
            auto itCheck = itemsDone_.find(fieldId);
            if (itCheck != itemsDone_.end()) {
                return static_cast<FieldItem *>(itCheck->second);
            }
        }
    } else {
        fieldTypeItem = container_.GetOrCreatePrimitiveTypeItem(fieldType.GetId());
    }

    ASSERT(fieldTypeItem != nullptr);

    FieldItem *fieldItem = cls->AddField(fieldName, fieldTypeItem, fieldAcc.GetAccessFlags());
    itemsDone_.insert({fieldId, static_cast<BaseItem *>(fieldItem)});

    SetFieldValue(fieldItem, fieldType, fieldAcc);

    fieldAcc.EnumerateAnnotations(
        [this, &fieldItem](File::EntityId annId) { fieldItem->AddAnnotation(CreateAnnotationItem(annId)); });

    fieldAcc.EnumerateRuntimeAnnotations(
        [this, &fieldItem](File::EntityId annId) { fieldItem->AddRuntimeAnnotation(CreateAnnotationItem(annId)); });

    fieldAcc.EnumerateRuntimeTypeAnnotations(
        [this, &fieldItem](File::EntityId annId) { fieldItem->AddRuntimeTypeAnnotation(CreateAnnotationItem(annId)); });

    fieldAcc.EnumerateTypeAnnotations(
        [this, &fieldItem](File::EntityId annId) { fieldItem->AddTypeAnnotation(CreateAnnotationItem(annId)); });

    return fieldItem;
}

ForeignMethodItem *FileReader::CreateForeignMethodItem(BaseClassItem *fcls, File::EntityId methodId)
{
    auto it = itemsDone_.find(methodId);
    if (it != itemsDone_.end()) {
        return static_cast<ForeignMethodItem *>(it->second);
    }

    MethodDataAccessor methodAcc(*file_, methodId);
    auto data = file_->GetStringData(methodAcc.GetNameId());
    std::string methodName(utf::Mutf8AsCString(data.data));
    auto *methodStrItem = container_.GetOrCreateStringItem(methodName);

    ProtoDataAccessor protoAcc(*file_, methodAcc.GetProtoId());
    Type retType = protoAcc.GetReturnType();
    size_t referenceNum = 0;
    TypeItem *retTypeItem = nullptr;
    if (retType.IsPrimitive()) {
        retTypeItem = container_.GetOrCreatePrimitiveTypeItem(retType);
    } else {
        const File::EntityId typeClsId = protoAcc.GetReferenceType(referenceNum);
        if (file_->IsExternal(typeClsId)) {
            retTypeItem = CreateForeignClassItem(typeClsId);
        } else {
            retTypeItem = CreateClassItem(typeClsId);
        }
        referenceNum++;
    }
    ASSERT(retTypeItem != nullptr);
    auto paramItems = CreateMethodParamItems(&protoAcc, &methodAcc, referenceNum);
    // Double check if we done this method while computing params
    auto itCheck = itemsDone_.find(methodId);
    if (itCheck != itemsDone_.end()) {
        return static_cast<ForeignMethodItem *>(itCheck->second);
    }
    auto *protoItem = container_.GetOrCreateProtoItem(retTypeItem, paramItems);

    auto *methodItem =
        container_.CreateItem<ForeignMethodItem>(fcls, methodStrItem, protoItem, methodAcc.GetAccessFlags());

    itemsDone_.insert({methodId, static_cast<BaseItem *>(methodItem)});

    return methodItem;
}

ForeignFieldItem *FileReader::CreateForeignFieldItem(BaseClassItem *fcls, File::EntityId fieldId)
{
    auto it = itemsDone_.find(fieldId);
    if (it != itemsDone_.end()) {
        return static_cast<ForeignFieldItem *>(it->second);
    }

    FieldDataAccessor fieldAcc(*file_, fieldId);

    auto data = file_->GetStringData(fieldAcc.GetNameId());
    std::string stringName(utf::Mutf8AsCString(data.data));
    auto *fieldName = container_.GetOrCreateStringItem(stringName);
    Type fieldType = Type::GetTypeFromFieldEncoding(fieldAcc.GetType());
    TypeItem *fieldTypeItem = nullptr;
    if (fieldType.IsReference()) {
        File::EntityId typeId(fieldAcc.GetType());
        if (file_->IsExternal(typeId)) {
            fieldTypeItem = CreateForeignClassItem(typeId);
        } else {
            fieldTypeItem = CreateClassItem(typeId);
            // Double check if we done this field while generated class item
            auto itCheck = itemsDone_.find(fieldId);
            if (itCheck != itemsDone_.end()) {
                return static_cast<ForeignFieldItem *>(itCheck->second);
            }
        }
    } else {
        fieldTypeItem = container_.GetOrCreatePrimitiveTypeItem(fieldType.GetId());
    }

    ASSERT(fieldTypeItem != nullptr);

    auto *fieldItem =
        container_.CreateItem<ForeignFieldItem>(fcls, fieldName, fieldTypeItem, fieldAcc.GetAccessFlags());
    itemsDone_.insert({fieldId, static_cast<BaseItem *>(fieldItem)});

    return fieldItem;
}

ForeignClassItem *FileReader::CreateForeignClassItem(File::EntityId classId)
{
    auto it = itemsDone_.find(classId);
    if (it != itemsDone_.end()) {
        return static_cast<ForeignClassItem *>(it->second);
    }

    std::string className(utf::Mutf8AsCString(file_->GetStringData(classId).data));
    auto *classItem = container_.GetOrCreateForeignClassItem(className);

    itemsDone_.insert({classId, static_cast<BaseItem *>(classItem)});

    return classItem;
}

BaseItem *FileReader::CheckAndGetExistingFileItem(File::EntityId id, ItemTypes itemType)
{
    auto it = itemsDone_.find(id);
    if (it != itemsDone_.end()) {
        auto iType = it->second->GetItemType();
        if (iType != itemType) {
            LOG(FATAL, PANDAFILE) << ItemTypeToString(itemType) << " ItemType Error " << static_cast<int>(iType);
        }
        return it->second;
    }
    return nullptr;
}

ClassItem *FileReader::CreateClassItem(File::EntityId classId)
{
    BaseItem *item = CheckAndGetExistingFileItem(classId, ItemTypes::CLASS_ITEM);
    if (item != nullptr) {
        return static_cast<ClassItem *>(item);
    }
    ClassDataAccessor classAcc(*file_, classId);

    std::string className(utf::Mutf8AsCString(file_->GetStringData(classId).data));
    auto *classItem = container_.GetOrCreateClassItem(className);

    itemsDone_.insert({classId, static_cast<BaseItem *>(classItem)});

    classItem->SetAccessFlags(classAcc.GetAccessFlags());

    auto sourceLangOpt = classAcc.GetSourceLang();
    if (sourceLangOpt) {
        classItem->SetSourceLang(sourceLangOpt.value());
    }

    auto superClassId = classAcc.GetSuperClassId();
    if (superClassId.GetOffset() != 0) {
        if (superClassId.GetOffset() == classId.GetOffset()) {
            LOG(FATAL, PANDAFILE) << "Class " << className << " has cyclic inheritance";
        }

        if (file_->IsExternal(superClassId)) {
            classItem->SetSuperClass(CreateForeignClassItem(superClassId));
        } else {
            classItem->SetSuperClass(CreateClassItem(superClassId));
        }
    }

    classAcc.EnumerateInterfaces([this, &classItem](File::EntityId ifaceId) {
        if (file_->IsExternal(ifaceId)) {
            classItem->AddInterface(CreateForeignClassItem(ifaceId));
        } else {
            classItem->AddInterface(CreateClassItem(ifaceId));
        }
    });

    classAcc.EnumerateAnnotations(
        [this, &classItem](File::EntityId annId) { classItem->AddAnnotation(CreateAnnotationItem(annId)); });

    classAcc.EnumerateRuntimeAnnotations(
        [this, &classItem](File::EntityId annId) { classItem->AddRuntimeAnnotation(CreateAnnotationItem(annId)); });

    classAcc.EnumerateTypeAnnotations(
        [this, &classItem](File::EntityId annId) { classItem->AddTypeAnnotation(CreateAnnotationItem(annId)); });

    classAcc.EnumerateFields(
        [this, &classItem](FieldDataAccessor &fieldAcc) { CreateFieldItem(classItem, fieldAcc.GetFieldId()); });

    classAcc.EnumerateMethods(
        [this, &classItem](MethodDataAccessor &methodAcc) { CreateMethodItem(classItem, methodAcc.GetMethodId()); });

    auto sourceFileId = classAcc.GetSourceFileId();
    if (sourceFileId) {
        std::string sourceFile = utf::Mutf8AsCString(file_->GetStringData(sourceFileId.value()).data);
        classItem->SetSourceFile(container_.GetOrCreateStringItem(sourceFile));
    }

    ASSERT(classItem != nullptr);

    return classItem;
}

bool FileReader::ReadLiteralArrayItems()
{
    const auto litArraysId = file_->GetLiteralArraysId();
    LiteralDataAccessor litArrayAccessor(*file_, litArraysId);
    size_t numLitarrays = litArrayAccessor.GetLiteralNum();

    for (size_t i = 0; i < numLitarrays; i++) {
        auto id = litArrayAccessor.GetLiteralArrayId(i);
        if (!CreateLiteralArrayItem(&litArrayAccessor, id, i)) {
            return false;
        }
    }

    return true;
}

bool FileReader::TryCreateMethodItem(File::EntityId methodId)
{
    MethodDataAccessor methodAcc(*file_, methodId);
    File::EntityId classId(methodAcc.GetClassId());
    if (file_->IsExternal(classId)) {
        auto *fclassItem = CreateForeignClassItem(classId);
        ASSERT(file_->IsExternal(methodId));
        if (CreateForeignMethodItem(fclassItem, methodId) == nullptr) {
            return false;
        }
    } else {
        auto *classItem = CreateClassItem(classId);
        if (file_->IsExternal(methodId)) {
            if (CreateForeignMethodItem(classItem, methodId) == nullptr) {
                return false;
            }
        } else if (CreateMethodItem(classItem, methodId) == nullptr) {
            return false;
        }
    }

    return true;
}

bool FileReader::TryCreateFieldItem(File::EntityId fieldId)
{
    FieldDataAccessor fieldAcc(*file_, fieldId);
    File::EntityId classId(fieldAcc.GetClassId());
    if (file_->IsExternal(classId)) {
        ASSERT(file_->IsExternal(fieldId));
        auto *fclassItem = CreateForeignClassItem(fieldAcc.GetClassId());
        if (CreateForeignFieldItem(fclassItem, fieldId) == nullptr) {
            return false;
        }
    } else {
        auto *classItem = CreateClassItem(fieldAcc.GetClassId());
        if (file_->IsExternal(fieldId)) {
            if (CreateForeignFieldItem(classItem, fieldId) == nullptr) {
                return false;
            }
        } else if (CreateFieldItem(classItem, fieldId) == nullptr) {
            return false;
        }
    }

    return true;
}

bool FileReader::ReadRegionHeaders()
{
    auto indexHeaders = file_->GetRegionHeaders();
    for (const auto &header : indexHeaders) {
        auto methodIndex = file_->GetMethodIndex(&header);
        for (auto methodId : methodIndex) {
            if (!TryCreateMethodItem(methodId)) {
                return false;
            }
        }
        auto fieldIndex = file_->GetFieldIndex(&header);
        for (auto fieldId : fieldIndex) {
            if (!TryCreateFieldItem(fieldId)) {
                return false;
            }
        }
    }
    return true;
}

bool FileReader::ReadClasses()
{
    const auto classIdx = file_->GetClasses();

    for (unsigned int id : classIdx) {
        File::EntityId eid(id);
        if (file_->IsExternal(eid)) {
            CreateForeignClassItem(eid);
        } else {
            CreateClassItem(eid);
        }
    }

    return true;
}

void FileReader::UpdateDebugInfoDependecies(File::EntityId debugInfoId)
{
    auto updater = FileReaderDebugInfoUpdater(file_.get(), &container_);
    updater.Scrap(debugInfoId);
}

void FileReader::UpdateDebugInfo(DebugInfoItem *debugInfoItem, File::EntityId debugInfoId)
{
    ASSERT(debugInfoItem != nullptr);
    ASSERT(file_ != nullptr);
    auto updater = FileReaderDebugInfoUpdater(file_.get(), &container_);
    updater.Emit(debugInfoItem->GetLineNumberProgram(), debugInfoItem->GetConstantPool(), debugInfoId);
}

void FileReader::InstCheckByFlags(BytecodeInstruction &inst, MethodItem *methodItem,
                                  const std::map<BaseItem *, File::EntityId> &reverseDone)
{
    using Flags = ark::BytecodeInst<ark::BytecodeInstMode::FAST>::Flags;

    if (inst.HasFlag(Flags::TYPE_ID)) {
        BytecodeId bId = inst.GetId();
        File::Index idx = bId.AsIndex();
        File::EntityId methodId = reverseDone.find(methodItem)->second;
        File::EntityId oldId = file_->ResolveClassIndex(methodId, idx);
        ASSERT(itemsDone_.find(oldId) != itemsDone_.end());
        auto *idxItem = static_cast<IndexedItem *>(itemsDone_.find(oldId)->second);
        methodItem->AddIndexDependency(idxItem);
    } else if (inst.HasFlag(Flags::METHOD_ID) || inst.HasFlag(Flags::STATIC_METHOD_ID)) {
        BytecodeId bId = inst.GetId();
        File::Index idx = bId.AsIndex();
        File::EntityId methodId = reverseDone.find(methodItem)->second;
        File::EntityId oldId = file_->ResolveMethodIndex(methodId, idx);
        ASSERT(itemsDone_.find(oldId) != itemsDone_.end());
        auto *idxItem = static_cast<IndexedItem *>(itemsDone_.find(oldId)->second);
        methodItem->AddIndexDependency(idxItem);
    } else if (inst.HasFlag(Flags::FIELD_ID) || inst.HasFlag(Flags::STATIC_FIELD_ID)) {
        BytecodeId bId = inst.GetId();
        File::Index idx = bId.AsIndex();
        File::EntityId methodId = reverseDone.find(methodItem)->second;
        File::EntityId oldId = file_->ResolveFieldIndex(methodId, idx);
        ASSERT(itemsDone_.find(oldId) != itemsDone_.end());
        auto *idxItem = static_cast<IndexedItem *>(itemsDone_.find(oldId)->second);
        methodItem->AddIndexDependency(idxItem);
    } else if (inst.HasFlag(Flags::STRING_ID)) {
        BytecodeId bId = inst.GetId();
        File::EntityId oldId = bId.AsFileId();
        auto data = file_->GetStringData(oldId);
        std::string itemStr(utf::Mutf8AsCString(data.data));
        container_.GetOrCreateStringItem(itemStr);
    }
}

void FileReader::UpdateCodeAndDebugInfoDependencies(const std::map<BaseItem *, File::EntityId> &reverseDone)
{
    auto *classMap = container_.GetClassMap();

    // First pass, add dependencies bytecode -> new items
    for (const auto &it : *classMap) {
        auto *baseClassItem = it.second;
        if (baseClassItem->IsForeign()) {
            continue;
        }
        auto *classItem = static_cast<ClassItem *>(baseClassItem);
        classItem->VisitMethods([this, &reverseDone](BaseItem *paramItem) {
            auto *methodItem = static_cast<MethodItem *>(paramItem);
            auto *codeItem = methodItem->GetCode();
            if (codeItem == nullptr) {
                return true;
            }

            auto *debugInfoItem = methodItem->GetDebugInfo();
            if (debugInfoItem != nullptr) {
                UpdateDebugInfoDependecies(reverseDone.find(debugInfoItem)->second);
            }

            size_t offset = 0;
            BytecodeInstruction inst(codeItem->GetInstructions()->data());
            while (offset < codeItem->GetCodeSize()) {
                InstCheckByFlags(inst, methodItem, reverseDone);

                offset += inst.GetSize();
                inst = inst.GetNext();
            }
            return true;
        });
    }
}

void FileReader::InstUpdateId(CodeItem *codeItem, MethodItem *methodItem,
                              std::map<BaseItem *, File::EntityId> &reverseDone)
{
    using Flags = ark::BytecodeInst<ark::BytecodeInstMode::FAST>::Flags;

    size_t offset = 0;
    BytecodeInstruction inst(codeItem->GetInstructions()->data());
    while (offset < codeItem->GetCodeSize()) {
        if (inst.HasFlag(Flags::TYPE_ID)) {
            BytecodeId bId = inst.GetId();
            File::Index idx = bId.AsIndex();

            File::EntityId methodId = reverseDone.find(methodItem)->second;
            File::EntityId oldId = file_->ResolveClassIndex(methodId, idx);
            ASSERT(itemsDone_.find(oldId) != itemsDone_.end());

            auto *idxItem = static_cast<IndexedItem *>(itemsDone_.find(oldId)->second);
            uint32_t index = idxItem->GetIndex(methodItem);
            inst.UpdateId(BytecodeId(index));
        } else if (inst.HasFlag(Flags::METHOD_ID) || inst.HasFlag(Flags::STATIC_METHOD_ID)) {
            BytecodeId bId = inst.GetId();
            File::Index idx = bId.AsIndex();

            File::EntityId methodId = reverseDone.find(methodItem)->second;
            File::EntityId oldId = file_->ResolveMethodIndex(methodId, idx);
            ASSERT(itemsDone_.find(oldId) != itemsDone_.end());

            auto *idxItem = static_cast<IndexedItem *>(itemsDone_.find(oldId)->second);
            uint32_t index = idxItem->GetIndex(methodItem);
            inst.UpdateId(BytecodeId(index));
        } else if (inst.HasFlag(Flags::FIELD_ID) || inst.HasFlag(Flags::STATIC_FIELD_ID)) {
            BytecodeId bId = inst.GetId();
            File::Index idx = bId.AsIndex();

            File::EntityId methodId = reverseDone.find(methodItem)->second;
            File::EntityId oldId = file_->ResolveFieldIndex(methodId, idx);
            ASSERT(itemsDone_.find(oldId) != itemsDone_.end());

            auto *idxItem = static_cast<IndexedItem *>(itemsDone_.find(oldId)->second);
            uint32_t index = idxItem->GetIndex(methodItem);
            inst.UpdateId(BytecodeId(index));
        } else if (inst.HasFlag(Flags::STRING_ID)) {
            BytecodeId bId = inst.GetId();
            File::EntityId oldId = bId.AsFileId();
            auto data = file_->GetStringData(oldId);

            std::string itemStr(utf::Mutf8AsCString(data.data));
            auto *stringItem = container_.GetOrCreateStringItem(itemStr);
            inst.UpdateId(BytecodeId(stringItem->GetFileId().GetOffset()));
        }

        offset += inst.GetSize();
        inst = inst.GetNext();
    }
}

void FileReader::ComputeLayoutAndUpdateIndices()
{
    std::map<BaseItem *, File::EntityId> reverseDone;
    for (const auto &it : itemsDone_) {
        reverseDone.insert({it.second, it.first});
    }

    auto *classMap = container_.GetClassMap();

    UpdateCodeAndDebugInfoDependencies(reverseDone);

    container_.ComputeLayout();

    // Second pass, update debug info
    for (const auto &it : *classMap) {
        auto *baseClassItem = it.second;
        if (baseClassItem->IsForeign()) {
            continue;
        }
        auto *classItem = static_cast<ClassItem *>(baseClassItem);
        classItem->VisitMethods([this, &reverseDone](BaseItem *paramItem) {
            auto *methodItem = static_cast<MethodItem *>(paramItem);
            auto *codeItem = methodItem->GetCode();
            if (codeItem == nullptr) {
                return true;
            }

            auto *debugInfoItem = methodItem->GetDebugInfo();
            if (debugInfoItem != nullptr) {
                UpdateDebugInfo(debugInfoItem, reverseDone.find(debugInfoItem)->second);
            }

            return true;
        });
    }

    container_.DeduplicateItems(false);
    container_.ComputeLayout();

    std::unordered_set<CodeItem *> codeItemsDone;

    // Third pass, update bytecode indices
    for (const auto &it : *classMap) {
        auto *baseClassItem = it.second;
        if (baseClassItem->IsForeign()) {
            continue;
        }
        auto *classItem = static_cast<ClassItem *>(baseClassItem);
        classItem->VisitMethods([this, &reverseDone, &codeItemsDone](BaseItem *paramItem) {
            auto *methodItem = static_cast<MethodItem *>(paramItem);
            auto *codeItem = methodItem->GetCode();

            auto codeIt = codeItemsDone.find(codeItem);
            if (codeItem == nullptr || codeIt != codeItemsDone.end()) {
                return true;
            }

            InstUpdateId(codeItem, methodItem, reverseDone);

            codeItemsDone.insert(codeItem);

            return true;
        });
    }
}

}  // namespace ark::panda_file
