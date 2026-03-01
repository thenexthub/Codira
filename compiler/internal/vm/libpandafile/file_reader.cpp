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

#include <cstdint>

#include "libpandafile/bytecode_instruction-inl.h"
#include "libpandafile/line_number_program.h"
#include "libpandafile/literal_data_accessor-inl.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "libpandafile/code_data_accessor-inl.h"
#include "libpandafile/debug_data_accessor-inl.h"
#include "libpandafile/field_data_accessor-inl.h"
#include "libpandafile/method_data_accessor-inl.h"

#include "file.h"

#include "libpandabase/utils/utf.h"

#include "libpandafile/file_reader.h"

namespace panda::panda_file {

bool FileReader::ReadContainer()
{
    if (!ReadClasses()) {
        return false;
    }
    if (!ReadLiteralArrayItems()) {
        return false;
    }
    if (!ReadIndexHeaders()) {
        return false;
    }

    ComputeLayoutAndUpdateIndices();

    return true;
}

/* static */
bool FileReader::CreateLiteralArrayItem(const LiteralDataAccessor::LiteralValue &lit_value, const LiteralTag &tag,
                                        File::EntityId array_id)
{
    auto it = items_done_.find(array_id);
    if (it != items_done_.end()) {
        return true;
    }

    LiteralArrayItem *item = container_.GetOrCreateLiteralArrayItem(std::to_string(array_id.GetOffset()));
    items_done_.insert({array_id, static_cast<BaseItem *>(item)});

    File::EntityId id(std::get<uint32_t>(lit_value));
    auto sp = file_->GetSpanFromId(id);

    std::vector<panda_file::LiteralItem> literal_array;
    literal_array.emplace_back(static_cast<uint8_t>(tag));
    switch (tag) {
        case panda_file::LiteralTag::BOOL: {
            auto v = helpers::Read<sizeof(bool)>(&sp);
            literal_array.emplace_back(static_cast<uint8_t>(v));
            break;
        }
        case panda_file::LiteralTag::TAGVALUE:
        case panda_file::LiteralTag::ACCESSOR:
        case panda_file::LiteralTag::NULLVALUE: {
            auto v = helpers::Read<sizeof(uint8_t)>(&sp);
            literal_array.emplace_back(v);
            break;
        }
        case panda_file::LiteralTag::ARRAY_U1:
        case panda_file::LiteralTag::ARRAY_I8:
        case panda_file::LiteralTag::ARRAY_U8: {
            auto len = helpers::Read<sizeof(uint32_t)>(&sp);
            literal_array.emplace_back(len);
            for (size_t i = 0; i < len; i++) {
                auto v = helpers::Read<sizeof(uint8_t)>(&sp);
                literal_array.emplace_back(v);
            }
            break;
        }
        case panda_file::LiteralTag::ARRAY_I16:
        case panda_file::LiteralTag::ARRAY_U16: {
            auto len = helpers::Read<sizeof(uint32_t)>(&sp);
            literal_array.emplace_back(len);
            for (size_t i = 0; i < len; i++) {
                auto v = helpers::Read<sizeof(uint16_t)>(&sp);
                literal_array.emplace_back(v);
            }
            break;
        }
        case panda_file::LiteralTag::INTEGER: {
            auto v = helpers::Read<sizeof(uint32_t)>(&sp);
            literal_array.emplace_back(v);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I32:
        case panda_file::LiteralTag::ARRAY_U32:
        case panda_file::LiteralTag::ARRAY_F32: {
            auto len = helpers::Read<sizeof(uint32_t)>(&sp);
            literal_array.emplace_back(len);
            for (size_t i = 0; i < len; i++) {
                auto v = helpers::Read<sizeof(uint32_t)>(&sp);
                literal_array.emplace_back(v);
            }
            break;
        }
        case panda_file::LiteralTag::ARRAY_I64:
        case panda_file::LiteralTag::ARRAY_U64:
        case panda_file::LiteralTag::ARRAY_F64: {
            auto len = helpers::Read<sizeof(uint32_t)>(&sp);
            literal_array.emplace_back(len);
            for (size_t i = 0; i < len; i++) {
                auto v = helpers::Read<sizeof(uint64_t)>(&sp);
                literal_array.emplace_back(v);
            }
            break;
        }
        case panda_file::LiteralTag::FLOAT: {
            auto v = helpers::Read<sizeof(uint32_t)>(&sp);
            literal_array.emplace_back(v);
            break;
        }
        case panda_file::LiteralTag::DOUBLE: {
            auto v = panda_file::helpers::Read<sizeof(uint64_t)>(&sp);
            literal_array.emplace_back(v);
            break;
        }
        case panda_file::LiteralTag::STRING: {
            File::EntityId str_id(helpers::Read<sizeof(uint32_t)>(&sp));
            auto data = file_->GetStringData(str_id);
            std::string item_str(utf::Mutf8AsCString(data.data));
            auto *string_item = container_.GetOrCreateStringItem(item_str);
            literal_array.emplace_back(string_item);
            break;
        }
        case panda_file::LiteralTag::ARRAY_STRING: {
            auto len = helpers::Read<sizeof(uint32_t)>(&sp);
            literal_array.emplace_back(len);
            for (size_t i = 0; i < len; i++) {
                File::EntityId str_id(helpers::Read<sizeof(uint32_t)>(&sp));
                auto data = file_->GetStringData(str_id);
                std::string item_str(utf::Mutf8AsCString(data.data));
                auto *string_item = container_.GetOrCreateStringItem(item_str);
                literal_array.emplace_back(string_item);
            }
            break;
        }
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GETTER:
        case panda_file::LiteralTag::SETTER:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD: {
            File::EntityId method_id(helpers::Read<sizeof(uint32_t)>(&sp));
            MethodDataAccessor method_acc(*file_, method_id);
            File::EntityId class_id(method_acc.GetClassId());
            auto *class_item = CreateClassItem(class_id);
            literal_array.emplace_back(CreateMethodItem(class_item, method_id));
            break;
        }
        default:
            UNREACHABLE();
    }

    item->AddItems(literal_array);

    return true;
}

// NOLINTNEXTLINE(readability-function-size)
AnnotationItem *FileReader::CreateAnnotationItem(File::EntityId ann_id)
{
    auto it = items_done_.find(ann_id);
    if (it != items_done_.end()) {
        return static_cast<AnnotationItem *>(it->second);
    }

    AnnotationDataAccessor ann_acc(*file_, ann_id);
    File::EntityId ann_class_id {ann_acc.GetClassId()};
    AnnotationItem *ann_item = nullptr;

    if (!file_->IsExternal(ann_class_id)) {
        auto *ann_class_item = CreateClassItem(ann_class_id);
        ann_item = container_.CreateItem<AnnotationItem>(ann_class_item, std::vector<AnnotationItem::Elem>(),
                                                         std::vector<AnnotationItem::Tag>());
    } else {
        auto *ann_class_item = CreateForeignClassItem(ann_class_id);
        ann_item = container_.CreateItem<AnnotationItem>(ann_class_item, std::vector<AnnotationItem::Elem>(),
                                                         std::vector<AnnotationItem::Tag>());
    }

    ASSERT(ann_item != nullptr);

    items_done_.insert({ann_id, static_cast<BaseItem *>(ann_item)});

    std::vector<AnnotationItem::Elem> item_elements;
    std::vector<AnnotationItem::Tag> tag_elements;

    for (size_t i = 0; i < ann_acc.GetCount(); i++) {
        AnnotationDataAccessor::Tag ann_tag = ann_acc.GetTag(i);
        AnnotationDataAccessor::Elem ann_elem = ann_acc.GetElement(i);
        ValueItem *elem_value_item = nullptr;
        switch (ann_tag.GetItem()) {
            case '1':
            case '2':
            case '3': {
                auto scalar = ann_elem.GetScalarValue();
                elem_value_item = container_.GetOrCreateIntegerValueItem(scalar.Get<uint8_t>());
                break;
            }
            case '4':
            case '5': {
                auto scalar = ann_elem.GetScalarValue();
                elem_value_item = container_.GetOrCreateIntegerValueItem(scalar.Get<uint16_t>());
                break;
            }
            case '6':
            case '7': {
                auto scalar = ann_elem.GetScalarValue();
                elem_value_item = container_.GetOrCreateIntegerValueItem(scalar.Get<uint32_t>());
                break;
            }
            case '8':
            case '9': {
                auto scalar = ann_elem.GetScalarValue();
                elem_value_item = container_.GetOrCreateLongValueItem(scalar.Get<uint64_t>());
                break;
            }
            case 'A': {
                auto scalar = ann_elem.GetScalarValue();
                elem_value_item = container_.GetOrCreateFloatValueItem(scalar.Get<float>());
                break;
            }
            case 'B': {
                auto scalar = ann_elem.GetScalarValue();
                elem_value_item = container_.GetOrCreateDoubleValueItem(scalar.Get<double>());
                break;
            }
            case 'C': {
                auto scalar = ann_elem.GetScalarValue();
                const File::EntityId str_id(scalar.Get<uint32_t>());
                auto data = file_->GetStringData(str_id);
                std::string item_str(utf::Mutf8AsCString(data.data));
                auto *str_item = container_.GetOrCreateStringItem(item_str);
                elem_value_item = container_.GetOrCreateIdValueItem(str_item);
                break;
            }
            case 'D': {
                auto scalar = ann_elem.GetScalarValue();
                const File::EntityId class_id {scalar.Get<uint32_t>()};
                elem_value_item = container_.GetOrCreateIdValueItem(CreateGenericClassItem(class_id));
                break;
            }
            case 'E': {
                auto scalar = ann_elem.GetScalarValue();
                const File::EntityId method_id {scalar.Get<uint32_t>()};
                MethodDataAccessor method_acc(*file_, method_id);
                auto *cls_item = CreateGenericClassItem(method_acc.GetClassId());
                elem_value_item = container_.GetOrCreateIdValueItem(CreateGenericMethodItem(cls_item, method_id));
                break;
            }
            case 'F': {
                auto scalar = ann_elem.GetScalarValue();
                const File::EntityId field_id {scalar.Get<uint32_t>()};
                FieldDataAccessor field_acc(*file_, field_id);
                auto *cls_item = CreateGenericClassItem(field_acc.GetClassId());
                elem_value_item = container_.GetOrCreateIdValueItem(CreateGenericFieldItem(cls_item, field_id));
                break;
            }
            case 'G': {
                auto scalar = ann_elem.GetScalarValue();
                const File::EntityId ann_item_id {scalar.Get<uint32_t>()};
                elem_value_item = container_.GetOrCreateIdValueItem(CreateAnnotationItem(ann_item_id));
                break;
            }
            case 'J': {
                LOG(FATAL, PANDAFILE) << "MethodHandle is not supported so far";
                break;
            }
            case '*': {
                elem_value_item = container_.GetOrCreateIntegerValueItem(0);
                break;
            }
            case 'K': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    ScalarValueItem scalar(static_cast<uint32_t>(array.Get<uint8_t>(j)));
                    items.emplace_back(std::move(scalar));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::U1), std::move(items)));
                break;
            }
            case 'L': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    ScalarValueItem scalar(static_cast<uint32_t>(array.Get<uint8_t>(j)));
                    items.emplace_back(std::move(scalar));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::I8), std::move(items)));
                break;
            }
            case 'M': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    ScalarValueItem scalar(static_cast<uint32_t>(array.Get<uint8_t>(j)));
                    items.emplace_back(std::move(scalar));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::U8), std::move(items)));
                break;
            }
            case 'N': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    ScalarValueItem scalar(static_cast<uint32_t>(array.Get<uint16_t>(j)));
                    items.emplace_back(std::move(scalar));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::I16), std::move(items)));
                break;
            }
            case 'O': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    ScalarValueItem scalar(static_cast<uint32_t>(array.Get<uint16_t>(j)));
                    items.emplace_back(std::move(scalar));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::U16), std::move(items)));
                break;
            }
            case 'P': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    ScalarValueItem scalar(array.Get<uint32_t>(j));
                    items.emplace_back(std::move(scalar));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::I32), std::move(items)));
                break;
            }
            case 'Q': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    ScalarValueItem scalar(array.Get<uint32_t>(j));
                    items.emplace_back(std::move(scalar));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::U32), std::move(items)));
                break;
            }
            case 'R': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    ScalarValueItem scalar(array.Get<uint64_t>(j));
                    items.emplace_back(std::move(scalar));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::I64), std::move(items)));
                break;
            }
            case 'S': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    ScalarValueItem scalar(array.Get<uint64_t>(j));
                    items.emplace_back(std::move(scalar));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::U64), std::move(items)));
                break;
            }
            case 'T': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    ScalarValueItem scalar(array.Get<float>(j));
                    items.emplace_back(std::move(scalar));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::F32), std::move(items)));
                break;
            }
            case 'U': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    ScalarValueItem scalar(array.Get<double>(j));
                    items.emplace_back(std::move(scalar));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::F64), std::move(items)));
                break;
            }
            case 'V': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    const File::EntityId str_id(array.Get<uint32_t>(j));
                    auto data = file_->GetStringData(str_id);
                    std::string item_str(utf::Mutf8AsCString(data.data));
                    items.emplace_back(ScalarValueItem(container_.GetOrCreateStringItem(item_str)));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::REFERENCE), std::move(items)));
                break;
            }
            case 'W': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    const File::EntityId class_id {array.Get<uint32_t>(j)};
                    BaseClassItem *cls_item = nullptr;
                    if (file_->IsExternal(class_id)) {
                        cls_item = CreateForeignClassItem(class_id);
                    } else {
                        cls_item = CreateClassItem(class_id);
                    }
                    ASSERT(cls_item != nullptr);
                    items.emplace_back(ScalarValueItem(cls_item));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::REFERENCE), std::move(items)));
                break;
            }
            case 'X': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    const File::EntityId method_id {array.Get<uint32_t>(j)};
                    MethodDataAccessor method_acc(*file_, method_id);
                    auto *cls_item = CreateGenericClassItem(method_acc.GetClassId());
                    items.emplace_back(ScalarValueItem(CreateGenericMethodItem(cls_item, method_id)));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::REFERENCE), std::move(items)));
                break;
            }
            case 'Y': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    const File::EntityId field_id {array.Get<uint32_t>(j)};
                    FieldDataAccessor field_acc(*file_, field_id);
                    auto *cls_item = CreateGenericClassItem(field_acc.GetClassId());
                    items.emplace_back(ScalarValueItem(CreateGenericFieldItem(cls_item, field_id)));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::REFERENCE), std::move(items)));
                break;
            }
            case 'H': {
                // ARRAY can appear for empty arrays only
                ASSERT(ann_elem.GetArrayValue().GetCount() == 0);
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::VOID), std::vector<ScalarValueItem>()));
                break;
            }
            case 'Z': {
                auto array = ann_elem.GetArrayValue();
                std::vector<ScalarValueItem> items;
                for (size_t j = 0; j < array.GetCount(); j++) {
                    const File::EntityId ann_item_id {array.Get<uint32_t>(j)};
                    items.emplace_back(CreateAnnotationItem(ann_item_id));
                }
                elem_value_item = static_cast<ValueItem *>(
                    container_.CreateItem<ArrayValueItem>(Type(Type::TypeId::REFERENCE), std::move(items)));
                break;
            }
            case '@': {
                // TODO(nsizov): support it
                LOG(FATAL, PANDAFILE) << "MethodHandle is not supported so far";
                break;
            }
                // array
            case 'I':
                // VOID(I) and ARRAY(H) value should not appear
            default:
                UNREACHABLE();
        }

        ASSERT(elem_value_item != nullptr);

        tag_elements.emplace_back(AnnotationItem::Tag(static_cast<char>(ann_tag.GetItem())));
        File::EntityId name_id(ann_elem.GetNameId());
        std::string annot_name_str(utf::Mutf8AsCString(file_->GetStringData(name_id).data));
        auto elem_name_item = container_.GetOrCreateStringItem(annot_name_str);
        item_elements.emplace_back(AnnotationItem::Elem(elem_name_item, elem_value_item));
    }

    ann_item->SetElements(std::move(item_elements));
    ann_item->SetTags(std::move(tag_elements));

    return ann_item;
}

