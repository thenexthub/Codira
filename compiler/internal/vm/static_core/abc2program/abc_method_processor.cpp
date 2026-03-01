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

#include "abc_method_processor.h"
#include "abc_code_processor.h"
#include "abc_annotation_processor.h"
#include "assembly-function.h"
#include "mangling.h"
#include <libarkfile/include/source_lang_enum.h>
#include "libarkfile/proto_data_accessor-inl.h"

namespace ark::abc2program {

AbcMethodProcessor::AbcMethodProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData)
    : AbcFileEntityProcessor(entityId, keyData), function_(pandasm::Function("", keyData.GetFileLanguage()))
{
    methodDataAccessor_ = std::make_unique<panda_file::MethodDataAccessor>(*file_, entityId_);
    FillProgramData();
}

void AbcMethodProcessor::FillProgramData()
{
    FillFunction();
}

void AbcMethodProcessor::FillFunction()
{
    GetMethodName();

    methodSignature_ = pandasm::MangleFunctionName(function_.name, function_.params, function_.returnType);
    if (keyData_.GetMethodNameToIdTable().find(methodSignature_) != keyData_.GetMethodNameToIdTable().end()) {
        return;
    }

    keyData_.GetMethodNameToIdTable().emplace(methodSignature_, entityId_);

    GetMethodCode();

    program_->AddToFunctionTable(std::move(function_), methodSignature_);
}

void AbcMethodProcessor::GetMethodName()
{
    function_.name = keyData_.GetFullFunctionNameById(entityId_);

    LOG(DEBUG, ABC2PROGRAM) << "name: " << function_.name;

    GetParams(methodDataAccessor_->GetProtoId());
    GetMetaData();
    methodDataAccessor_->EnumerateAnnotations([&](panda_file::File::EntityId annotationId) {
        AbcAnnotationProcessor annotationProcessor(annotationId, keyData_, function_);
        annotationProcessor.FillProgramData();
    });
}

void AbcMethodProcessor::GetMethodCode()
{
    if (!function_.HasImplementation()) {
        return;
    }

    std::optional<panda_file::File::EntityId> codeId = methodDataAccessor_->GetCodeId();
    if (codeId.has_value()) {
        AbcCodeProcessor codeProcessor(codeId.value(), keyData_, entityId_);
        function_.ins = codeProcessor.GetIns();
        function_.regsNum = codeProcessor.GetNumVregs();
        function_.catchBlocks = codeProcessor.GetCatchBlocks();
    } else {
        LOG(ERROR, ABC2PROGRAM) << "> error encountered at " << entityId_ << " (0x" << std::hex << entityId_
                                << "). implementation of method expected, but no \'CODE\' tag was found!";

        return;
    }
}

void AbcMethodProcessor::GetMetaData()
{
    LOG(DEBUG, ABC2PROGRAM) << "[getting metadata]\nmethod id: " << entityId_ << " (0x" << std::hex << entityId_ << ")";

    const auto methodNameRaw = stringTable_->GetStringById((methodDataAccessor_->GetNameId()));

    if (!methodDataAccessor_->IsStatic()) {
        auto className = stringTable_->GetStringById((methodDataAccessor_->GetClassId()));
        std::replace(className.begin(), className.end(), '/', '.');
        auto thisType = pandasm::Type::FromDescriptor(className);

        LOG(DEBUG, ABC2PROGRAM) << "method (raw: \'" << methodNameRaw
                                << "\') is not static. emplacing self-argument of type " << thisType.GetName();

        function_.params.emplace(function_.params.begin(), thisType, keyData_.GetFileLanguage());
    }
    SetEntityAttribute(
        function_, [this]() { return methodDataAccessor_->IsStatic(); }, "static");

    SetEntityAttribute(
        function_, [this]() { return file_->IsExternal(methodDataAccessor_->GetMethodId()); }, "external");

    SetEntityAttribute(
        function_, [this]() { return methodDataAccessor_->IsNative(); }, "native");

    SetEntityAttribute(
        function_, [this]() { return methodDataAccessor_->IsAbstract(); }, "noimpl");

    SetEntityAttributeValue(
        function_, [this]() { return methodDataAccessor_->IsPublic(); }, "access.function", "public");

    SetEntityAttributeValue(
        function_, [this]() { return methodDataAccessor_->IsProtected(); }, "access.function", "protected");

    SetEntityAttributeValue(
        function_, [this]() { return methodDataAccessor_->IsPrivate(); }, "access.function", "private");

    SetEntityAttribute(
        function_, [this]() { return methodDataAccessor_->IsFinal(); }, "final");

    std::string ctorName = ark::panda_file::GetCtorName(keyData_.GetFileLanguage());
    std::string cctorName = ark::panda_file::GetCctorName(keyData_.GetFileLanguage());

    const bool isCtor = (methodNameRaw == ctorName);
    const bool isCctor = (methodNameRaw == cctorName);

    if (isCtor) {
        function_.metadata->SetAttribute("ctor");
        function_.name.replace(function_.name.find(ctorName), ctorName.length(), "_ctor_");
    } else if (isCctor) {
        function_.metadata->SetAttribute("cctor");
        function_.name.replace(function_.name.find(cctorName), cctorName.length(), "_cctor_");
    }
}

