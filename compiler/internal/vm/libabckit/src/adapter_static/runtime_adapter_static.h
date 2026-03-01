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

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_RUNTIME_ADAPTER_STATIC_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_RUNTIME_ADAPTER_STATIC_H

#include "static_core/compiler/optimizer/ir/runtime_interface.h"
#include "static_core/libarkfile/bytecode_instruction.h"
#include "static_core/libarkfile/class_data_accessor-inl.h"
#include "static_core/libarkfile/code_data_accessor.h"
#include "static_core/libarkfile/field_data_accessor.h"
#include "static_core/libarkfile/file.h"
#include "static_core/libarkfile/file_items.h"
#include "static_core/libarkfile/method_data_accessor.h"
#include "static_core/libarkfile/proto_data_accessor.h"
#include "static_core/libarkfile/proto_data_accessor-inl.h"
#include "static_core/libarkfile/type_helper.h"

namespace libabckit {
using ark::compiler::RuntimeInterface;

class AbckitRuntimeAdapterStatic : public RuntimeInterface {
public:
    explicit AbckitRuntimeAdapterStatic(const ark::panda_file::File &abcFile) : abcFile_(abcFile) {}
    ~AbckitRuntimeAdapterStatic() override = default;
    NO_COPY_SEMANTIC(AbckitRuntimeAdapterStatic);
    NO_MOVE_SEMANTIC(AbckitRuntimeAdapterStatic);

    BinaryFilePtr GetBinaryFileForMethod([[maybe_unused]] MethodPtr method) const override
    {
        return const_cast<ark::panda_file::File *>(&abcFile_);
    }

    MethodId ResolveMethodIndex(MethodPtr parentMethod, MethodIndex index) const override
    {
        return abcFile_.ResolveMethodIndex(MethodCast(parentMethod), index).GetOffset();
    }

    FieldId ResolveFieldIndex(MethodPtr parentMethod, FieldIndex index) const override
    {
        return abcFile_.ResolveFieldIndex(MethodCast(parentMethod), index).GetOffset();
    }

    IdType ResolveTypeIndex(MethodPtr parentMethod, TypeIndex index) const override
    {
        return abcFile_.ResolveClassIndex(MethodCast(parentMethod), index).GetOffset();
    }

    MethodPtr GetMethodById([[maybe_unused]] MethodPtr caller, MethodId id) const override
    {
        return reinterpret_cast<MethodPtr>(id);
    }

    MethodId GetMethodId(MethodPtr method) const override
    {
        return static_cast<MethodId>(reinterpret_cast<uintptr_t>(method));
    }

    ark::compiler::DataType::Type GetMethodReturnType(MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));
        ark::panda_file::ProtoDataAccessor pda(abcFile_, mda.GetProtoId());