TypeItem *FileReader::CreateParamTypeItem(ProtoDataAccessor *proto_acc, size_t param_num, size_t reference_num)
{
    Type param_type = proto_acc->GetArgType(param_num);
    TypeItem *param_type_item = nullptr;
    if (param_type.IsPrimitive()) {
        param_type_item = container_.GetOrCreatePrimitiveTypeItem(param_type);
    } else {
        const File::EntityId type_cls_id = proto_acc->GetReferenceType(reference_num);
        if (file_->IsExternal(type_cls_id)) {
            param_type_item = CreateForeignClassItem(type_cls_id);
        } else {
            param_type_item = CreateClassItem(type_cls_id);
        }
    }

    ASSERT(param_type_item != nullptr);

    return param_type_item;
}

std::vector<MethodParamItem> FileReader::CreateMethodParamItems(ProtoDataAccessor *proto_acc,
                                                                MethodDataAccessor *method_acc, size_t reference_num)
{
    std::vector<MethodParamItem> param_items;

    for (size_t i = 0; i < proto_acc->GetNumArgs(); i++) {
        TypeItem *param_type_item = CreateParamTypeItem(proto_acc, i, reference_num);
        if (param_type_item->GetType().IsReference()) {
            reference_num++;
        }
        param_items.emplace_back(MethodParamItem(param_type_item));
    }

    auto param_ann_id = method_acc->GetParamAnnotationId();
    if (param_ann_id) {
        ParamAnnotationsDataAccessor param_acc(*file_, param_ann_id.value());
        for (size_t i = 0; i < proto_acc->GetNumArgs(); i++) {
            ParamAnnotationsDataAccessor::AnnotationArray ann_arr = param_acc.GetAnnotationArray(i);
            ann_arr.EnumerateAnnotations([&](File::EntityId ann_id) {
                auto ann_item = CreateAnnotationItem(ann_id);
                param_items[i].AddAnnotation(ann_item);
            });
        }
    }

    auto runtime_param_ann_id = method_acc->GetRuntimeParamAnnotationId();
    if (runtime_param_ann_id) {
        ParamAnnotationsDataAccessor param_acc(*file_, runtime_param_ann_id.value());
        for (size_t i = 0; i < proto_acc->GetNumArgs(); i++) {
            ParamAnnotationsDataAccessor::AnnotationArray ann_arr = param_acc.GetAnnotationArray(i);
            ann_arr.EnumerateAnnotations([&](File::EntityId ann_id) {
                auto ann_item = CreateAnnotationItem(ann_id);
                param_items[i].AddRuntimeAnnotation(ann_item);
            });
        }
    }

    return param_items;
}

