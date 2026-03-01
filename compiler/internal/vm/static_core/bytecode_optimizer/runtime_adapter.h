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
#ifndef PANDA_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H
#define PANDA_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H

#include "compiler/optimizer/ir/runtime_interface.h"
#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/code_data_accessor.h"
#include "libarkfile/field_data_accessor.h"
#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "libarkfile/method_data_accessor.h"
#include "libarkfile/proto_data_accessor.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "libarkfile/type_helper.h"

namespace ark {
using compiler::RuntimeInterface;

class BytecodeOptimizerRuntimeAdapter : public RuntimeInterface {
public:
    explicit BytecodeOptimizerRuntimeAdapter(const panda_file::File &pandaFile) : pandaFile_(pandaFile) {}
    ~BytecodeOptimizerRuntimeAdapter() override = default;
    NO_COPY_SEMANTIC(BytecodeOptimizerRuntimeAdapter);
    NO_MOVE_SEMANTIC(BytecodeOptimizerRuntimeAdapter);

    BinaryFilePtr GetBinaryFileForMethod([[maybe_unused]] MethodPtr method) const override
    {
        return const_cast<panda_file::File *>(&pandaFile_);
    }

    MethodId ResolveMethodIndex(MethodPtr parentMethod, MethodIndex index) const override
    {
        return pandaFile_.ResolveMethodIndex(MethodCast(parentMethod), index).GetOffset();
    }

    FieldId ResolveFieldIndex(MethodPtr parentMethod, FieldIndex index) const override
    {
        return pandaFile_.ResolveFieldIndex(MethodCast(parentMethod), index).GetOffset();
    }

    IdType ResolveTypeIndex(MethodPtr parentMethod, TypeIndex index) const override
    {
        return pandaFile_.ResolveClassIndex(MethodCast(parentMethod), index).GetOffset();
    }

    MethodPtr GetMethodById([[maybe_unused]] MethodPtr caller, MethodId id) const override
    {
        return reinterpret_cast<MethodPtr>(id);
    }

    MethodId GetMethodId(MethodPtr method) const override
    {
        return static_cast<MethodId>(reinterpret_cast<uintptr_t>(method));
    }

    compiler::DataType::Type GetMethodReturnType(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));
        panda_file::ProtoDataAccessor pda(pandaFile_, mda.GetProtoId());