        return ToCompilerType(ark::panda_file::GetEffectiveType(pda.GetReturnType()));
    }

    IdType GetMethodReturnTypeId(MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));
        ark::panda_file::ProtoDataAccessor pda(abcFile_, mda.GetProtoId());

        return pda.GetReferenceType(0).GetOffset();
    }

    ark::compiler::DataType::Type GetMethodTotalArgumentType(MethodPtr method, size_t index) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));

        if (!mda.IsStatic()) {
            if (index == 0) {
                return ToCompilerType(
                    ark::panda_file::GetEffectiveType(ark::panda_file::Type(ark::panda_file::Type::TypeId::REFERENCE)));
            }
            --index;
        }

        ark::panda_file::ProtoDataAccessor pda(abcFile_, mda.GetProtoId());
        return ToCompilerType(ark::panda_file::GetEffectiveType(pda.GetArgType(index)));
    }

    ark::compiler::DataType::Type GetMethodArgumentType([[maybe_unused]] MethodPtr caller, MethodId id,
                                                        size_t index) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, ark::panda_file::File::EntityId(id));
        ark::panda_file::ProtoDataAccessor pda(abcFile_, mda.GetProtoId());

        return ToCompilerType(ark::panda_file::GetEffectiveType(pda.GetArgType(index)));
    }

    size_t GetMethodTotalArgumentsCount(MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());
        ark::panda_file::CodeDataAccessor cda(abcFile_, mda.GetCodeId().value());

        return cda.GetNumArgs();
    }

    size_t GetMethodArgumentsCount([[maybe_unused]] MethodPtr caller, MethodId id) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, ark::panda_file::File::EntityId(id));
        ark::panda_file::ProtoDataAccessor pda(abcFile_, mda.GetProtoId());

        return pda.GetNumArgs();
    }

    ark::compiler::DataType::Type GetMethodReturnType(MethodPtr caller, MethodId id) const override
    {
        return GetMethodReturnType(GetMethodById(caller, id));
    }

    size_t GetMethodRegistersCount(MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());
        ark::panda_file::CodeDataAccessor cda(abcFile_, mda.GetCodeId().value());

        return cda.GetNumVregs();
    }

    const uint8_t *GetMethodCode(MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());
        ark::panda_file::CodeDataAccessor cda(abcFile_, mda.GetCodeId().value());

        return cda.GetInstructions();
    }

    size_t GetMethodCodeSize(MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());
        ark::panda_file::CodeDataAccessor cda(abcFile_, mda.GetCodeId().value());

        return cda.GetCodeSize();
    }

    ark::SourceLanguage GetMethodSourceLanguage(MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());

        auto sourceLang = mda.GetSourceLang();
        ASSERT(sourceLang.has_value());

        return static_cast<ark::SourceLanguage>(sourceLang.value());
    }

    size_t GetClassIdForField([[maybe_unused]] MethodPtr method, size_t fieldId) const override
    {
        ark::panda_file::FieldDataAccessor fda(abcFile_, ark::panda_file::File::EntityId(fieldId));

        return static_cast<size_t>(fda.GetClassId().GetOffset());
    }

    ClassPtr GetClassForField(FieldPtr field) const override
    {
        ark::panda_file::FieldDataAccessor fda(abcFile_, FieldCast(field));

        return reinterpret_cast<ClassPtr>(fda.GetClassId().GetOffset());
    }

    uint32_t GetClassIdForMethod(MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));

        return static_cast<size_t>(mda.GetClassId().GetOffset());
    }

    uint32_t GetClassIdForMethod([[maybe_unused]] MethodPtr caller, size_t methodId) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, ark::panda_file::File::EntityId(methodId));

        return static_cast<size_t>(mda.GetClassId().GetOffset());
    }

    bool IsMethodExternal([[maybe_unused]] MethodPtr caller, MethodPtr callee) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(callee));

        return mda.IsExternal();
    }

    bool IsMethodIntrinsic([[maybe_unused]] MethodPtr method) const override
    {
        return false;
    }

    bool IsMethodIntrinsic([[maybe_unused]] MethodPtr caller, [[maybe_unused]] MethodId id) const override
    {
        return false;
    }

    IntrinsicId GetIntrinsicId([[maybe_unused]] MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));
        auto className = GetClassNameFromMethod(method);
        auto methodName = GetMethodName(method);
        return GetIntrinsicId(className, methodName, mda);
    }

    bool IsMethodStatic(MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));

        return mda.IsStatic();
    }

    bool IsMethodStatic([[maybe_unused]] MethodPtr caller, MethodId id) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, ark::panda_file::File::EntityId(id));

        return mda.IsStatic();
    }

    // return true if the method is Native with exception
    bool HasNativeException([[maybe_unused]] MethodPtr method) const override
    {
        return false;
    }

    std::string GetClassNameFromMethod(MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));

        return mda.GetClassName();
    }

    std::string GetClassName(ClassPtr cls) const override
    {
        auto stringData = abcFile_.GetStringData(ClassCast(cls));

        return ark::panda_file::ClassDataAccessor::DemangledName(stringData);
    }

    std::string GetMethodName(MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));

        auto stringData = abcFile_.GetStringData(mda.GetNameId());

        return std::string(reinterpret_cast<const char *>(stringData.data));
    }

    bool IsConstructor(MethodPtr method, ark::SourceLanguage lang) override
    {
        return GetMethodName(method) == ark::panda_file::GetCtorName(lang);
    }

    std::string GetMethodFullName(MethodPtr method, bool /* with_signature */) const override
    {
        auto className = GetClassNameFromMethod(method);
        auto methodName = GetMethodName(method);

        return className + "::" + methodName;
    }

    ClassPtr GetClass(MethodPtr method) const override
    {
        ark::panda_file::MethodDataAccessor mda(abcFile_, MethodCast(method));

        return reinterpret_cast<ClassPtr>(mda.GetClassId().GetOffset());
    }

    std::string GetBytecodeString(MethodPtr method, uintptr_t pc) const override
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ark::BytecodeInstruction inst(GetMethodCode(method) + pc);
        std::stringstream ss;

        ss << inst;
        return ss.str();
    }

    bool IsArrayClass([[maybe_unused]] MethodPtr method, IdType id) const override
    {
        ark::panda_file::File::EntityId cid(id);

        return ark::panda_file::IsArrayDescriptor(abcFile_.GetStringData(cid).data);
    }

    FieldPtr ResolveField([[maybe_unused]] MethodPtr method, size_t id, [[maybe_unused]] bool isStatic,
                          [[maybe_unused]] bool allowExternal, uint32_t * /* class_id */) override
    {
        return reinterpret_cast<FieldPtr>(id);
    }

    ark::compiler::DataType::Type GetFieldType(FieldPtr field) const override
    {
        ark::panda_file::FieldDataAccessor fda(abcFile_, FieldCast(field));

        return ToCompilerType(ark::panda_file::Type::GetTypeFromFieldEncoding(fda.GetType()));
    }

    ark::compiler::DataType::Type GetFieldTypeById([[maybe_unused]] MethodPtr parentMethod, IdType id) const override
    {
        ark::panda_file::FieldDataAccessor fda(abcFile_, ark::panda_file::File::EntityId(id));

        return ToCompilerType(ark::panda_file::Type::GetTypeFromFieldEncoding(fda.GetType()));
    }

    IdType GetFieldValueTypeId([[maybe_unused]] MethodPtr method, IdType id) const override
    {
        auto typeId = ark::panda_file::FieldDataAccessor::GetTypeId(abcFile_, ark::panda_file::File::EntityId(id));
        return typeId.GetOffset();
    }

    static ark::panda_file::File::EntityId EnumerateCDAFields(ark::panda_file::File::EntityId &fieldId,
                                                              ark::panda_file::FieldDataAccessor &fda,
                                                              ark::panda_file::FieldDataAccessor &fieldDataAccessor)
    {
        auto &pf = fda.GetPandaFile();
        auto fieldType = ark::panda_file::Type::GetTypeFromFieldEncoding(fda.GetType());
        if (fda.GetType() != fieldDataAccessor.GetType()) {
            return fieldId;
        }

        if (pf.GetStringData(fda.GetNameId()) != pf.GetStringData(fieldDataAccessor.GetNameId())) {
            return fieldId;
        }

        if (fieldType.IsReference()) {
            if (pf.GetStringData(ark::panda_file::File::EntityId(fda.GetType())) !=
                pf.GetStringData(ark::panda_file::File::EntityId(fieldDataAccessor.GetType()))) {
                return fieldId;
            }
        }

        return fieldDataAccessor.GetFieldId();
    }

    bool IsFieldVolatile(FieldPtr field) const override
    {
        ark::panda_file::FieldDataAccessor fda(abcFile_, FieldCast(field));

        if (!fda.IsExternal()) {
            return fda.IsVolatile();
        }

        auto fieldId = ark::panda_file::File::EntityId();

        if (abcFile_.IsExternal(fda.GetClassId())) {
            // If the field is external and class of the field is also external
            // assume that field is volatile
            return true;
        }

        auto classId = ark::panda_file::ClassDataAccessor(abcFile_, fda.GetClassId()).GetSuperClassId();
#ifndef NDEBUG
        auto visitedClasses = std::unordered_set<ark::panda_file::File::EntityId> {classId};
#endif
        while (classId.IsValid() && !abcFile_.IsExternal(classId)) {
            auto cda = ark::panda_file::ClassDataAccessor(abcFile_, classId);
            cda.EnumerateFields([&fieldId, &fda](ark::panda_file::FieldDataAccessor &fieldDataAccessor) {
                fieldId = EnumerateCDAFields(fieldId, fda, fieldDataAccessor);
            });

            classId = cda.GetSuperClassId();
#ifndef NDEBUG
            ASSERT_PRINT(visitedClasses.count(classId) == 0, "Class hierarchy is incorrect");
            visitedClasses.insert(classId);
#endif
        }

        if (!fieldId.IsValid()) {
            // If we cannot find field (for example it's in the class that located in other panda file)
            // assume that field is volatile
            return true;
        }
        ASSERT(fieldId.IsValid());
        ark::panda_file::FieldDataAccessor fieldDa(abcFile_, fieldId);
        return fieldDa.IsVolatile();
    }

    ClassPtr ResolveType([[maybe_unused]] MethodPtr method, size_t id) const override
    {
        return reinterpret_cast<ClassPtr>(id);
    }

    std::string GetFieldName(FieldPtr field) const override
    {
        ark::panda_file::FieldDataAccessor fda(abcFile_, FieldCast(field));
        auto stringData = abcFile_.GetStringData(fda.GetNameId());
        return ark::utf::Mutf8AsCString(stringData.data);
    }

