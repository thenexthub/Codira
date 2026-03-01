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

#include "libarkfile/file_items.h"
#include "libarkbase/macros.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/leb128.h"
#include "libarkbase/utils/utf.h"

#include <iomanip>

namespace ark::panda_file {

#include <libarkfile/include/file_items_gen.inc>

size_t IndexedItem::itemAllocIdNext_ = 0;

template <class Tag, class Val>
static bool WriteUlebTaggedValue(Writer *writer, Tag tag, Val v)
{
    if (!writer->WriteByte(static_cast<uint8_t>(tag))) {
        return false;
    }

    if (!writer->WriteUleb128(v)) {
        return false;
    }

    return true;
}

template <class Tag, class Val>
static bool WriteSlebTaggedValue(Writer *writer, Tag tag, Val v)
{
    if (!writer->WriteByte(static_cast<uint8_t>(tag))) {
        return false;
    }

    if (!writer->WriteSleb128(v)) {
        return false;
    }

    return true;
}

template <class Tag, class Val>
static bool WriteTaggedValue(Writer *writer, Tag tag, Val v)
{
    if (!writer->WriteByte(static_cast<uint8_t>(tag))) {
        return false;
    }

    if (!writer->Write(v)) {
        return false;
    }

    return true;
}

template <class Tag>
static bool WriteIdTaggedValue(Writer *writer, Tag tag, BaseItem *item)
{
    ASSERT(item->GetOffset() != 0);
    return WriteTaggedValue(writer, tag, item->GetOffset());
}

// CC-OFFNXT(G.FUN.01-CPP, huge_cyclomatic_complexity[C++], huge_method[C++]) big switch case
std::string ItemTypeToString(ItemTypes type)
{
    switch (type) {
        case ItemTypes::ANNOTATION_ITEM:
            return "annotation_item";
        case ItemTypes::CATCH_BLOCK_ITEM:
            return "catch_block_item";
        case ItemTypes::CLASS_INDEX_ITEM:
            return "class_index_item";
        case ItemTypes::CLASS_ITEM:
            return "class_item";
        case ItemTypes::CODE_ITEM:
            return "code_item";
        case ItemTypes::DEBUG_INFO_ITEM:
            return "debug_info_item";
        case ItemTypes::END_ITEM:
            return "end_item";
        case ItemTypes::FIELD_INDEX_ITEM:
            return "field_index_item";
        case ItemTypes::FIELD_ITEM:
            return "field_item";
        case ItemTypes::FOREIGN_CLASS_ITEM:
            return "foreign_class_item";
        case ItemTypes::FOREIGN_FIELD_ITEM:
            return "foreign_field_item";
        case ItemTypes::FOREIGN_METHOD_ITEM:
            return "foreign_method_item";
        case ItemTypes::LINE_NUMBER_PROGRAM_INDEX_ITEM:
            return "line_number_program_index_item";
        case ItemTypes::LINE_NUMBER_PROGRAM_ITEM:
            return "line_number_program_item";
        case ItemTypes::LITERAL_ARRAY_ITEM:
            return "literal_array_item";
        case ItemTypes::LITERAL_ITEM:
            return "literal_item";
        case ItemTypes::METHOD_HANDLE_ITEM:
            return "method_handle_item";
        case ItemTypes::METHOD_INDEX_ITEM:
            return "method_index_item";
        case ItemTypes::METHOD_ITEM:
            return "method_item";
        case ItemTypes::PARAM_ANNOTATIONS_ITEM:
            return "param_annotations_item";
        case ItemTypes::PRIMITIVE_TYPE_ITEM:
            return "primitive_type_item";
        case ItemTypes::PROTO_INDEX_ITEM:
            return "proto_index_item";
        case ItemTypes::PROTO_ITEM:
            return "proto_item";
        case ItemTypes::REGION_HEADER:
            return "region_header";
        case ItemTypes::REGION_SECTION:
            return "region_section";
        case ItemTypes::STRING_ITEM:
            return "string_item";
        case ItemTypes::TRY_BLOCK_ITEM:
            return "try_block_item";
        case ItemTypes::VALUE_ITEM:
            return "value_item";
        default:
            return "";
    }
}

std::string BaseItem::GetName() const
{
    return ItemTypeToString(GetItemType());
}

StringItem::StringItem(std::string str) : str_(std::move(str))
{
    str_.push_back(0);
    utf16Length_ = utf::MUtf8ToUtf16Size(utf::CStringAsMutf8(str_.data()));
    isAscii_ = 1;

    for (auto c : str_) {
        if (static_cast<uint8_t>(c) > utf::UTF8_1B_MAX) {
            isAscii_ = 0;
            break;
        }
    }
}

StringItem::StringItem(File::StringData data)
    : str_(reinterpret_cast<const char *>(data.data)), utf16Length_(data.utf16Length)
{
}

size_t StringItem::CalculateSize() const
{
    size_t n = str_.size();
    return leb128::UnsignedEncodingSize((utf16Length_ << 1U) | isAscii_) + n;
}

bool StringItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());
    constexpr size_t MAX_LENGTH = 0x7fffffffU;
    if (utf16Length_ > MAX_LENGTH) {
        LOG(ERROR, PANDAFILE) << "Writing StringItem with size greater than 0x7fffffffU is not supported!";
        return false;
    }

    if (!writer->WriteUleb128((utf16Length_ << 1U) | isAscii_)) {
        return false;
    }

    for (auto c : str_) {
        if (!writer->WriteByte(static_cast<uint8_t>(c))) {
            return false;
        }
    }
    return true;
}

size_t BaseClassItem::CalculateSize() const
{
    return name_.GetSize();
}

void BaseClassItem::ComputeLayout()
{
    uint32_t offset = GetOffset();

    ASSERT(offset != 0);

    name_.SetOffset(offset);
}

bool BaseClassItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());
    return name_.Write(writer);
}