void AbcMethodProcessor::GetParams(const panda_file::File::EntityId &protoId)
{
    /// frame size - 2^16 - 1
    static const uint32_t MAX_ARG_NUM = 0xFFFF;

    LOG(DEBUG, ABC2PROGRAM) << "[getting params]\nproto id: " << protoId << " (0x" << std::hex << protoId << ")";

    panda_file::ProtoDataAccessor protoAccessor(*file_, protoId);

    if (protoAccessor.GetNumArgs() > MAX_ARG_NUM) {
        LOG(ERROR, ABC2PROGRAM) << "> error encountered at " << protoId << " (0x" << std::hex << protoId
                                << "). number of function's arguments (" << std::dec << protoAccessor.GetNumArgs()
                                << ") exceeds MAX_ARG_NUM (" << MAX_ARG_NUM << ") !";

        return;
    }

    size_t refIdx = 0;
    function_.returnType = PFTypeToPandasmType(protoAccessor.GetReturnType(), protoAccessor, refIdx);

    for (size_t i = 0; i < protoAccessor.GetNumArgs(); i++) {
        auto argType = PFTypeToPandasmType(protoAccessor.GetArgType(i), protoAccessor, refIdx);
        function_.params.emplace_back(argType, keyData_.GetFileLanguage());
    }
}

pandasm::Type AbcMethodProcessor::PFTypeToPandasmType(const panda_file::Type &type, panda_file::ProtoDataAccessor &pda,
                                                      size_t &refIdx) const
{
    pandasm::Type pandasmType;

    static const std::unordered_map<panda_file::Type::TypeId, std::string> ID_TO_STRING = {
        {panda_file::Type::TypeId::INVALID, "invalid"}, {panda_file::Type::TypeId::VOID, "void"},
        {panda_file::Type::TypeId::U1, "u1"},           {panda_file::Type::TypeId::I8, "i8"},
        {panda_file::Type::TypeId::U8, "u8"},           {panda_file::Type::TypeId::I16, "i16"},
        {panda_file::Type::TypeId::U16, "u16"},         {panda_file::Type::TypeId::I32, "i32"},
        {panda_file::Type::TypeId::U32, "u32"},         {panda_file::Type::TypeId::F32, "f32"},
        {panda_file::Type::TypeId::F64, "f64"},         {panda_file::Type::TypeId::I64, "i64"},
        {panda_file::Type::TypeId::U64, "u64"},         {panda_file::Type::TypeId::TAGGED, "any"},
    };

    if (type.GetId() != panda_file::Type::TypeId::REFERENCE) {
        pandasmType = pandasm::Type(ID_TO_STRING.at(type.GetId()), 0);
    } else {
        std::string typeName = stringTable_->GetStringById(pda.GetReferenceType(refIdx++));
        std::replace(typeName.begin(), typeName.end(), '/', '.');
        pandasmType = pandasm::Type::FromDescriptor(typeName);
    }

    return pandasmType;
}

std::string AbcMethodProcessor::GetMethodSignature()
{
    return methodSignature_;
}

}  // namespace ark::abc2program