DebugInfoItem *FileReader::CreateDebugInfoItem(File::EntityId debug_info_id)
{
    auto it = items_done_.find(debug_info_id);
    if (it != items_done_.end()) {
        return static_cast<DebugInfoItem *>(it->second);
    }

    auto *lnp_item = container_.CreateLineNumberProgramItem();
    auto *debug_info_item = container_.CreateItem<DebugInfoItem>(lnp_item);
    items_done_.insert({debug_info_id, static_cast<BaseItem *>(debug_info_item)});

    DebugInfoDataAccessor debug_acc(*file_, debug_info_id);

    debug_info_item->SetLineNumber(debug_acc.GetLineStart());
    debug_acc.EnumerateParameters([&](File::EntityId param_id) {
        auto data = file_->GetStringData(param_id);
        std::string item_str(utf::Mutf8AsCString(data.data));
        auto *string_item = container_.GetOrCreateStringItem(item_str);
        debug_info_item->AddParameter(string_item);
    });

    return debug_info_item;
}

TypeItem *FileReader::GetReturnTypeItem(Type ret_type, size_t &reference_num)
{
    TypeItem *ret_type_item = nullptr;
    if (ret_type.IsPrimitive()) {
        ret_type_item = container_.GetOrCreatePrimitiveTypeItem(ret_type);
    } else {
        const File::EntityId type_cls_id = proto_acc.GetReferenceType(reference_num);
        if (file_->IsExternal(type_cls_id)) {
            ret_type_item = CreateForeignClassItem(type_cls_id);
        } else {
            ret_type_item = CreateClassItem(type_cls_id);
        }
        reference_num++;
    }
    return ret_type_item;
}