size_t ClassItem::CalculateSizeWithoutFieldsAndMethods() const
{
    size_t size = BaseClassItem::CalculateSize() + ID_SIZE + leb128::UnsignedEncodingSize(accessFlags_);

    size += leb128::UnsignedEncodingSize(fields_.size());
    size += leb128::UnsignedEncodingSize(methods_.size());

    if (!ifaces_.empty()) {
        size += TAG_SIZE + leb128::UnsignedEncodingSize(ifaces_.size()) + IDX_SIZE * ifaces_.size();
    }

    if (sourceLang_ != SourceLang::PANDA_ASSEMBLY) {
        size += TAG_SIZE + sizeof(SourceLang);
    }

    size += (TAG_SIZE + ID_SIZE) * runtimeAnnotations_.size();
    size += (TAG_SIZE + ID_SIZE) * annotations_.size();
    size += (TAG_SIZE + ID_SIZE) * runtimeTypeAnnotations_.size();
    size += (TAG_SIZE + ID_SIZE) * typeAnnotations_.size();

    if (sourceFile_ != nullptr) {
        size += TAG_SIZE + ID_SIZE;
    }

    size += TAG_SIZE;  // null tag

    return size;
}

size_t ClassItem::CalculateSize() const
{
    size_t size = CalculateSizeWithoutFieldsAndMethods();

    for (auto &field : fields_) {
        size += field->GetSize();
    }

    for (auto &method : methods_) {
        size += method->GetSize();
    }

    return size;
}

void ClassItem::ComputeLayout()
{
    BaseClassItem::ComputeLayout();

    uint32_t offset = GetOffset();

    offset += CalculateSizeWithoutFieldsAndMethods();

    for (auto &field : fields_) {
        field->SetOffset(offset);
        field->ComputeLayout();
        offset += field->GetSize();
    }

    for (auto &method : methods_) {
        method->SetOffset(offset);
        method->ComputeLayout();
        offset += method->GetSize();
    }
}

bool ClassItem::WriteIfaces(Writer *writer)
{
    if (!ifaces_.empty()) {
        if (!writer->WriteByte(static_cast<uint8_t>(ClassTag::INTERFACES))) {
            return false;
        }

        if (!writer->WriteUleb128(ifaces_.size())) {
            return false;
        }

        for (auto iface : ifaces_) {
            ASSERT(iface->HasIndex(this));
            if (!writer->Write<uint16_t>(iface->GetIndex(this))) {
                return false;
            }
        }
    }

    return true;
}

bool ClassItem::WriteAnnotations(Writer *writer)
{
    for (auto runtimeAnnotation : runtimeAnnotations_) {
        if (!WriteIdTaggedValue(writer, ClassTag::RUNTIME_ANNOTATION, runtimeAnnotation)) {
            return false;
        }
    }

    for (auto annotation : annotations_) {
        if (!WriteIdTaggedValue(writer, ClassTag::ANNOTATION, annotation)) {
            return false;
        }
    }

    for (auto runtimeTypeAnnotation : runtimeTypeAnnotations_) {
        if (!WriteIdTaggedValue(writer, ClassTag::RUNTIME_TYPE_ANNOTATION, runtimeTypeAnnotation)) {
            return false;
        }
    }

    for (auto typeAnnotation : typeAnnotations_) {
        if (!WriteIdTaggedValue(writer, ClassTag::TYPE_ANNOTATION, typeAnnotation)) {
            return false;
        }
    }

    return true;
}

bool ClassItem::WriteTaggedData(Writer *writer)
{
    if (!WriteIfaces(writer)) {
        return false;
    }

    if (sourceLang_ != SourceLang::PANDA_ASSEMBLY) {
        if (!WriteTaggedValue(writer, ClassTag::SOURCE_LANG, static_cast<uint8_t>(sourceLang_))) {
            return false;
        }
    }

    if (!WriteAnnotations(writer)) {
        return false;
    }

    if (sourceFile_ != nullptr) {
        if (!WriteIdTaggedValue(writer, ClassTag::SOURCE_FILE, sourceFile_)) {
            return false;
        }
    }

    return writer->WriteByte(static_cast<uint8_t>(ClassTag::NOTHING));
}

bool ClassItem::Write(Writer *writer)
{
    if (!BaseClassItem::Write(writer)) {
        return false;
    }

    uint32_t offset = superClass_ != nullptr ? superClass_->GetOffset() : 0;
    if (!writer->Write(offset)) {
        return false;
    }

    if (!writer->WriteUleb128(accessFlags_)) {
        return false;
    }

    if (!writer->WriteUleb128(fields_.size())) {
        return false;
    }

    if (!writer->WriteUleb128(methods_.size())) {
        return false;
    }

    if (!WriteTaggedData(writer)) {
        return false;
    }

    for (auto &field : fields_) {
        if (!field->Write(writer)) {
            return false;
        }
    }

    for (auto &method : methods_) {
        if (!method->Write(writer)) {
            return false;
        }
    }

    return true;
}

void ClassItem::SetDependencyMark()
{
    BaseItem::SetDependencyMark();
    for (auto &iface : ifaces_) {
        iface->SetDependencyMark();
    }
    if (superClass_ != nullptr) {
        superClass_->SetDependencyMark();
    }
    if (sourceFile_ != nullptr) {
        sourceFile_->SetDependencyMark();
    }
    for (auto &item : annotations_) {
        item->SetDependencyMark();
    }
    for (auto &item : runtimeAnnotations_) {
        item->SetDependencyMark();
    }
    for (auto &item : typeAnnotations_) {
        item->SetDependencyMark();
    }
    for (auto &item : runtimeTypeAnnotations_) {
        item->SetDependencyMark();
    }
    for (auto &field : fields_) {
        if (!field->GetDependencyMark()) {
            field->SetDependencyMark();
        }
    }
    for (auto &method : methods_) {
        if (!method->GetDependencyMark()) {
            method->SetDependencyMark();
        }
    }
}

ParamAnnotationsItem::ParamAnnotationsItem(MethodItem *method, bool isRuntimeAnnotations)
{
    for (const auto &param : method->GetParams()) {
        if (isRuntimeAnnotations) {
            annotations_.push_back(param.GetRuntimeAnnotations());
        } else {
            annotations_.push_back(param.GetAnnotations());
        }
    }

    if (isRuntimeAnnotations) {
        method->SetRuntimeParamAnnotationItem(this);
    } else {
        method->SetParamAnnotationItem(this);
    }
}

size_t ParamAnnotationsItem::CalculateSize() const
{
    size_t size = sizeof(uint32_t);  // size

    for (const auto &paramAnnotations : annotations_) {
        size += sizeof(uint32_t);  // count
        size += paramAnnotations.size() * ID_SIZE;
    }

    return size;
}

