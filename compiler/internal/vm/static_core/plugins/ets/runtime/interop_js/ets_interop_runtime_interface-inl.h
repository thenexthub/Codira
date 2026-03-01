/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_INTEROP_RUNTIME_INTERFACE_INL_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_INTEROP_RUNTIME_INTERFACE_INL_H

// CC-OFFNXT(G.NAM.01) false positive
static std::pair<IntrinsicId, compiler::DataType::Type> GetInfoForInteropConvert(panda_file::Type::TypeId typeId)
{
    switch (typeId) {
        case panda_file::Type::TypeId::VOID:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_VOID_TO_LOCAL, compiler::DataType::NO_TYPE};
        case panda_file::Type::TypeId::U1:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_U1_TO_LOCAL, compiler::DataType::BOOL};
        case panda_file::Type::TypeId::I8:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_I8_TO_LOCAL, compiler::DataType::INT8};
        case panda_file::Type::TypeId::U8:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_U8_TO_LOCAL, compiler::DataType::UINT8};
        case panda_file::Type::TypeId::I16:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_I16_TO_LOCAL, compiler::DataType::INT16};
        case panda_file::Type::TypeId::U16:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_U16_TO_LOCAL, compiler::DataType::UINT16};
        case panda_file::Type::TypeId::I32:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_I32_TO_LOCAL, compiler::DataType::INT32};
        case panda_file::Type::TypeId::U32:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_U32_TO_LOCAL, compiler::DataType::UINT32};
        case panda_file::Type::TypeId::I64:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_I64_TO_LOCAL, compiler::DataType::INT64};
        case panda_file::Type::TypeId::U64:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_U64_TO_LOCAL, compiler::DataType::UINT64};
        case panda_file::Type::TypeId::F32:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_F32_TO_LOCAL, compiler::DataType::FLOAT32};
        case panda_file::Type::TypeId::F64:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_F64_TO_LOCAL, compiler::DataType::FLOAT64};
        case panda_file::Type::TypeId::REFERENCE:
            return {IntrinsicId::INTRINSIC_COMPILER_CONVERT_REF_TYPE_TO_LOCAL, compiler::DataType::REFERENCE};
        default: {
            UNREACHABLE();
        }
    }
}

void GetInfoForInteropCallArgsConversion(
    MethodPtr methodPtr, uint32_t skipArgs,
    // CC-OFFNXT(G.NAM.01) false positive
    ArenaVector<std::pair<IntrinsicId, compiler::DataType::Type>> *intrinsics) const override
{
    auto method = MethodCast(methodPtr);
    uint32_t numArgs = method->GetNumArgs() - skipArgs;
    panda_file::ShortyIterator it(method->GetShorty());
    for (uint32_t i = 0; i < 1U + skipArgs; i++) {  // 1 for return value
        it.IncrementWithoutCheck();
    }
    for (uint32_t argIdx = 0; argIdx < numArgs; ++argIdx, it.IncrementWithoutCheck()) {
        auto type = *it;
        intrinsics->push_back(GetInfoForInteropConvert(type.GetId()));
    }
}

compiler::RuntimeInterface::IntrinsicId GetInteropRefReturnValueConvertIntrinsic(Method *method) const
{
    auto vm = reinterpret_cast<PandaEtsVM *>(Thread::GetCurrent()->GetVM());
    auto klass = GetClass(method, GetMethodReturnTypeId(method));
    // start fastpath
    if (klass == PlatformTypes(vm)->interopJSValue->GetRuntimeClass()) {
        return IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_JS_VALUE;
    }
    if (klass == PlatformTypes(vm)->coreString->GetRuntimeClass()) {
        return IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_STRING;
    }
    return IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_REF_TYPE;
}

// CC-OFFNXT(G.FMT.07) project code style
std::optional<std::pair<compiler::RuntimeInterface::IntrinsicId, compiler::DataType::Type>>
GetInfoForInteropCallRetValueConversion(MethodPtr methodPtr) const override
{
    auto method = MethodCast(methodPtr);
    auto type = method->GetReturnType();
    switch (type.GetId()) {
        case panda_file::Type::TypeId::VOID:
            return {};
        case panda_file::Type::TypeId::U1:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_U1, compiler::DataType::BOOL}};
        case panda_file::Type::TypeId::I8:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_I8, compiler::DataType::INT8}};
        case panda_file::Type::TypeId::U8:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_U8, compiler::DataType::UINT8}};
        case panda_file::Type::TypeId::I16:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_I16, compiler::DataType::INT16}};
        case panda_file::Type::TypeId::U16:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_U16, compiler::DataType::UINT16}};
        case panda_file::Type::TypeId::I32:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_I32, compiler::DataType::INT32}};
        case panda_file::Type::TypeId::U32:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_U32, compiler::DataType::UINT32}};
        case panda_file::Type::TypeId::I64:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_I64, compiler::DataType::INT64}};
        case panda_file::Type::TypeId::U64:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_U64, compiler::DataType::UINT64}};
        case panda_file::Type::TypeId::F32:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_F32, compiler::DataType::FLOAT32}};
        case panda_file::Type::TypeId::F64:
            return {{IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_F64, compiler::DataType::FLOAT64}};
        case panda_file::Type::TypeId::REFERENCE: {
            return {{GetInteropRefReturnValueConvertIntrinsic(method), compiler::DataType::REFERENCE}};
        }
        default: {
            UNREACHABLE();
        }
    }
    return {};
}

FieldPtr GetInteropConstantPoolOffsetField(ClassPtr klass) const override
{
    auto fields = ClassCast(klass)->GetStaticFields();
    ASSERT(fields.Size() == 1);
    return &fields[0];
}

// returns index of the first element in annotation array having the same value as element `index`
uint32_t GetAnnotationElementUniqueIndex(ClassPtr klass, const char *annotation, uint32_t index) override
{
    auto [it, inserted] = annotationOffsets_.try_emplace(klass);
    if (!inserted) {
        return it->second[index];
    }
    auto pf = ClassCast(klass)->GetPandaFile();
    panda_file::ClassDataAccessor cda(*pf, ClassCast(klass)->GetFileId());

    [[maybe_unused]] auto annotationFound =
        cda.EnumerateAnnotation(annotation, [pf, &indices = it->second](panda_file::AnnotationDataAccessor &ada) {
            for (uint32_t i = 0; i < ada.GetCount(); i++) {
                auto adae = ada.GetElement(i);
                auto *elemName = pf->GetStringData(adae.GetNameId()).data;
                if (!utf::IsEqual(utf::CStringAsMutf8("value"), elemName)) {
                    continue;
                }
                auto arr = adae.GetArrayValue();
                indices.resize(arr.GetCount());
                std::unordered_map<uint32_t, uint32_t> firstIndexByOffset;
                for (uint32_t strIdx = 0; strIdx < indices.size(); strIdx++) {
                    auto strOffset = arr.Get<uint32_t>(strIdx);
                    indices[strIdx] = firstIndexByOffset.try_emplace(strOffset, strIdx).first->second;
                }
                return true;
            }
            UNREACHABLE();
        });
    ASSERT(annotationFound.has_value());
    return it->second[index];
}

private:
std::unordered_map<ClassPtr, std::vector<uint32_t>> annotationOffsets_;

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_INTEROP_RUNTIME_INTERFACE_INL_H