void FileReader::EnumerateBlocks(MethodDataAccessor method_acc, MethodItem *method_item)
{
    // quick check
    auto code_id = method_acc.GetCodeId();
    if (code_id == std::nullopt) {
        return;
    }

    CodeDataAccessor code_acc(*file_, code_id.value());
    std::vector<uint8_t> instructions(code_acc.GetCodeSize());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    instructions.assign(code_acc.GetInstructions(), code_acc.GetInstructions() + code_acc.GetCodeSize());
    auto *code_item =
        container_.CreateItem<CodeItem>(code_acc.GetNumVregs(), code_acc.GetNumArgs(), std::move(instructions));

    code_acc.EnumerateTryBlocks([&](CodeDataAccessor::TryBlock &try_block) {
        std::vector<CodeItem::CatchBlock> catch_blocks;
        try_block.EnumerateCatchBlocks([&](CodeDataAccessor::CatchBlock &catch_block) {
            BaseClassItem *catch_type_item = nullptr;
            auto type_idx = catch_block.GetTypeIdx();
            if (type_idx != panda_file::INVALID_INDEX) {
                File::EntityId catch_cls_id = file_->ResolveClassIndex(method_id, catch_block.GetTypeIdx());
                catch_type_item = (file_->IsExternal(catch_cls_id)) ? CreateForeignClassItem(catch_cls_id) :
                                                                      CreateClassItem(catch_cls_id);
                method_item->AddIndexDependency(catch_type_item);
            }
            // Add new CatchBlock
            catch_blocks.emplace_back(CodeItem::CatchBlock(method_item, catch_type_item, catch_block.GetHandlerPc(),
                                                           catch_block.GetCodeSize()));
            return true;
        });
        code_item->AddTryBlock(
            CodeItem::TryBlock(try_block.GetStartPc(), try_block.GetLength(), std::move(catch_blocks)));
        return true;
    });

    method_item->SetCode(code_item);
}

MethodItem *FileReader::CreateMethodItem(ClassItem *cls, File::EntityId method_id)
{
    // Check if we have done this method
    auto it = items_done_.find(method_id);
    if (it != items_done_.end()) {
        return static_cast<MethodItem *>(it->second);
    }

    MethodDataAccessor method_acc(*file_, method_id);
    auto data = file_->GetStringData(method_acc.GetNameId());
    std::string method_name(utf::Mutf8AsCString(data.data));
    auto method_str_item = container_.GetOrCreateStringItem(method_name);

    ProtoDataAccessor proto_acc(*file_, method_acc.GetProtoId());
    Type ret_type = proto_acc.GetReturnType();
    size_t reference_num = 0;
    TypeItem *ret_type_item = GetReturnTypeItem(ret_type, reference_num);
    ASSERT(ret_type_item != nullptr);
    auto param_items = CreateMethodParamItems(&proto_acc, &method_acc, reference_num);

    // Double check if we have done this method while computing params
    auto it_check = items_done_.find(method_id);
    if (it_check != items_done_.end()) {
        return static_cast<MethodItem *>(it_check->second);
    }

    auto proto_item = container_.GetOrCreateProtoItem(ret_type_item, param_items);
    auto method_item =
        cls->AddMethod(method_str_item, proto_item, method_acc.GetAccessFlags(), std::move(param_items));
    if (method_item->HasRuntimeParamAnnotations()) {
        container_.CreateItem<ParamAnnotationsItem>(method_item, true);
    }

    if (method_item->HasParamAnnotations()) {
        container_.CreateItem<ParamAnnotationsItem>(method_item, false);
    }
    items_done_.insert({method_id, static_cast<BaseItem *>(method_item)});

    method_acc.EnumerateAnnotations(
        [&](File::EntityId ann_id) { method_item->AddAnnotation(CreateAnnotationItem(ann_id)); });

    method_acc.EnumerateRuntimeAnnotations(
        [&](File::EntityId ann_id) { method_item->AddRuntimeAnnotation(CreateAnnotationItem(ann_id)); });

    method_acc.EnumerateTypeAnnotations(
        [&](File::EntityId ann_id) { method_item->AddTypeAnnotation(CreateAnnotationItem(ann_id)); });

    method_acc.EnumerateRuntimeTypeAnnotations(
        [&](File::EntityId ann_id) { method_item->AddRuntimeTypeAnnotation(CreateAnnotationItem(ann_id)); });

    EnumerateBlocks(method_acc, method_item);

    auto debug_info_id = method_acc.GetDebugInfoId();
    if (debug_info_id) {
        method_item->SetDebugInfo(CreateDebugInfoItem(debug_info_id.value()));
    }

    auto source_lang = method_acc.GetSourceLang();
    if (source_lang) {
        method_item->SetSourceLang(source_lang.value());
    }

    return method_item;
}

MethodHandleItem *FileReader::CreateMethodHandleItem(File::EntityId mh_id)
{
    (void)mh_id;
    ASSERT(false);
    return nullptr;  // STUB
}

FieldItem *FileReader::CreateFieldItem(ClassItem *cls, File::EntityId field_id)
{
    auto it = items_done_.find(field_id);
    if (it != items_done_.end()) {
        return static_cast<FieldItem *>(it->second);
    }

    FieldDataAccessor field_acc(*file_, field_id);

    auto data = file_->GetStringData(field_acc.GetNameId());
    std::string string_name(utf::Mutf8AsCString(data.data));
    auto *field_name = container_.GetOrCreateStringItem(string_name);
    Type field_type = Type::GetTypeFromFieldEncoding(field_acc.GetType());

    TypeItem *field_type_item = nullptr;
    if (field_type.IsReference()) {
        File::EntityId type_id(field_acc.GetType());
        if (file_->IsExternal(type_id)) {
            field_type_item = CreateForeignClassItem(type_id);
        } else {
            field_type_item = CreateClassItem(type_id);
            // Double check if we done this field while generated class item
            auto it_check = items_done_.find(field_id);
            if (it_check != items_done_.end()) {
                return static_cast<FieldItem *>(it_check->second);
            }
        }
    } else {
        field_type_item = container_.GetOrCreatePrimitiveTypeItem(field_type.GetId());
    }

    ASSERT(field_type_item != nullptr);

    FieldItem *field_item = cls->AddField(field_name, field_type_item, field_acc.GetAccessFlags());
    items_done_.insert({field_id, static_cast<BaseItem *>(field_item)});

    switch (field_type.GetId()) {
        case Type::TypeId::U1:
        case Type::TypeId::I8:
        case Type::TypeId::U8:
            SetIntegerFieldValue<uint8_t>(&field_acc, field_item);
            break;
        case Type::TypeId::I16:
        case Type::TypeId::U16:
            SetIntegerFieldValue<uint16_t>(&field_acc, field_item);
            break;
        case Type::TypeId::I32:
        case Type::TypeId::U32:
            SetIntegerFieldValue<uint32_t>(&field_acc, field_item);
            break;
        case Type::TypeId::I64:
        case Type::TypeId::U64:
            SetIntegerFieldValue<uint64_t>(&field_acc, field_item);
            break;
        case Type::TypeId::F32:
            SetFloatFieldValue<float>(&field_acc, field_item);
            break;
        case Type::TypeId::F64:
            SetFloatFieldValue<double>(&field_acc, field_item);
            break;
        case Type::TypeId::REFERENCE:
            SetStringFieldValue(&field_acc, field_item);
            break;
        case Type::TypeId::TAGGED:
        default:
            UNREACHABLE();
            break;
    }

    field_acc.EnumerateAnnotations(
        [&](File::EntityId ann_id) { field_item->AddAnnotation(CreateAnnotationItem(ann_id)); });

    field_acc.EnumerateRuntimeAnnotations(
        [&](File::EntityId ann_id) { field_item->AddRuntimeAnnotation(CreateAnnotationItem(ann_id)); });

    field_acc.EnumerateRuntimeTypeAnnotations(
        [&](File::EntityId ann_id) { field_item->AddRuntimeTypeAnnotation(CreateAnnotationItem(ann_id)); });

    field_acc.EnumerateTypeAnnotations(
        [&](File::EntityId ann_id) { field_item->AddTypeAnnotation(CreateAnnotationItem(ann_id)); });

    return field_item;
}