bool ParamAnnotationsItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    if (!writer->Write(static_cast<uint32_t>(annotations_.size()))) {
        return false;
    }

    for (const auto &paramAnnotations : annotations_) {
        if (!writer->Write(static_cast<uint32_t>(paramAnnotations.size()))) {
            return false;
        }

        for (auto *item : paramAnnotations) {
            ASSERT(item->GetOffset() != 0);

            if (!writer->Write(item->GetOffset())) {
                return false;
            }
        }
    }

    return true;
}

ProtoItem::ProtoItem(TypeItem *retType, const std::vector<MethodParamItem> &params)
{
    size_t n = 0;
    shorty_.push_back(0);
    AddType(retType, &n);
    for (auto &p : params) {
        AddType(p.GetType(), &n);
    }
}

void ProtoItem::AddType(TypeItem *type, size_t *n)
{
    constexpr size_t SHORTY_ELEMS_COUNT = std::numeric_limits<uint16_t>::digits / SHORTY_ELEM_SIZE;

    uint16_t v = shorty_.back();

    size_t shift = (*n % SHORTY_ELEMS_COUNT) * SHORTY_ELEM_SIZE;

    v |= static_cast<uint16_t>(static_cast<uint16_t>(type->GetType().GetEncoding()) << shift);
    shorty_.back() = v;

    if (!type->GetType().IsPrimitive()) {
        referenceTypes_.push_back(type);
        AddIndexDependency(type);
    }

    *n += 1;

    if (*n % SHORTY_ELEMS_COUNT == 0) {
        shorty_.push_back(0);
    }
}

bool ProtoItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());
    for (auto s : shorty_) {
        if (!writer->Write(s)) {
            return false;
        }
    }

    for (auto r : referenceTypes_) {
        ASSERT(r->HasIndex(this));
        if (!writer->Write<uint16_t>(r->GetIndex(this))) {
            return false;
        }
    }

    return true;
}

void ProtoItem::SetDependencyMark()
{
    BaseItem::SetDependencyMark();
    for (auto item : referenceTypes_) {
        item->SetDependencyMark();
    }
}

BaseMethodItem::BaseMethodItem(BaseClassItem *cls, StringItem *name, ProtoItem *proto, uint32_t accessFlags)
    : class_(cls), name_(name), proto_(proto), accessFlags_(accessFlags)
{
    AddIndexDependency(cls);
    AddIndexDependency(proto);
}

size_t BaseMethodItem::CalculateSize() const
{
    // class id + proto id + name id + access flags
    return IDX_SIZE + IDX_SIZE + ID_SIZE + leb128::UnsignedEncodingSize(accessFlags_);
}

bool BaseMethodItem::Write(Writer *writer)
{
    ASSERT(writer != nullptr);
    ASSERT(GetOffset() == writer->GetOffset());
    ASSERT(class_ != nullptr);
    ASSERT(class_->HasIndex(this));

    if (!writer->Write<uint16_t>(class_->GetIndex(this))) {
        return false;
    }

    ASSERT(proto_->HasIndex(this));

    if (!writer->Write<uint16_t>(proto_->GetIndex(this))) {
        return false;
    }

    ASSERT(name_->GetOffset() != 0);

    if (!writer->Write(name_->GetOffset())) {
        return false;
    }

    return writer->WriteUleb128(accessFlags_);
}

void BaseMethodItem::SetDependencyMark()
{
    BaseItem::SetDependencyMark();
    if (class_ != nullptr) {
        if (!class_->GetDependencyMark()) {
            class_->SetDependencyMark();
        }
    }
    if (name_ != nullptr) {
        name_->SetDependencyMark();
    }
    if (proto_ != nullptr) {
        proto_->SetDependencyMark();
    }
    auto deps = GetIndexDependencies();
    for (auto dep : deps) {
        if (!dep->GetDependencyMark()) {
            dep->SetDependencyMark();
        }
    }
}

MethodItem::MethodItem(ClassItem *cls, StringItem *name, ProtoItem *proto, uint32_t accessFlags,
                       std::vector<MethodParamItem> params)
    : BaseMethodItem(cls, name, proto, accessFlags), params_(std::move(params))
{
}

size_t MethodItem::CalculateSize() const
{
    size_t size = BaseMethodItem::CalculateSize();

    if (code_ != nullptr) {
        size += TAG_SIZE + ID_SIZE;
    }

    if (sourceLang_ != SourceLang::PANDA_ASSEMBLY) {
        size += TAG_SIZE + sizeof(SourceLang);
    }

    size += (TAG_SIZE + ID_SIZE) * runtimeAnnotations_.size();

    if (runtimeParamAnnotations_ != nullptr) {
        size += TAG_SIZE + ID_SIZE;
    }

    size += (TAG_SIZE + ID_SIZE) * annotations_.size();

    if (paramAnnotations_ != nullptr) {
        size += TAG_SIZE + ID_SIZE;
    }

    size += (TAG_SIZE + ID_SIZE) * runtimeTypeAnnotations_.size();
    size += (TAG_SIZE + ID_SIZE) * typeAnnotations_.size();

    if (debugInfo_ != nullptr) {
        size += TAG_SIZE + ID_SIZE;
    }

    if (profileSize_ != 0) {
        size += TAG_SIZE + sizeof(profileSize_);
    }

    size += TAG_SIZE;  // null tag

    return size;
}

bool MethodItem::WriteRuntimeAnnotations(Writer *writer)
{
    for (auto runtimeAnnotation : runtimeAnnotations_) {
        if (!WriteIdTaggedValue(writer, MethodTag::RUNTIME_ANNOTATION, runtimeAnnotation)) {
            return false;
        }
    }

    if (runtimeParamAnnotations_ != nullptr) {
        if (!WriteIdTaggedValue(writer, MethodTag::RUNTIME_PARAM_ANNOTATION, runtimeParamAnnotations_)) {
            return false;
        }
    }

    return true;
}