        return ToCompilerType(panda_file::GetEffectiveType(pda.GetReturnType()));
    }

    IdType GetMethodReturnTypeId(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));
        panda_file::ProtoDataAccessor pda(pandaFile_, mda.GetProtoId());

        return pda.GetReferenceType(0).GetOffset();
    }

    compiler::DataType::Type GetMethodTotalArgumentType(MethodPtr method, size_t index) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));

        if (!mda.IsStatic()) {
            if (index == 0) {
                return ToCompilerType(
                    panda_file::GetEffectiveType(panda_file::Type(panda_file::Type::TypeId::REFERENCE)));
            }
            --index;
        }

        panda_file::ProtoDataAccessor pda(pandaFile_, mda.GetProtoId());
        return ToCompilerType(panda_file::GetEffectiveType(pda.GetArgType(index)));
    }

    compiler::DataType::Type GetMethodArgumentType([[maybe_unused]] MethodPtr caller, MethodId id,
                                                   size_t index) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, panda_file::File::EntityId(id));
        panda_file::ProtoDataAccessor pda(pandaFile_, mda.GetProtoId());

        return ToCompilerType(panda_file::GetEffectiveType(pda.GetArgType(index)));
    }

    size_t GetMethodTotalArgumentsCount(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());
        panda_file::CodeDataAccessor cda(pandaFile_, mda.GetCodeId().value());

        return cda.GetNumArgs();
    }

    size_t GetMethodArgumentsCount([[maybe_unused]] MethodPtr caller, MethodId id) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, panda_file::File::EntityId(id));
        panda_file::ProtoDataAccessor pda(pandaFile_, mda.GetProtoId());

        return pda.GetNumArgs();
    }

    compiler::DataType::Type GetMethodReturnType(MethodPtr caller, MethodId id) const override
    {
        return GetMethodReturnType(GetMethodById(caller, id));
    }

    size_t GetMethodRegistersCount(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());
        panda_file::CodeDataAccessor cda(pandaFile_, mda.GetCodeId().value());

        return cda.GetNumVregs();
    }

    const uint8_t *GetMethodCode(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());
        panda_file::CodeDataAccessor cda(pandaFile_, mda.GetCodeId().value());

        return cda.GetInstructions();
    }

    size_t GetMethodCodeSize(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());
        panda_file::CodeDataAccessor cda(pandaFile_, mda.GetCodeId().value());

        return cda.GetCodeSize();
    }

    SourceLanguage GetMethodSourceLanguage(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());

        auto sourceLang = mda.GetSourceLang();
        ASSERT(sourceLang.has_value());

        return static_cast<SourceLanguage>(sourceLang.value());
    }

    size_t GetClassIdForField([[maybe_unused]] MethodPtr method, size_t fieldId) const override
    {
        panda_file::FieldDataAccessor fda(pandaFile_, panda_file::File::EntityId(fieldId));

        return static_cast<size_t>(fda.GetClassId().GetOffset());
    }

    ClassPtr GetClassForField(FieldPtr field) const override
    {
        panda_file::FieldDataAccessor fda(pandaFile_, FieldCast(field));

        return reinterpret_cast<ClassPtr>(fda.GetClassId().GetOffset());
    }

    uint32_t GetClassIdForMethod(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));

        return mda.GetClassId().GetOffset();
    }

    uint32_t GetClassIdForMethod([[maybe_unused]] MethodPtr caller, size_t methodId) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, panda_file::File::EntityId(methodId));

        return mda.GetClassId().GetOffset();
    }

    bool IsMethodExternal([[maybe_unused]] MethodPtr caller, MethodPtr callee) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(callee));

        return mda.IsExternal();
    }

    bool IsMethodIntrinsic(MethodPtr method) const override
    {
        return GetIntrinsicId(method) != IntrinsicId::INVALID;
    }

    bool IsMethodIntrinsic(MethodPtr caller, MethodId id) const override
    {
        return GetMethodAsIntrinsic(caller, id) != nullptr;
    }

    MethodPtr GetMethodAsIntrinsic(MethodPtr caller, MethodId id) const override
    {
        auto *method = GetMethodById(caller, id);
        return IsMethodIntrinsic(method) ? method : nullptr;
    }

    IntrinsicId GetIntrinsicId([[maybe_unused]] MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));
        auto className = GetClassNameFromMethod(method);
        auto methodName = GetMethodName(method);
        return GetIntrinsicId(className, methodName, mda);
    }

    bool IsMethodStatic(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));

        return mda.IsStatic();
    }

    bool IsMethodStatic([[maybe_unused]] MethodPtr caller, MethodId id) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, panda_file::File::EntityId(id));

        return mda.IsStatic();
    }

    // return true if the method is Native with exception
    bool HasNativeException([[maybe_unused]] MethodPtr method) const override
    {
        return false;
    }

    std::string GetClassNameFromMethod(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));

        return mda.GetClassName();
    }

    std::string GetClassName(ClassPtr cls) const override
    {
        auto stringData = pandaFile_.GetStringData(ClassCast(cls));

        return panda_file::ClassDataAccessor::DemangledName(stringData);
    }

    std::string GetMethodName(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));

        auto stringData = pandaFile_.GetStringData(mda.GetNameId());

        return std::string(reinterpret_cast<const char *>(stringData.data));
    }

    bool IsConstructor(MethodPtr method, SourceLanguage lang) override
    {
        return GetMethodName(method) == panda_file::GetCtorName(lang);
    }

    std::string GetMethodFullName(MethodPtr method, bool /* with_signature */) const override
    {
        auto className = GetClassNameFromMethod(method);
        auto methodName = GetMethodName(method);

        return className + "::" + methodName;
    }

    ClassPtr GetClass(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));

        return reinterpret_cast<ClassPtr>(mda.GetClassId().GetOffset());
    }

    std::string GetBytecodeString(MethodPtr method, uintptr_t pc) const override
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        BytecodeInstruction inst(GetMethodCode(method) + pc);
        std::stringstream ss;

        ss << inst;
        return ss.str();
    }

    bool IsArrayClass([[maybe_unused]] MethodPtr method, IdType id) const override
    {
        panda_file::File::EntityId cid(id);

        return panda_file::IsArrayDescriptor(pandaFile_.GetStringData(cid).data);
    }

    FieldPtr ResolveField([[maybe_unused]] MethodPtr method, size_t id, [[maybe_unused]] bool isStatic,
                          [[maybe_unused]] bool allowExternal, uint32_t *classId) override
    {
        if (classId != nullptr) {
            *classId = GetClassIdForField(method, id);
        }
        return reinterpret_cast<FieldPtr>(id);
    }

    compiler::DataType::Type GetFieldType(FieldPtr field) const override
    {
        panda_file::FieldDataAccessor fda(pandaFile_, FieldCast(field));

        return ToCompilerType(panda_file::Type::GetTypeFromFieldEncoding(fda.GetType()));
    }

    compiler::DataType::Type GetFieldTypeById([[maybe_unused]] MethodPtr parentMethod, IdType id) const override
    {
        panda_file::FieldDataAccessor fda(pandaFile_, panda_file::File::EntityId(id));

        return ToCompilerType(panda_file::Type::GetTypeFromFieldEncoding(fda.GetType()));
    }

    IdType GetFieldValueTypeId([[maybe_unused]] MethodPtr method, IdType id) const override
    {
        auto typeId = panda_file::FieldDataAccessor::GetTypeId(pandaFile_, panda_file::File::EntityId(id));
        return typeId.GetOffset();
    }

    FieldId GetFieldId([[maybe_unused]] FieldPtr field) const override
    {
        panda_file::FieldDataAccessor fda(pandaFile_, FieldCast(field));

        return fda.GetFieldId().GetOffset();
    }

    bool IsFieldVolatile(FieldPtr field) const override
    {
        panda_file::FieldDataAccessor fda(pandaFile_, FieldCast(field));

        if (!fda.IsExternal()) {
            return fda.IsVolatile();
        }

        auto fieldId = panda_file::File::EntityId();

        if (pandaFile_.IsExternal(fda.GetClassId())) {
            // If the field is external and class of the field is also external
            // assume that field is volatile
            return true;
        }

        auto classId = panda_file::ClassDataAccessor(pandaFile_, fda.GetClassId()).GetSuperClassId();
#ifndef NDEBUG
        auto visitedClasses = std::unordered_set<panda_file::File::EntityId> {classId};
#endif
        while (classId.IsValid() && !pandaFile_.IsExternal(classId)) {
            auto cda = panda_file::ClassDataAccessor(pandaFile_, classId);
            cda.EnumerateFields([&fieldId, &fda](panda_file::FieldDataAccessor &fieldDataAccessor) {
                auto &pf = fda.GetPandaFile();
                auto fieldType = panda_file::Type::GetTypeFromFieldEncoding(fda.GetType());
                if (fda.GetType() != fieldDataAccessor.GetType()) {
                    return;
                }

                if (pf.GetStringData(fda.GetNameId()) != pf.GetStringData(fieldDataAccessor.GetNameId())) {
                    return;
                }

                if (fieldType.IsReference() &&
                    (pf.GetStringData(panda_file::File::EntityId(fda.GetType())) !=
                     pf.GetStringData(panda_file::File::EntityId(fieldDataAccessor.GetType())))) {
                    return;
                }
                fieldId = fieldDataAccessor.GetFieldId();
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
        panda_file::FieldDataAccessor fieldDa(pandaFile_, fieldId);
        return fieldDa.IsVolatile();
    }

    ClassPtr ResolveType([[maybe_unused]] MethodPtr method, size_t id) const override
    {
        return reinterpret_cast<ClassPtr>(id);
    }

    std::string GetFieldName(FieldPtr field) const override
    {
        panda_file::FieldDataAccessor fda(pandaFile_, FieldCast(field));
        auto stringData = pandaFile_.GetStringData(fda.GetNameId());
        return utf::Mutf8AsCString(stringData.data);
    }

protected:
    static compiler::DataType::Type ToCompilerType(panda_file::Type type)
    {
        switch (type.GetId()) {
            case panda_file::Type::TypeId::VOID:
                return compiler::DataType::VOID;
            case panda_file::Type::TypeId::U1:
                return compiler::DataType::BOOL;
            case panda_file::Type::TypeId::I8:
                return compiler::DataType::INT8;
            case panda_file::Type::TypeId::U8:
                return compiler::DataType::UINT8;
            case panda_file::Type::TypeId::I16:
                return compiler::DataType::INT16;
            case panda_file::Type::TypeId::U16:
                return compiler::DataType::UINT16;
            case panda_file::Type::TypeId::I32:
                return compiler::DataType::INT32;
            case panda_file::Type::TypeId::U32:
                return compiler::DataType::UINT32;
            case panda_file::Type::TypeId::I64:
                return compiler::DataType::INT64;
            case panda_file::Type::TypeId::U64:
                return compiler::DataType::UINT64;
            case panda_file::Type::TypeId::F32:
                return compiler::DataType::FLOAT32;
            case panda_file::Type::TypeId::F64:
                return compiler::DataType::FLOAT64;
            case panda_file::Type::TypeId::REFERENCE:
                return compiler::DataType::REFERENCE;
            case panda_file::Type::TypeId::TAGGED:
            case panda_file::Type::TypeId::INVALID:
                return compiler::DataType::ANY;
            default:
                break;
        }
        UNREACHABLE();
    }

    static panda_file::File::EntityId MethodCast(RuntimeInterface::MethodPtr method)
    {
        return panda_file::File::EntityId(reinterpret_cast<uintptr_t>(method));
    }

    static panda_file::File::EntityId ClassCast(RuntimeInterface::ClassPtr cls)
    {
        return panda_file::File::EntityId(reinterpret_cast<uintptr_t>(cls));
    }

    static panda_file::File::EntityId FieldCast(RuntimeInterface::FieldPtr field)
    {
        return panda_file::File::EntityId(reinterpret_cast<uintptr_t>(field));
    }

    static IntrinsicId GetIntrinsicId(std::string_view className, std::string_view methodName,
                                      panda_file::MethodDataAccessor mda);

    const panda_file::File &pandaFile_;  // NOLINT(misc-non-private-member-variables-in-classes)
};
}  // namespace ark

#endif  // PANDA_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H