ForeignMethodItem *FileReader::CreateForeignMethodItem(BaseClassItem *fcls, File::EntityId method_id)
{
    auto it = items_done_.find(method_id);
    if (it != items_done_.end()) {
        return static_cast<ForeignMethodItem *>(it->second);
    }

    MethodDataAccessor method_acc(*file_, method_id);
    auto data = file_->GetStringData(method_acc.GetNameId());
    std::string method_name(utf::Mutf8AsCString(data.data));
    auto *method_str_item = container_.GetOrCreateStringItem(method_name);

    ProtoDataAccessor proto_acc(*file_, method_acc.GetProtoId());
    Type ret_type = proto_acc.GetReturnType();
    size_t reference_num = 0;
    TypeItem *ret_type_item = nullptr;
    if (ret_type.IsPrimitive()) {
        ret_type_item = container_.GetOrCreatePrimitiveTypeItem(ret_type);
    } else {
        const File::EntityId type_cls_id = proto_acc.GetReferenceType(reference_num);
        if (file_->IsExternal(type_cls_id)) {
            ret_type_item = CreateForeignClassItem(type_cls_id);
        } else {
            ret_type_item = CreateClassItem(type_cls_id);
        }
        reference_num++;
    }
    ASSERT(ret_type_item != nullptr);
    auto param_items = CreateMethodParamItems(&proto_acc, &method_acc, reference_num);
    // Double check if we done this method while computing params
    auto it_check = items_done_.find(method_id);
    if (it_check != items_done_.end()) {
        return static_cast<ForeignMethodItem *>(it_check->second);
    }
    auto *proto_item = container_.GetOrCreateProtoItem(ret_type_item, param_items);

    auto *method_item =
        container_.CreateItem<ForeignMethodItem>(fcls, method_str_item, proto_item, method_acc.GetAccessFlags());

    items_done_.insert({method_id, static_cast<BaseItem *>(method_item)});

    return method_item;
}

ForeignFieldItem *FileReader::CreateForeignFieldItem(BaseClassItem *fcls, File::EntityId field_id)
{
    auto it = items_done_.find(field_id);
    if (it != items_done_.end()) {
        return static_cast<ForeignFieldItem *>(it->second);
    }

    FieldDataAccessor field_acc(*file_, field_id);

    auto data = file_->GetStringData(field_acc.GetNameId());
    std::string string_name(utf::Mutf8AsCString(data.data));
    auto *field_name = container_.GetOrCreateStringItem(string_name);
    Type field_type = Type::GetTypeFromFieldEncoding(field_acc.GetType());
    TypeItem *field_type_item = nullptr;
    if (field_type.IsReference()) {
        File::EntityId type_id(field_acc.GetType());
        if (file_->IsExternal(type_id)) {
            field_type_item = CreateForeignClassItem(type_id);
        } else {
            field_type_item = CreateClassItem(type_id);
            // Double check if we done this field while generated class item
            auto it_check = items_done_.find(field_id);
            if (it_check != items_done_.end()) {
                return static_cast<ForeignFieldItem *>(it_check->second);
            }
        }
    } else {
        field_type_item = container_.GetOrCreatePrimitiveTypeItem(field_type.GetId());
    }

    ASSERT(field_type_item != nullptr);

    auto *field_item = container_.CreateItem<ForeignFieldItem>(fcls, field_name, field_type_item);
    items_done_.insert({field_id, static_cast<BaseItem *>(field_item)});

    return field_item;
}

ForeignClassItem *FileReader::CreateForeignClassItem(File::EntityId class_id)
{
    auto it = items_done_.find(class_id);
    if (it != items_done_.end()) {
        return static_cast<ForeignClassItem *>(it->second);
    }

    std::string class_name(utf::Mutf8AsCString(file_->GetStringData(class_id).data));
    auto *class_item = container_.GetOrCreateForeignClassItem(class_name);

    items_done_.insert({class_id, static_cast<BaseItem *>(class_item)});

    return class_item;
}

void FileReader::CreateSuperClassItem(ClassDataAccessor& class_acc,
                                      ClassItem* class_item,
                                      const std::string& class_name)
{
    auto super_class_id = class_acc.GetSuperClassId();
    if (super_class_id.GetOffset() != 0) {
        if (super_class_id.GetOffset() == class_id.GetOffset()) {
            LOG(FATAL, PANDAFILE) << "Class " << class_name << " has cyclic inheritance";
        }

        if (file_->IsExternal(super_class_id)) {
            auto *super_class_item = CreateForeignClassItem(super_class_id);
            class_item->SetSuperClass(super_class_item);
        } else {
            auto *super_class_item = CreateClassItem(super_class_id);
            class_item->SetSuperClass(super_class_item);
        }
    }
}

ClassItem *FileReader::CreateClassItem(File::EntityId class_id)
{
    auto it = items_done_.find(class_id);
    if (it != items_done_.end()) {
        return static_cast<ClassItem *>(it->second);
    }
    ClassDataAccessor class_acc(*file_, class_id);

    std::string class_name(utf::Mutf8AsCString(file_->GetStringData(class_id).data));
    auto *class_item = container_.GetOrCreateClassItem(class_name);

    items_done_.insert({class_id, static_cast<BaseItem *>(class_item)});

    class_item->SetAccessFlags(class_acc.GetAccessFlags());

    auto source_lang_opt = class_acc.GetSourceLang();
    if (source_lang_opt) {
        class_item->SetSourceLang(source_lang_opt.value());
    }

    CreateSuperClassItem(class_acc, class_item, class_name);

    class_acc.EnumerateInterfaces([&](File::EntityId iface_id) {
        if (file_->IsExternal(iface_id)) {
            class_item->AddInterface(CreateForeignClassItem(iface_id));
        } else {
            class_item->AddInterface(CreateClassItem(iface_id));
        }
    });

    class_acc.EnumerateAnnotations(
        [&](File::EntityId ann_id) { class_item->AddAnnotation(CreateAnnotationItem(ann_id)); });

    class_acc.EnumerateRuntimeAnnotations(
        [&](File::EntityId ann_id) { class_item->AddRuntimeAnnotation(CreateAnnotationItem(ann_id)); });

    class_acc.EnumerateTypeAnnotations(
        [&](File::EntityId ann_id) { class_item->AddTypeAnnotation(CreateAnnotationItem(ann_id)); });

    class_acc.EnumerateFields(
        [&](FieldDataAccessor &field_acc) { CreateFieldItem(class_item, field_acc.GetFieldId()); });

    class_acc.EnumerateMethods(
        [&](MethodDataAccessor &method_acc) { CreateMethodItem(class_item, method_acc.GetMethodId()); });

    auto source_file_id = class_acc.GetSourceFileId();
    if (source_file_id) {
        std::string source_file = utf::Mutf8AsCString(file_->GetStringData(source_file_id.value()).data);
        class_item->SetSourceFile(container_.GetOrCreateStringItem(source_file));
    }

    ASSERT(class_item != nullptr);

    return class_item;
}