bool MethodItem::WriteTypeAnnotations(Writer *writer)
{
    for (auto runtimeTypeAnnotation : runtimeTypeAnnotations_) {
        if (!WriteIdTaggedValue(writer, MethodTag::RUNTIME_TYPE_ANNOTATION, runtimeTypeAnnotation)) {
            return false;
        }
    }

    for (auto typeAnnotation : typeAnnotations_) {
        if (!WriteIdTaggedValue(writer, MethodTag::TYPE_ANNOTATION, typeAnnotation)) {
            return false;
        }
    }

    return true;
}

bool MethodItem::WriteTaggedData(Writer *writer)
{
    if (code_ != nullptr) {
        if (!WriteIdTaggedValue(writer, MethodTag::CODE, code_)) {
            return false;
        }
    }

    if (sourceLang_ != SourceLang::PANDA_ASSEMBLY) {
        if (!WriteTaggedValue(writer, MethodTag::SOURCE_LANG, static_cast<uint8_t>(sourceLang_))) {
            return false;
        }
    }

    if (!WriteRuntimeAnnotations(writer)) {
        return false;
    }

    if (debugInfo_ != nullptr) {
        if (!WriteIdTaggedValue(writer, MethodTag::DEBUG_INFO, debugInfo_)) {
            return false;
        }
    }

    for (auto annotation : annotations_) {
        if (!WriteIdTaggedValue(writer, MethodTag::ANNOTATION, annotation)) {
            return false;
        }
    }

    if (!WriteTypeAnnotations(writer)) {
        return false;
    }

    if (paramAnnotations_ != nullptr) {
        if (!WriteIdTaggedValue(writer, MethodTag::PARAM_ANNOTATION, paramAnnotations_)) {
            return false;
        }
    }

    if (profileSize_ != 0) {
        if (!WriteTaggedValue(writer, MethodTag::PROFILE_INFO, static_cast<uint16_t>(profileSize_))) {
            return false;
        }
    }

    return writer->WriteByte(static_cast<uint8_t>(MethodTag::NOTHING));
}

bool MethodItem::Write(Writer *writer)
{
    if (!BaseMethodItem::Write(writer)) {
        return false;
    }

    return WriteTaggedData(writer);
}

void MethodItem::SetDependencyMark()
{
    BaseMethodItem::SetDependencyMark();
    if (code_ != nullptr) {
        code_->SetDependencyMark();
    }
    if (debugInfo_ != nullptr) {
        debugInfo_->SetDependencyMark();
    }
    for (auto &item : annotations_) {
        item->SetDependencyMark();
    }
    for (auto &item : runtimeAnnotations_) {
        item->SetDependencyMark();
    }
    for (auto &item : typeAnnotations_) {
        item->SetDependencyMark();
    }
    for (auto &item : runtimeTypeAnnotations_) {
        item->SetDependencyMark();
    }
}

size_t CodeItem::CatchBlock::CalculateSize() const
{
    ASSERT(type_ == nullptr || type_->HasIndex(method_));
    uint32_t typeOff = type_ != nullptr ? type_->GetIndex(method_) + 1 : 0;
    return leb128::UnsignedEncodingSize(typeOff) + leb128::UnsignedEncodingSize(handlerPc_) +
           leb128::UnsignedEncodingSize(codeSize_);
}

bool CodeItem::CatchBlock::Write(Writer *writer)
{
    ASSERT(writer != nullptr);
    ASSERT(GetOffset() == writer->GetOffset());
    ASSERT(type_ == nullptr || type_->HasIndex(method_));

    uint32_t typeOff = type_ != nullptr ? type_->GetIndex(method_) + 1 : 0;
    if (!writer->WriteUleb128(typeOff)) {
        return false;
    }

    if (!writer->WriteUleb128(handlerPc_)) {
        return false;
    }

    if (!writer->WriteUleb128(codeSize_)) {
        return false;
    }

    return true;
}

void CodeItem::TryBlock::ComputeLayout()
{
    size_t offset = GetOffset();
    offset += CalculateSizeWithoutCatchBlocks();

    for (auto &catchBlock : catchBlocks_) {
        catchBlock.SetOffset(offset);
        catchBlock.ComputeLayout();
        offset += catchBlock.GetSize();
    }
}

size_t CodeItem::TryBlock::CalculateSizeWithoutCatchBlocks() const
{
    return leb128::UnsignedEncodingSize(startPc_) + leb128::UnsignedEncodingSize(length_) +
           leb128::UnsignedEncodingSize(catchBlocks_.size());
}

size_t CodeItem::TryBlock::CalculateSize() const
{
    size_t size = CalculateSizeWithoutCatchBlocks();

    for (auto &catchBlock : catchBlocks_) {
        size += catchBlock.GetSize();
    }

    return size;
}

bool CodeItem::TryBlock::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    if (!writer->WriteUleb128(startPc_)) {
        return false;
    }

    if (!writer->WriteUleb128(length_)) {
        return false;
    }

    if (!writer->WriteUleb128(catchBlocks_.size())) {
        return false;
    }

    for (auto &catchBlock : catchBlocks_) {
        if (!catchBlock.Write(writer)) {
            return false;
        }
    }

    return true;
}

void CodeItem::ComputeLayout()
{
    uint32_t offset = GetOffset();

    offset += CalculateSizeWithoutTryBlocks();

    for (auto &tryBlock : tryBlocks_) {
        tryBlock.SetOffset(offset);
        tryBlock.ComputeLayout();
        offset += tryBlock.GetSize();
    }
}

size_t CodeItem::CalculateSizeWithoutTryBlocks() const
{
    size_t size = leb128::UnsignedEncodingSize(numVregs_) + leb128::UnsignedEncodingSize(numArgs_) +
                  leb128::UnsignedEncodingSize(instructions_.size()) + leb128::UnsignedEncodingSize(tryBlocks_.size());

    size += instructions_.size();

    return size;
}

size_t CodeItem::GetCodeSize() const
{
    return instructions_.size();
}

size_t CodeItem::CalculateSize() const
{
    size_t size = CalculateSizeWithoutTryBlocks();

    for (auto &tryBlock : tryBlocks_) {
        size += tryBlock.GetSize();
    }

    return size;
}