private:
    static ark::compiler::DataType::Type ToCompilerType(ark::panda_file::Type type)
    {
        switch (type.GetId()) {
            case ark::panda_file::Type::TypeId::VOID:
                return ark::compiler::DataType::VOID;
            case ark::panda_file::Type::TypeId::U1:
                return ark::compiler::DataType::BOOL;
            case ark::panda_file::Type::TypeId::I8:
                return ark::compiler::DataType::INT8;
            case ark::panda_file::Type::TypeId::U8:
                return ark::compiler::DataType::UINT8;
            case ark::panda_file::Type::TypeId::I16:
                return ark::compiler::DataType::INT16;
            case ark::panda_file::Type::TypeId::U16:
                return ark::compiler::DataType::UINT16;
            case ark::panda_file::Type::TypeId::I32:
                return ark::compiler::DataType::INT32;
            case ark::panda_file::Type::TypeId::U32:
                return ark::compiler::DataType::UINT32;
            case ark::panda_file::Type::TypeId::I64:
                return ark::compiler::DataType::INT64;
            case ark::panda_file::Type::TypeId::U64:
                return ark::compiler::DataType::UINT64;
            case ark::panda_file::Type::TypeId::F32:
                return ark::compiler::DataType::FLOAT32;
            case ark::panda_file::Type::TypeId::F64:
                return ark::compiler::DataType::FLOAT64;
            case ark::panda_file::Type::TypeId::REFERENCE:
                return ark::compiler::DataType::REFERENCE;
            case ark::panda_file::Type::TypeId::TAGGED:
            case ark::panda_file::Type::TypeId::INVALID:
                return ark::compiler::DataType::ANY;
            default:
                break;
        }
        UNREACHABLE();
    }

    static ark::panda_file::File::EntityId MethodCast(RuntimeInterface::MethodPtr method)
    {
        return ark::panda_file::File::EntityId(reinterpret_cast<uintptr_t>(method));
    }

    static ark::panda_file::File::EntityId ClassCast(RuntimeInterface::ClassPtr cls)
    {
        return ark::panda_file::File::EntityId(reinterpret_cast<uintptr_t>(cls));
    }

    static ark::panda_file::File::EntityId FieldCast(RuntimeInterface::FieldPtr field)
    {
        return ark::panda_file::File::EntityId(reinterpret_cast<uintptr_t>(field));
    }

    static IntrinsicId GetIntrinsicId(std::string_view className, std::string_view methodName,
                                      ark::panda_file::MethodDataAccessor mda);

    static bool IsEqual(ark::panda_file::MethodDataAccessor mda,
                        std::initializer_list<ark::panda_file::Type::TypeId> shorties,
                        std::initializer_list<std::string_view> refTypes);

    const ark::panda_file::File &abcFile_;
};
}  // namespace libabckit

#endif  // LIBABCKIT_SRC_ADAPTER_STATIC_RUNTIME_ADAPTER_STATIC_H