bool FileReader::ReadLiteralArrayItems()
{
    const auto lit_arrays_id = file_->GetLiteralArraysId();
    LiteralDataAccessor lit_array_accessor(*file_, lit_arrays_id);
    size_t num_litarrays = lit_array_accessor.GetLiteralNum();

    for (size_t i = 0; i < num_litarrays; i++) {
        auto id = lit_array_accessor.GetLiteralArrayId(i);
        lit_array_accessor.EnumerateLiteralVals(
            id, [id, this](const panda_file::LiteralDataAccessor::LiteralValue &value,
                           const panda_file::LiteralTag &tag) { CreateLiteralArrayItem(value, tag, id); });
    }

    return true;
}

bool FileReader::ReadIndexHeaders()
{
    auto index_headers = file_->GetIndexHeaders();
    for (const auto &header : index_headers) {
        auto method_index = file_->GetMethodIndex(&header);
        for (auto method_id : method_index) {
            MethodDataAccessor method_acc(*file_, method_id);
            File::EntityId class_id(method_acc.GetClassId());
            if (file_->IsExternal(class_id)) {
                auto *fclass_item = CreateForeignClassItem(class_id);
                ASSERT(file_->IsExternal(method_id));
                if (CreateForeignMethodItem(fclass_item, method_id) == nullptr) {
                    return false;
                }
            } else {
                auto *class_item = CreateClassItem(class_id);
                if (file_->IsExternal(method_id)) {
                    if (CreateForeignMethodItem(class_item, method_id) == nullptr) {
                        return false;
                    }
                } else if (CreateMethodItem(class_item, method_id) == nullptr) {
                    return false;
                }
            }
        }
        auto field_index = file_->GetFieldIndex(&header);
        for (auto field_id : field_index) {
            FieldDataAccessor field_acc(*file_, field_id);
            File::EntityId class_id(field_acc.GetClassId());
            if (file_->IsExternal(class_id)) {
                ASSERT(file_->IsExternal(field_id));
                auto *fclass_item = CreateForeignClassItem(field_acc.GetClassId());
                if (CreateForeignFieldItem(fclass_item, field_id) == nullptr) {
                    return false;
                }
            } else {
                auto *class_item = CreateClassItem(field_acc.GetClassId());
                if (file_->IsExternal(field_id)) {
                    if (CreateForeignFieldItem(class_item, field_id) == nullptr) {
                        return false;
                    }
                } else if (CreateFieldItem(class_item, field_id) == nullptr) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool FileReader::ReadClasses()
{
    const auto class_idx = file_->GetClasses();

    for (unsigned int id : class_idx) {
        File::EntityId eid(id);
        if (file_->IsExternal(eid)) {
            CreateForeignClassItem(eid);
        } else {
            CreateClassItem(eid);
        }
    }

    return true;
}

void FileReader::UpdateDebugInfoDependecies(File::EntityId debug_info_id)
{
    DebugInfoDataAccessor debug_acc(*file_, debug_info_id);
    const uint8_t *program = debug_acc.GetLineNumberProgram();
    auto size = file_->GetSpanFromId(file_->GetIdFromPointer(program)).size();
    auto opcode_sp = Span(program, size);

    size_t i = 0;
    LineNumberProgramItem::Opcode opcode;
    panda_file::LineProgramState state(*file_, File::EntityId(0), debug_acc.GetLineStart(),
                                       debug_acc.GetConstantPool());
    while ((opcode = LineNumberProgramItem::Opcode(opcode_sp[i++])) != LineNumberProgramItem::Opcode::END_SEQUENCE) {
        switch (opcode) {
            case LineNumberProgramItem::Opcode::ADVANCE_PC:
            case LineNumberProgramItem::Opcode::ADVANCE_LINE:
            case LineNumberProgramItem::Opcode::SET_PROLOGUE_END:
            case LineNumberProgramItem::Opcode::SET_EPILOGUE_BEGIN: {
                break;
            }
            case LineNumberProgramItem::Opcode::START_LOCAL: {
                [[maybe_unused]] int32_t reg_number;
                size_t n;
                bool is_full;
                std::tie(reg_number, n, is_full) = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                i += n;

                auto name_id = File::EntityId(state.ReadULeb128());
                std::string name = utf::Mutf8AsCString(file_->GetStringData(name_id).data);
                container_.GetOrCreateStringItem(name);

                auto type_id = File::EntityId(state.ReadULeb128());
                std::string type_name = utf::Mutf8AsCString(file_->GetStringData(type_id).data);
                if (file_->IsExternal(type_id)) {
                    container_.GetOrCreateForeignClassItem(type_name);
                } else {
                    container_.GetOrCreateClassItem(type_name);
                }
                break;
            }
            case LineNumberProgramItem::Opcode::START_LOCAL_EXTENDED: {
                [[maybe_unused]] int32_t reg_number;
                size_t n;
                bool is_full;
                std::tie(reg_number, n, is_full) = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                i += n;

                auto name_id = File::EntityId(state.ReadULeb128());
                std::string name = utf::Mutf8AsCString(file_->GetStringData(name_id).data);
                container_.GetOrCreateStringItem(name);

                auto type_id = File::EntityId(state.ReadULeb128());
                std::string type_name = utf::Mutf8AsCString(file_->GetStringData(type_id).data);
                if (file_->IsExternal(type_id)) {
                    container_.GetOrCreateForeignClassItem(type_name);
                } else {
                    container_.GetOrCreateClassItem(type_name);
                }

                auto type_signature_id = File::EntityId(state.ReadULeb128());
                std::string type_signature = utf::Mutf8AsCString(file_->GetStringData(type_signature_id).data);
                container_.GetOrCreateStringItem(type_signature);
                break;
            }
            case LineNumberProgramItem::Opcode::END_LOCAL:
            case LineNumberProgramItem::Opcode::RESTART_LOCAL: {
                [[maybe_unused]] int32_t reg_number;
                size_t n;
                bool is_full;
                std::tie(reg_number, n, is_full) = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                i += n;
                break;
            }
            case LineNumberProgramItem::Opcode::SET_FILE: {
                auto source_file_id = File::EntityId(state.ReadULeb128());
                std::string source_file = utf::Mutf8AsCString(file_->GetStringData(source_file_id).data);
                container_.GetOrCreateStringItem(source_file);
                break;
            }
            case LineNumberProgramItem::Opcode::SET_SOURCE_CODE: {
                auto source_code_id = File::EntityId(state.ReadULeb128());
                std::string source_code = utf::Mutf8AsCString(file_->GetStringData(source_code_id).data);
                container_.GetOrCreateStringItem(source_code);
                break;
            }
            default: {
                break;
            }
        }
    }
}

void FileReader::UpdateDebugInfo(DebugInfoItem *debug_info_item, File::EntityId debug_info_id)
{
    auto *lnp_item = debug_info_item->GetLineNumberProgram();
    DebugInfoDataAccessor debug_acc(*file_, debug_info_id);
    const uint8_t *program = debug_acc.GetLineNumberProgram();
    auto size = file_->GetSpanFromId(file_->GetIdFromPointer(program)).size();
    auto opcode_sp = Span(program, size);

    size_t i = 0;
    LineNumberProgramItem::Opcode opcode;
    panda_file::LineProgramState state(*file_, File::EntityId(0), debug_acc.GetLineStart(),
                                       debug_acc.GetConstantPool());
    while ((opcode = LineNumberProgramItem::Opcode(opcode_sp[i++])) != LineNumberProgramItem::Opcode::END_SEQUENCE) {
        switch (opcode) {
            case LineNumberProgramItem::Opcode::ADVANCE_PC: {
                lnp_item->EmitAdvancePc(debug_info_item->GetConstantPool(), state.ReadULeb128());
                break;
            }
            case LineNumberProgramItem::Opcode::ADVANCE_LINE: {
                lnp_item->EmitAdvanceLine(debug_info_item->GetConstantPool(), state.ReadSLeb128());
                break;
            }
            case LineNumberProgramItem::Opcode::START_LOCAL: {
                auto [reg_number, n, is_full] = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                i += n;

                auto name_id = File::EntityId(state.ReadULeb128());
                std::string name = utf::Mutf8AsCString(file_->GetStringData(name_id).data);
                auto *name_item = container_.GetOrCreateStringItem(name);

                auto type_id = File::EntityId(state.ReadULeb128());
                std::string type_name = utf::Mutf8AsCString(file_->GetStringData(type_id).data);
                auto *type_item = file_->IsExternal(type_id)
                                      ? static_cast<BaseClassItem *>(container_.GetOrCreateForeignClassItem(type_name))
                                      : static_cast<BaseClassItem *>(container_.GetOrCreateClassItem(type_name));

                lnp_item->EmitStartLocal(debug_info_item->GetConstantPool(), reg_number, name_item,
                                         type_item->GetNameItem());
                break;
            }
            case LineNumberProgramItem::Opcode::START_LOCAL_EXTENDED: {
                auto [reg_number, n, is_full] = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                i += n;

                auto name_id = File::EntityId(state.ReadULeb128());
                std::string name = utf::Mutf8AsCString(file_->GetStringData(name_id).data);
                auto *name_item = container_.GetOrCreateStringItem(name);

                auto type_id = File::EntityId(state.ReadULeb128());
                std::string type_name = utf::Mutf8AsCString(file_->GetStringData(type_id).data);
                auto *type_item = file_->IsExternal(type_id)
                                      ? static_cast<BaseClassItem *>(container_.GetOrCreateForeignClassItem(type_name))
                                      : static_cast<BaseClassItem *>(container_.GetOrCreateClassItem(type_name));

                auto type_signature_id = File::EntityId(state.ReadULeb128());
                std::string type_signature = utf::Mutf8AsCString(file_->GetStringData(type_signature_id).data);
                auto *type_signature_item = container_.GetOrCreateStringItem(type_signature);

                lnp_item->EmitStartLocalExtended(debug_info_item->GetConstantPool(), reg_number, name_item,
                                                 type_item->GetNameItem(), type_signature_item);
                break;
            }
            case LineNumberProgramItem::Opcode::END_LOCAL: {
                auto [reg_number, n, is_full] = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                i += n;

                lnp_item->EmitEndLocal(reg_number);
                break;
            }
            case LineNumberProgramItem::Opcode::RESTART_LOCAL: {
                auto [reg_number, n, is_full] = leb128::DecodeSigned<int32_t>(&opcode_sp[i]);
                LOG_IF(!is_full, FATAL, COMMON) << "Cannot read a register number";
                i += n;

                lnp_item->EmitRestartLocal(reg_number);
                break;
            }
            case LineNumberProgramItem::Opcode::SET_PROLOGUE_END: {
                lnp_item->EmitPrologEnd();
                break;
            }
            case LineNumberProgramItem::Opcode::SET_EPILOGUE_BEGIN: {
                lnp_item->EmitEpilogBegin();
                break;
            }
            case LineNumberProgramItem::Opcode::SET_FILE: {
                auto source_file_id = File::EntityId(state.ReadULeb128());
                std::string source_file = utf::Mutf8AsCString(file_->GetStringData(source_file_id).data);
                auto *source_file_item = container_.GetOrCreateStringItem(source_file);
                lnp_item->EmitSetFile(debug_info_item->GetConstantPool(), source_file_item);
                break;
            }
            case LineNumberProgramItem::Opcode::SET_SOURCE_CODE: {
                auto source_code_id = File::EntityId(state.ReadULeb128());
                std::string source_code = utf::Mutf8AsCString(file_->GetStringData(source_code_id).data);
                auto *source_code_item = container_.GetOrCreateStringItem(source_code);
                lnp_item->EmitSetFile(debug_info_item->GetConstantPool(), source_code_item);
                break;
            }
            default: {
                auto opcode_value = static_cast<uint8_t>(opcode);
                auto adjust_opcode = opcode_value - LineNumberProgramItem::OPCODE_BASE;
                uint32_t pc_diff = adjust_opcode / LineNumberProgramItem::LINE_RANGE;
                int32_t line_diff =
                    adjust_opcode % LineNumberProgramItem::LINE_RANGE + LineNumberProgramItem::LINE_BASE;
                lnp_item->EmitSpecialOpcode(pc_diff, line_diff);
                break;
            }
        }
    }
    lnp_item->EmitEnd();
}

void AddIndexDependencyInstFlag(CodeItem *code_item, MethodItem *method_item,
                                const std::unordered_map<File::EntityId, File::EntityId> &reverse_done)
{
    using Flags = panda::BytecodeInst<panda::BytecodeInstMode::FAST>::Flags;

    size_t offset = 0;
    BytecodeInstruction inst(code_item->GetInstructions()->data());
    while (offset < code_item->GetCodeSize()) {
        if (inst.HasFlag(Flags::TYPE_ID)) {
            BytecodeId b_id = inst.GetId();
            File::Index idx = b_id.AsIndex();
            File::EntityId method_id = reverse_done.find(method_item)->second;
            File::EntityId old_id = file_->ResolveClassIndex(method_id, idx);
            ASSERT(items_done_.find(old_id) != items_done_.end());
            auto *idx_item = static_cast<IndexedItem *>(items_done_.find(old_id)->second);
            method_item->AddIndexDependency(idx_item);
        } else if (inst.HasFlag(Flags::METHOD_ID)) {
            BytecodeId b_id = inst.GetId();
            File::Index idx = b_id.AsIndex();
            File::EntityId method_id = reverse_done.find(method_item)->second;
            File::EntityId old_id = file_->ResolveMethodIndex(method_id, idx);
            ASSERT(items_done_.find(old_id) != items_done_.end());
            auto *idx_item = static_cast<IndexedItem *>(items_done_.find(old_id)->second);
            method_item->AddIndexDependency(idx_item);
        } else if (inst.HasFlag(Flags::FIELD_ID)) {
            BytecodeId b_id = inst.GetId();
            File::Index idx = b_id.AsIndex();
            File::EntityId method_id = reverse_done.find(method_item)->second;
            File::EntityId old_id = file_->ResolveFieldIndex(method_id, idx);
            ASSERT(items_done_.find(old_id) != items_done_.end());
            auto *idx_item = static_cast<IndexedItem *>(items_done_.find(old_id)->second);
            method_item->AddIndexDependency(idx_item);
        } else if (inst.HasFlag(Flags::STRING_ID)) {
            BytecodeId b_id = inst.GetId();
            File::EntityId old_id = b_id.AsFileId();
            auto data = file_->GetStringData(old_id);
            std::string item_str(utf::Mutf8AsCString(data.data));
            container_.GetOrCreateStringItem(item_str);
        }
        offset += inst.GetSize();
        inst = inst.GetNext();
    }
}

void FileReader::UpdateCodeAndDebugInfoDependencies(const std::map<BaseItem *, File::EntityId> &reverse_done)
{
    auto *class_map = container_.GetClassMap();

    // First pass, add dependencies bytecode -> new items
    for (const auto &it : *class_map) {
        auto *base_class_item = it.second;
        if (base_class_item->IsForeign()) {
            continue;
        }
        auto *class_item = static_cast<ClassItem *>(base_class_item);
        class_item->VisitMethods([this, &reverse_done](BaseItem *param_item) {
            auto *method_item = static_cast<MethodItem *>(param_item);
            auto *code_item = method_item->GetCode();
            if (code_item == nullptr) {
                return true;
            }

            auto *debug_info_item = method_item->GetDebugInfo();
            if (debug_info_item != nullptr) {
                UpdateDebugInfoDependecies(reverse_done.find(debug_info_item)->second);
            }

            AddIndexDependencyInstFlag(inst, method_item, reverse_done);

            return true;
        });
    }
}

void UpdateIdInstFlag(CodeItem *code_item, MethodItem *method_item,
                      const std::unordered_map<File::EntityId, File::EntityId> &reverse_done)
{
    using Flags = panda::BytecodeInst<panda::BytecodeInstMode::FAST>::Flags;

    size_t offset = 0;
    BytecodeInstruction inst(code_item->GetInstructions()->data());
    while (offset < code_item->GetCodeSize()) {
        if (inst.HasFlag(Flags::TYPE_ID)) {
            BytecodeId b_id = inst.GetId();
            File::Index idx = b_id.AsIndex();
            File::EntityId method_id = reverse_done.find(method_item)->second;
            File::EntityId old_id = file_->ResolveClassIndex(method_id, idx);
            ASSERT(items_done_.find(old_id) != items_done_.end());
            auto *idx_item = static_cast<IndexedItem *>(items_done_.find(old_id)->second);
            uint32_t index = idx_item->GetIndex(method_item);
            inst.UpdateId(BytecodeId(index));
        } else if (inst.HasFlag(Flags::METHOD_ID)) {
            BytecodeId b_id = inst.GetId();
            File::Index idx = b_id.AsIndex();
            File::EntityId method_id = reverse_done.find(method_item)->second;
            File::EntityId old_id = file_->ResolveMethodIndex(method_id, idx);
            ASSERT(items_done_.find(old_id) != items_done_.end());
            auto *idx_item = static_cast<IndexedItem *>(items_done_.find(old_id)->second);
            uint32_t index = idx_item->GetIndex(method_item);
            inst.UpdateId(BytecodeId(index));
        } else if (inst.HasFlag(Flags::FIELD_ID)) {
            BytecodeId b_id = inst.GetId();
            File::Index idx = b_id.AsIndex();
            File::EntityId method_id = reverse_done.find(method_item)->second;
            File::EntityId old_id = file_->ResolveFieldIndex(method_id, idx);
            ASSERT(items_done_.find(old_id) != items_done_.end());
            auto *idx_item = static_cast<IndexedItem *>(items_done_.find(old_id)->second);
            uint32_t index = idx_item->GetIndex(method_item);
            inst.UpdateId(BytecodeId(index));
        } else if (inst.HasFlag(Flags::STRING_ID)) {
            BytecodeId b_id = inst.GetId();
            File::EntityId old_id = b_id.AsFileId();
            auto data = file_->GetStringData(old_id);
            std::string item_str(utf::Mutf8AsCString(data.data));
            auto *string_item = container_.GetOrCreateStringItem(item_str);
            inst.UpdateId(BytecodeId(string_item->GetFileId().GetOffset()));
        } else if (inst.HasFlag(Flags::LITERALARRAY_ID)) {
            BytecodeId b_id = inst.GetId();
            File::EntityId old_id = b_id.AsFileId();
            ASSERT(items_done_.find(old_id) != items_done_.end());
            auto *array_item = items_done_.find(old_id)->second;
            inst.UpdateId(BytecodeId(array_item->GetFileId().GetOffset()));
        }
        offset += inst.GetSize();
        inst = inst.GetNext();
    }
}

void FileReader::ComputeLayoutAndUpdateIndices()
{
    std::map<BaseItem *, File::EntityId> reverse_done;
    for (const auto &it : items_done_) {
        reverse_done.insert({it.second, it.first});
    }

    auto *class_map = container_.GetClassMap();

    UpdateCodeAndDebugInfoDependencies(reverse_done);

    container_.ComputeLayout();

    // Second pass, update debug info
    for (const auto &it : *class_map) {
        auto *base_class_item = it.second;
        if (base_class_item->IsForeign()) {
            continue;
        }
        auto *class_item = static_cast<ClassItem *>(base_class_item);
        class_item->VisitMethods([this, &reverse_done](BaseItem *param_item) {
            auto *method_item = static_cast<MethodItem *>(param_item);
            auto *code_item = method_item->GetCode();
            if (code_item == nullptr) {
                return true;
            }

            auto *debug_info_item = method_item->GetDebugInfo();
            if (debug_info_item != nullptr) {
                UpdateDebugInfo(debug_info_item, reverse_done.find(debug_info_item)->second);
            }

            return true;
        });
    }

    container_.DeduplicateItems(false);
    container_.ComputeLayout();

    // Third pass, update bytecode indices
    for (const auto &it : *class_map) {
        auto *base_class_item = it.second;
        if (base_class_item->IsForeign()) {
            continue;
        }
        auto *class_item = static_cast<ClassItem *>(base_class_item);
        class_item->VisitMethods([this, &reverse_done](BaseItem *param_item) {
            auto *method_item = static_cast<MethodItem *>(param_item);
            auto *code_item = method_item->GetCode();
            if (code_item == nullptr) {
                return true;
            }

            UpdateIdInstFlag(code_item, method_item, reverse_done);

            return true;
        });
    }
}

}  // namespace panda::panda_file