bool CodeItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    if (!writer->WriteUleb128(numVregs_)) {
        return false;
    }

    if (!writer->WriteUleb128(numArgs_)) {
        return false;
    }

    if (!writer->WriteUleb128(instructions_.size())) {
        return false;
    }

    if (!writer->WriteUleb128(tryBlocks_.size())) {
        return false;
    }

    if (!writer->WriteBytes(instructions_)) {
        return false;
    }

    for (auto &tryBlock : tryBlocks_) {
        if (!tryBlock.Write(writer)) {
            return false;
        }
    }

    return true;
}

ScalarValueItem *ValueItem::GetAsScalar()
{
    ASSERT(!IsArray());
    return static_cast<ScalarValueItem *>(this);
}

ArrayValueItem *ValueItem::GetAsArray()
{
    ASSERT(IsArray());
    return static_cast<ArrayValueItem *>(this);
}

size_t ScalarValueItem::GetULeb128EncodedSize()
{
    switch (GetType()) {
        case Type::INTEGER:
            return leb128::UnsignedEncodingSize(GetValue<uint32_t>());

        case Type::LONG:
            return leb128::UnsignedEncodingSize(GetValue<uint64_t>());

        case Type::ID:
            return leb128::UnsignedEncodingSize(GetId().GetOffset());

        default:
            return 0;
    }
}

size_t ScalarValueItem::GetSLeb128EncodedSize()
{
    switch (GetType()) {
        case Type::INTEGER:
            return leb128::SignedEncodingSize(static_cast<int32_t>(GetValue<uint32_t>()));

        case Type::LONG:
            return leb128::SignedEncodingSize(static_cast<int64_t>(GetValue<uint64_t>()));

        default:
            return 0;
    }
}

size_t ScalarValueItem::CalculateSize() const
{
    size_t size = 0;
    switch (GetType()) {
        case Type::INTEGER: {
            size = sizeof(uint32_t);
            break;
        }

        case Type::LONG: {
            size = sizeof(uint64_t);
            break;
        }

        case Type::FLOAT: {
            size = sizeof(float);
            break;
        }

        case Type::DOUBLE: {
            size = sizeof(double);
            break;
        }

        case Type::ID: {
            size = ID_SIZE;
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }

    return size;
}

size_t ScalarValueItem::Alignment()
{
    return GetSize();
}

bool ScalarValueItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    switch (GetType()) {
        case Type::INTEGER:
            return writer->Write(GetValue<uint32_t>());

        case Type::LONG:
            return writer->Write(GetValue<uint64_t>());

        case Type::FLOAT:
            return writer->Write(bit_cast<uint32_t>(GetValue<float>()));

        case Type::DOUBLE:
            return writer->Write(bit_cast<uint64_t>(GetValue<double>()));

        case Type::ID: {
            ASSERT(GetId().IsValid());
            return writer->Write(GetId().GetOffset());
        }
        default: {
            UNREACHABLE();
            break;
        }
    }

    return true;
}

bool ScalarValueItem::WriteAsUleb128(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    switch (GetType()) {
        case Type::INTEGER:
            return writer->WriteUleb128(GetValue<uint32_t>());

        case Type::LONG:
            return writer->WriteUleb128(GetValue<uint64_t>());

        case Type::ID: {
            ASSERT(GetId().IsValid());
            return writer->WriteUleb128(GetId().GetOffset());
        }
        default:
            return false;
    }
}

size_t ArrayValueItem::CalculateSize() const
{
    size_t size = leb128::UnsignedEncodingSize(items_.size()) + items_.size() * GetComponentSize();
    return size;
}

void ArrayValueItem::ComputeLayout()
{
    uint32_t offset = GetOffset();

    ASSERT(offset != 0);

    offset += leb128::UnsignedEncodingSize(items_.size());

    for (auto &item : items_) {
        item.SetOffset(offset);
        offset += GetComponentSize();
    }
}

bool ArrayValueItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    if (!writer->WriteUleb128(items_.size())) {
        return false;
    }

    switch (componentType_.GetId()) {
        case panda_file::Type::TypeId::U1:
        case panda_file::Type::TypeId::I8:
        case panda_file::Type::TypeId::U8: {
            for (auto &item : items_) {
                auto value = static_cast<uint8_t>(item.GetValue<uint32_t>());
                if (!writer->Write(value)) {
                    return false;
                }
            }
            break;
        }
        case panda_file::Type::TypeId::I16:
        case panda_file::Type::TypeId::U16: {
            for (auto &item : items_) {
                auto value = static_cast<uint16_t>(item.GetValue<uint32_t>());
                if (!writer->Write(value)) {
                    return false;
                }
            }
            break;
        }
        default: {
            for (auto &item : items_) {
                if (!item.Write(writer)) {
                    return false;
                }
            }
            break;
        }
    }

    return true;
}

size_t ArrayValueItem::GetComponentSize() const
{
    switch (componentType_.GetId()) {
        case panda_file::Type::TypeId::U1:
        case panda_file::Type::TypeId::I8:
        case panda_file::Type::TypeId::U8:
            return sizeof(uint8_t);
        case panda_file::Type::TypeId::I16:
        case panda_file::Type::TypeId::U16:
            return sizeof(uint16_t);
        case panda_file::Type::TypeId::I32:
        case panda_file::Type::TypeId::U32:
        case panda_file::Type::TypeId::F32:
            return sizeof(uint32_t);
        case panda_file::Type::TypeId::I64:
        case panda_file::Type::TypeId::U64:
        case panda_file::Type::TypeId::F64:
            return sizeof(uint64_t);
        case panda_file::Type::TypeId::REFERENCE:
            return ID_SIZE;
        case panda_file::Type::TypeId::VOID:
            return 0;
        default: {
            UNREACHABLE();
            // Avoid cpp warning
            return 0;
        }
    }
}

void ArrayValueItem::SetDependencyMark()
{
    for (auto &it : items_) {
        it.SetDependencyMark();
    }
    BaseItem::SetDependencyMark();
}

size_t LiteralItem::CalculateSize() const
{
    size_t size = 0;
    switch (GetType()) {
        case Type::B1: {
            size = sizeof(uint8_t);
            break;
        }

        case Type::B2: {
            size = sizeof(uint16_t);
            break;
        }

        case Type::B4: {
            size = sizeof(uint32_t);
            break;
        }

        case Type::B8: {
            size = sizeof(uint64_t);
            break;
        }

        case Type::STRING:
        case Type::METHOD:
        case Type::LITERALARRAY: {
            size = ID_SIZE;
            break;
        }

        default: {
            UNREACHABLE();
            break;
        }
    }

    return size;
}

size_t LiteralItem::Alignment()
{
    return GetSize();
}

File::EntityId LiteralItem::GetLiteralArrayFileId() const
{
    return File::EntityId(GetValue<LiteralArrayItem *>()->GetOffset());
}

void LiteralItem::SetDependencyMark()
{
    BaseItem::SetDependencyMark();
    if (type_ == Type::STRING) {
        auto stringItem = GetValue<StringItem *>();
        stringItem->SetDependencyMark();
    } else if (type_ == Type::METHOD) {
        auto methodItem = GetValue<MethodItem *>();
        methodItem->SetDependencyMark();
    }
}

bool LiteralItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    switch (GetType()) {
        case Type::B1: {
            return writer->Write(GetValue<uint8_t>());
        }
        case Type::B2: {
            return writer->Write(GetValue<uint16_t>());
        }
        case Type::B4: {
            return writer->Write(GetValue<uint32_t>());
        }
        case Type::B8: {
            return writer->Write(GetValue<uint64_t>());
        }
        case Type::STRING: {
            ASSERT(GetId().IsValid());
            return writer->Write(GetId().GetOffset());
        }
        case Type::METHOD: {
            ASSERT(GetMethodId().IsValid());
            return writer->Write(GetMethodId().GetOffset());
        }
        case Type::LITERALARRAY: {
            ASSERT(GetLiteralArrayFileId().IsValid());
            return writer->Write(GetLiteralArrayFileId().GetOffset());
        }
        default: {
            UNREACHABLE();
            break;
        }
    }

    return true;
}

void LiteralArrayItem::AddItems(const std::vector<LiteralItem> &item)
{
    items_.assign(item.begin(), item.end());
}

size_t LiteralArrayItem::CalculateSize() const
{
    size_t size = sizeof(uint32_t);
    for (auto &item : items_) {
        size += item.CalculateSize();
    }

    return size;
}

void LiteralArrayItem::ComputeLayout()
{
    uint32_t offset = GetOffset();

    ASSERT(offset != 0);

    offset += sizeof(uint32_t);

    for (auto &item : items_) {
        item.SetOffset(offset);
        offset += item.CalculateSize();
    }
}

bool LiteralArrayItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    if (!writer->Write(static_cast<uint32_t>(items_.size()))) {
        return false;
    }

    for (auto &item : items_) {
        if (!item.Write(writer)) {
            return false;
        }
    }

    return true;
}

void LiteralArrayItem::SetDependencyMark()
{
    BaseItem::SetDependencyMark();
    for (auto &it : items_) {
        it.SetDependencyMark();
    }
}

BaseFieldItem::BaseFieldItem(BaseClassItem *cls, StringItem *name, TypeItem *type, uint32_t accessFlags = 0U)
    : class_(cls), name_(name), type_(type), accessFlags_(accessFlags)
{
    AddIndexDependency(cls);
    AddIndexDependency(type);
}

size_t BaseFieldItem::CalculateSize() const
{
    // class id + type id + name id
    return IDX_SIZE + IDX_SIZE + ID_SIZE + leb128::UnsignedEncodingSize(GetAccessFlags());
}

bool BaseFieldItem::Write(Writer *writer)
{
    ASSERT(writer != nullptr);
    ASSERT(GetOffset() == writer->GetOffset());
    ASSERT(class_->HasIndex(this));
    ASSERT(type_->HasIndex(this));

    if (!writer->Write<uint16_t>(class_->GetIndex(this))) {
        return false;
    }

    if (!writer->Write<uint16_t>(type_->GetIndex(this))) {
        return false;
    }

    if (!writer->Write(name_->GetOffset())) {
        return false;
    }

    return writer->WriteUleb128(accessFlags_);
}

void BaseFieldItem::SetDependencyMark()
{
    BaseItem::SetDependencyMark();
    if (class_ != nullptr) {
        if (!class_->GetDependencyMark()) {
            class_->SetDependencyMark();
        }
    }
    if (name_ != nullptr) {
        name_->SetDependencyMark();
    }
    if (type_ != nullptr) {
        type_->SetDependencyMark();
    }
}

FieldItem::FieldItem(ClassItem *cls, StringItem *name, TypeItem *type, uint32_t accessFlags)
    : BaseFieldItem(cls, name, type, accessFlags)
{
}

void FieldItem::SetValue(ValueItem *value)
{
    value_ = value;
    value_->SetNeedsEmit(!value_->Is32bit());
}

size_t FieldItem::CalculateSize() const
{
    size_t size = BaseFieldItem::CalculateSize();

    if (value_ != nullptr) {
        if (value_->GetType() == ValueItem::Type::INTEGER) {
            size += TAG_SIZE + value_->GetAsScalar()->GetSLeb128EncodedSize();
        } else {
            size += TAG_SIZE + ID_SIZE;
        }
    }

    size += (TAG_SIZE + ID_SIZE) * runtimeAnnotations_.size();
    size += (TAG_SIZE + ID_SIZE) * annotations_.size();
    size += (TAG_SIZE + ID_SIZE) * runtimeTypeAnnotations_.size();
    size += (TAG_SIZE + ID_SIZE) * typeAnnotations_.size();

    size += TAG_SIZE;  // null tag

    return size;
}

bool FieldItem::WriteValue(Writer *writer)
{
    if (value_ == nullptr) {
        return true;
    }

    if (value_->GetType() == ValueItem::Type::INTEGER) {
        auto v = static_cast<int32_t>(value_->GetAsScalar()->GetValue<uint32_t>());
        if (!WriteSlebTaggedValue(writer, FieldTag::INT_VALUE, v)) {
            return false;
        }
    } else if (value_->GetType() == ValueItem::Type::FLOAT) {
        auto v = bit_cast<uint32_t>(value_->GetAsScalar()->GetValue<float>());
        if (!WriteTaggedValue(writer, FieldTag::VALUE, v)) {
            return false;
        }
    } else if (value_->GetType() == ValueItem::Type::ID) {
        auto id = value_->GetAsScalar()->GetId();
        ASSERT(id.GetOffset() != 0);
        if (!WriteTaggedValue(writer, FieldTag::VALUE, id.GetOffset())) {
            return false;
        }
    } else {
        ASSERT(!value_->Is32bit());
        if (!WriteIdTaggedValue(writer, FieldTag::VALUE, value_)) {
            return false;
        }
    }

    return true;
}

bool FieldItem::WriteAnnotations(Writer *writer)
{
    for (auto runtimeAnnotation : runtimeAnnotations_) {
        if (!WriteIdTaggedValue(writer, FieldTag::RUNTIME_ANNOTATION, runtimeAnnotation)) {
            return false;
        }
    }

    for (auto annotation : annotations_) {
        if (!WriteIdTaggedValue(writer, FieldTag::ANNOTATION, annotation)) {
            return false;
        }
    }

    for (auto runtimeTypeAnnotation : runtimeTypeAnnotations_) {
        if (!WriteIdTaggedValue(writer, FieldTag::RUNTIME_TYPE_ANNOTATION, runtimeTypeAnnotation)) {
            return false;
        }
    }

    for (auto typeAnnotation : typeAnnotations_) {
        if (!WriteIdTaggedValue(writer, FieldTag::TYPE_ANNOTATION, typeAnnotation)) {
            return false;
        }
    }

    return true;
}

bool FieldItem::WriteTaggedData(Writer *writer)
{
    if (!WriteValue(writer)) {
        return false;
    }

    if (!WriteAnnotations(writer)) {
        return false;
    }

    return writer->WriteByte(static_cast<uint8_t>(FieldTag::NOTHING));
}

bool FieldItem::Write(Writer *writer)
{
    if (!BaseFieldItem::Write(writer)) {
        return false;
    }

    return WriteTaggedData(writer);
}

void FieldItem::SetDependencyMark()
{
    BaseFieldItem::SetDependencyMark();
    if (value_ != nullptr) {
        value_->SetDependencyMark();
    }
    for (auto &item : annotations_) {
        item->SetDependencyMark();
    }
    for (auto &item : runtimeAnnotations_) {
        item->SetDependencyMark();
    }
    for (auto &item : typeAnnotations_) {
        item->SetDependencyMark();
    }
    for (auto &item : runtimeTypeAnnotations_) {
        item->SetDependencyMark();
    }
}

size_t AnnotationItem::CalculateSize() const
{
    // class id + count + (name id + value id) * count + tag size * count
    size_t size = IDX_SIZE + sizeof(uint16_t) + (ID_SIZE + ID_SIZE) * elements_.size() + sizeof(uint8_t) * tags_.size();

    return size;
}

bool AnnotationItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());
    ASSERT(class_->HasIndex(this));

    if (!writer->Write<uint16_t>(class_->GetIndex(this))) {
        return false;
    }

    if (!writer->Write(static_cast<uint16_t>(elements_.size()))) {
        return false;
    }

    for (auto elem : elements_) {
        ASSERT(elem.GetName()->GetOffset() != 0);
        if (!writer->Write(elem.GetName()->GetOffset())) {
            return false;
        }

        ValueItem *valueItem = elem.GetValue();

        switch (valueItem->GetType()) {
            case ValueItem::Type::INTEGER: {
                if (!writer->Write(valueItem->GetAsScalar()->GetValue<uint32_t>())) {
                    return false;
                }
                break;
            }
            case ValueItem::Type::FLOAT: {
                if (!writer->Write(bit_cast<uint32_t>(valueItem->GetAsScalar()->GetValue<float>()))) {
                    return false;
                }
                break;
            }
            case ValueItem::Type::ID: {
                if (!writer->Write(valueItem->GetAsScalar()->GetId().GetOffset())) {
                    return false;
                }
                break;
            }
            default: {
                ASSERT(valueItem->GetOffset() != 0);
                if (!writer->Write(valueItem->GetOffset())) {
                    return false;
                }
                break;
            }
        }
    }

    for (auto tag : tags_) {
        if (!writer->Write(tag.GetItem())) {
            return false;
        }
    }

    return true;
}

void AnnotationItem::SetDependencyMark()
{
    BaseItem::SetDependencyMark();
    if (class_ != nullptr) {
        if (!class_->GetDependencyMark()) {
            class_->SetDependencyMark();
        }
    }
    for (auto &it : elements_) {
        it.SetDependencyMark();
    }
}

void LineNumberProgramItemBase::EmitEnd()
{
    EmitOpcode(Opcode::END_SEQUENCE);
}

void LineNumberProgramItemBase::EmitAdvancePc(std::vector<uint8_t> *constantPool, uint32_t value)
{
    EmitOpcode(Opcode::ADVANCE_PC);
    EmitUleb128(constantPool, value);
}

void LineNumberProgramItemBase::EmitAdvanceLine(std::vector<uint8_t> *constantPool, int32_t value)
{
    EmitOpcode(Opcode::ADVANCE_LINE);
    EmitSleb128(constantPool, value);
}

void LineNumberProgramItemBase::EmitColumn(std::vector<uint8_t> *constantPool, uint32_t pcInc, uint32_t column)
{
    if (pcInc != 0U) {
        EmitAdvancePc(constantPool, pcInc);
    }
    EmitOpcode(Opcode::SET_COLUMN);
    EmitUleb128(constantPool, column);
}

void LineNumberProgramItemBase::EmitStartLocal(std::vector<uint8_t> *constantPool, int32_t registerNumber,
                                               StringItem *name, StringItem *type)
{
    EmitStartLocalExtended(constantPool, registerNumber, name, type, nullptr);
}

void LineNumberProgramItemBase::EmitStartLocalExtended(std::vector<uint8_t> *constantPool, int32_t registerNumber,
                                                       StringItem *name, StringItem *type, StringItem *typeSignature)
{
    if (type == nullptr) {
        return;
    }

    ASSERT(name != nullptr);
    ASSERT(name->GetOffset() != 0);
    ASSERT(type->GetOffset() != 0);

    EmitOpcode(typeSignature == nullptr ? Opcode::START_LOCAL : Opcode::START_LOCAL_EXTENDED);
    EmitRegister(registerNumber);
    EmitUleb128(constantPool, name->GetOffset());
    EmitUleb128(constantPool, type->GetOffset());

    if (typeSignature != nullptr) {
        ASSERT(typeSignature->GetOffset() != 0);
        EmitUleb128(constantPool, typeSignature->GetOffset());
    }
}

void LineNumberProgramItemBase::EmitEndLocal(int32_t registerNumber)
{
    EmitOpcode(Opcode::END_LOCAL);
    EmitRegister(registerNumber);
}

void LineNumberProgramItemBase::EmitRestartLocal(int32_t registerNumber)
{
    EmitOpcode(Opcode::RESTART_LOCAL);
    EmitRegister(registerNumber);
}

bool LineNumberProgramItemBase::EmitSpecialOpcode(uint32_t pcInc, int32_t lineInc)
{
    if (lineInc < LINE_BASE || (lineInc - LINE_BASE) >= LINE_RANGE) {
        return false;
    }

    auto opcode = static_cast<size_t>(lineInc - LINE_BASE) + static_cast<size_t>(pcInc * LINE_RANGE) + OPCODE_BASE;
    if (opcode > std::numeric_limits<uint8_t>::max()) {
        return false;
    }

    EmitOpcode(static_cast<uint8_t>(opcode));
    return true;
}

void LineNumberProgramItemBase::EmitPrologueEnd()
{
    EmitOpcode(Opcode::SET_PROLOGUE_END);
}

void LineNumberProgramItemBase::EmitEpilogueBegin()
{
    EmitOpcode(Opcode::SET_EPILOGUE_BEGIN);
}

void LineNumberProgramItemBase::EmitSetFile(std::vector<uint8_t> *constantPool, StringItem *sourceFile)
{
    ASSERT(sourceFile);
    ASSERT(sourceFile->GetOffset() != 0);

    EmitOpcode(Opcode::SET_FILE);
    EmitUleb128(constantPool, sourceFile->GetOffset());
}

void LineNumberProgramItemBase::EmitSetSourceCode(std::vector<uint8_t> *constantPool, StringItem *sourceCode)
{
    ASSERT(sourceCode);
    ASSERT(sourceCode->GetOffset() != 0);

    EmitOpcode(Opcode::SET_SOURCE_CODE);
    EmitUleb128(constantPool, sourceCode->GetOffset());
}

void LineNumberProgramItemBase::EmitOpcode(Opcode opcode)
{
    EmitOpcode(static_cast<uint8_t>(opcode));
}

/* static */
void LineNumberProgramItemBase::EmitUleb128(std::vector<uint8_t> *data, uint32_t value)
{
    ASSERT(data);

    size_t n = leb128::UnsignedEncodingSize(value);
    std::vector<uint8_t> out(n);
    leb128::EncodeUnsigned(value, out.data());
    data->insert(data->end(), out.cbegin(), out.cend());
}

/* static */
void LineNumberProgramItemBase::EmitSleb128(std::vector<uint8_t> *data, int32_t value)
{
    ASSERT(data);

    size_t n = leb128::SignedEncodingSize(value);
    std::vector<uint8_t> out(n);
    leb128::EncodeSigned(value, out.data());
    data->insert(data->end(), out.cbegin(), out.cend());
}

void LineNumberProgramItem::EmitOpcode(uint8_t opcode)
{
    data_.push_back(opcode);
}

void LineNumberProgramItem::EmitRegister(int32_t registerNumber)
{
    EmitSleb128(&data_, registerNumber);
}

size_t LineNumberProgramItem::CalculateSize() const
{
    return data_.size();
}

bool LineNumberProgramItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    return writer->WriteBytes(data_);
}

void LineNumberProgramItem::SetData(std::vector<uint8_t> &&data)
{
    data_ = std::move(data);
}

size_t DebugInfoItem::CalculateSize() const
{
    size_t n = leb128::UnsignedEncodingSize(lineNum_) + leb128::UnsignedEncodingSize(parameters_.size());

    for (auto *p : parameters_) {
        ASSERT(p == nullptr || p->GetOffset() != 0);
        n += leb128::UnsignedEncodingSize(p == nullptr ? 0 : p->GetOffset());
    }

    n += leb128::UnsignedEncodingSize(constantPool_.size());
    n += constantPool_.size();

    n += leb128::UnsignedEncodingSize(program_->GetIndex(this));

    return n;
}

bool DebugInfoItem::Write(Writer *writer)
{
    ASSERT(writer != nullptr);
    ASSERT(GetOffset() == writer->GetOffset());

    if (!writer->WriteUleb128(lineNum_)) {
        return false;
    }

    if (!writer->WriteUleb128(parameters_.size())) {
        return false;
    }

    for (auto *p : parameters_) {
        ASSERT(p == nullptr || p->GetOffset() != 0);

        if (!writer->WriteUleb128(p == nullptr ? 0 : p->GetOffset())) {
            return false;
        }
    }

    if (!writer->WriteUleb128(constantPool_.size())) {
        return false;
    }

    if (!writer->WriteBytes(constantPool_)) {
        return false;
    }

    ASSERT(program_ != nullptr);
    ASSERT(program_->HasIndex(this));

    return writer->WriteUleb128(program_->GetIndex(this));
}

void DebugInfoItem::Dump(std::ostream &os) const
{
    os << "line_start = " << lineNum_ << std::endl;

    os << "num_parameters = " << parameters_.size() << std::endl;
    for (auto *item : parameters_) {
        if (item != nullptr) {
            os << "  string_item[" << item->GetOffset() << "]" << std::endl;
        } else {
            os << "  string_item[INVALID_OFFSET]" << std::endl;
        }
    }

    os << "constant_pool = [";
    for (size_t i = 0; i < constantPool_.size(); i++) {
        size_t b = constantPool_[i];
        os << "0x" << std::setfill('0') << std::setw(2U) << std::right << std::hex << b << std::dec;
        if (i < constantPool_.size() - 1) {
            os << ", ";
        }
    }
    os << "]" << std::endl;

    os << "line_number_program = line_number_program_idx[";
    if (program_ != nullptr && program_->HasIndex(this)) {
        os << program_->GetIndex(this);
    } else {
        os << "NO_INDEX";
    }
    os << "]";
}

bool MethodHandleItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    if (!writer->WriteByte(static_cast<uint8_t>(type_))) {
        return false;
    }

    return writer->WriteUleb128(entity_->GetOffset());
}

}  // namespace ark::panda_file
