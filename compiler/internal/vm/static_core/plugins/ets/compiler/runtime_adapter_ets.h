/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_ETS_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H
#define PANDA_ETS_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H

#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "bytecode_optimizer/runtime_adapter.h"

namespace ark {

class EtsBytecodeOptimizerRuntimeAdapter : public BytecodeOptimizerRuntimeAdapter {
public:
    explicit EtsBytecodeOptimizerRuntimeAdapter(const panda_file::File &pandaFile)
        : BytecodeOptimizerRuntimeAdapter(pandaFile)
    {
    }
    ~EtsBytecodeOptimizerRuntimeAdapter() override = default;
    NO_COPY_SEMANTIC(EtsBytecodeOptimizerRuntimeAdapter);
    NO_MOVE_SEMANTIC(EtsBytecodeOptimizerRuntimeAdapter);

    bool IsEtsConstructor(MethodPtr method) const
    {
        const auto methodName = GetMethodName(method);
        return (methodName == ets::panda_file_items::CTOR || methodName == ets::panda_file_items::CCTOR ||
                methodName == ets::panda_file_items::DOT_CTOR || methodName == ets::panda_file_items::DOT_CCTOR);
    }

    std::string GetSignature(MethodPtr method) const
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));
        panda_file::ProtoDataAccessor pda(pandaFile_, mda.GetProtoId());

        std::ostringstream signature;
        signature << '(';

        auto methodId = GetMethodId(method);
        auto argCount = GetMethodArgumentsCount(method, methodId);
        auto returnType = pda.GetReturnType();

        for (size_t i = 0; i < argCount; i++) {
            auto argType = pda.GetArgType(i);
            if (argType.IsPrimitive()) {
                signature << panda_file::Type::GetSignatureByTypeId(argType);
            } else {
                size_t argOffset = returnType.IsPrimitive() ? 0 : 1;
                auto id = pda.GetReferenceType(i + argOffset);
                auto refType = pandaFile_.GetStringData(id);
                signature << reinterpret_cast<const char *>(refType.data);
            }
        }
        signature << ')';

        if (returnType.IsPrimitive()) {
            signature << panda_file::Type::GetSignatureByTypeId(returnType);
        } else {
            auto classId = pda.GetReferenceType(0);
            auto type = pandaFile_.GetStringData(classId);
            signature << reinterpret_cast<const char *>(type.data);
        }

        return signature.str();
    }

    bool IsMethodStringConcat(MethodPtr method) const override
    {
        return GetMethodFullName(method, false) == ets::panda_file_items::methods::STRING_CONCAT &&
               GetSignature(method) == ets::panda_file_items::signatures::STRING_ARRAY_RET_STRING;
    }

    bool IsMethodStringGetLength(MethodPtr method) const override
    {
        auto methodName = GetMethodFullName(method, false);
        return (methodName == ets::panda_file_items::getters::STRING_GET_LENGTH ||
                methodName == ets::panda_file_items::methods::STRING_GET_LENGTH) &&
               GetSignature(method) == ets::panda_file_items::signatures::RET_INT;
    }

    bool IsMethodStringBuilderConstructorWithStringArg(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));
        return IsEtsConstructor(method) &&
               GetClassNameFromMethod(method) == ets::panda_file_items::classes::STRING_BUILDER &&
               GetSignature(method) == ets::panda_file_items::signatures::STRING_RET_VOID;
    }

    bool IsMethodStringBuilderConstructorWithCharArrayArg(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));
        return IsEtsConstructor(method) &&
               GetClassNameFromMethod(method) == ets::panda_file_items::classes::STRING_BUILDER &&
               GetSignature(method) == ets::panda_file_items::signatures::CHAR_ARRAY_RET_VOID;
    }

    bool IsMethodStringBuilderDefaultConstructor(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));
        return IsEtsConstructor(method) &&
               GetClassNameFromMethod(method) == ets::panda_file_items::classes::STRING_BUILDER &&
               GetSignature(method) == ets::panda_file_items::signatures::RET_VOID;
    }

    bool IsMethodStringBuilderToString(MethodPtr method) const override
    {
        return GetMethodFullName(method, false) == ets::panda_file_items::methods::STRING_BUILDER_TO_STRING &&
               GetSignature(method) == ets::panda_file_items::signatures::RET_STRING;
    }

    bool IsMethodStringBuilderAppend(MethodPtr method) const override
    {
        return GetMethodFullName(method, false) == ets::panda_file_items::methods::STRING_BUILDER_APPEND;
    }

    bool IsClassStringBuilder([[maybe_unused]] ClassPtr klass) const override
    {
        return GetClassName(klass) == ets::panda_file_items::classes::STRING_BUILDER;
    }

    bool IsMethodStringBuilderGetStringLength([[maybe_unused]] MethodPtr method) const override
    {
        return GetMethodFullName(method, false) == ets::panda_file_items::getters::STRING_BUILDER_GET_STRING_LENGTH;
    }

    bool IsIntrinsicStringBuilderToString([[maybe_unused]] IntrinsicId id) const override
    {
        return id == RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_TO_STRING;
    }

    bool IsIntrinsicStringConcat([[maybe_unused]] IntrinsicId id) const override
    {
        switch (id) {
            case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT2:
                return true;
            case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT3:
                return true;
            case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT4:
                return true;
            default:
                return false;
        }
    }

    bool IsIntrinsicStringBuilderAppendString([[maybe_unused]] IntrinsicId id) const override
    {
        switch (id) {
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING2:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING3:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING4:
                return true;
            default:
                return false;
        }
    }

    bool IsIntrinsicStringBuilderAppend([[maybe_unused]] IntrinsicId id) const override
    {
        switch (id) {
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_FLOAT:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_DOUBLE:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_LONG:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_INT:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_BYTE:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_SHORT:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_CHAR:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_BOOL:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING2:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING3:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING4:
                return true;
            default:
                return false;
        }
    }

    IntrinsicId GetStringConcatStringsIntrinsicId([[maybe_unused]] size_t numArgs) const override
    {
        // NOLINTBEGIN(readability-magic-numbers)
        switch (numArgs) {
            case 2U:
                return IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT2;
            case 3U:
                return IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT3;
            case 4U:
                return IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT4;
            default:
                UNREACHABLE();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    IntrinsicId GetStringBuilderAppendStringsIntrinsicId([[maybe_unused]] size_t numArgs) const override
    {
        // NOLINTBEGIN(readability-magic-numbers)
        switch (numArgs) {
            case 1U:
                return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING;
            case 2U:
                return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING2;
            case 3U:
                return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING3;
            case 4U:
                return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING4;
            default:
                UNREACHABLE();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    IntrinsicId GetCompilerNullcheckIntrinsicId() const override
    {
        return IntrinsicId::INTRINSIC_COMPILER_ETS_NULLCHECK;
    }

    ClassPtr GetStringBuilderClass() const override
    {
        const uint8_t *mutf8Name = utf::CStringAsMutf8(ets::panda_file_items::class_descriptors::STRING_BUILDER.data());
        auto classId = pandaFile_.GetClassId(mutf8Name);
        return reinterpret_cast<ClassPtr>(classId.GetOffset());
    }

    FieldPtr GetFieldStringBuilderLength([[maybe_unused]] ClassPtr klass) const override
    {
        ASSERT(IsClassStringBuilder(klass));

        auto classEntityId = ClassCast(klass);
        auto fieldId = panda_file::File::EntityId();

        if (classEntityId.IsValid() && !pandaFile_.IsExternal(classEntityId)) {
            panda_file::ClassDataAccessor cda(pandaFile_, classEntityId);

            const uint8_t *lengthMutf8 = utf::CStringAsMutf8(ets::panda_file_items::fields::LENGTH.data());
            panda_file::File::StringData lengthSD = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(lengthMutf8)),
                                                     lengthMutf8};

            cda.EnumerateFields([&fieldId, &cda, &lengthSD](panda_file::FieldDataAccessor &fieldDataAccessor) {
                // NOLINTNEXTLINE(readability-identifier-naming)
                auto fieldId_ = fieldDataAccessor.GetFieldId();
                auto &pf = cda.GetPandaFile();
                auto stringData = pf.GetStringData(fieldDataAccessor.GetNameId());
                if (lengthSD == stringData) {
                    fieldId = fieldId_;
                    return;
                }
            });

            return reinterpret_cast<FieldPtr>(fieldId.GetOffset());
        }

        return nullptr;
    }

    MethodPtr FindMethodByName([[maybe_unused]] const std::string &methodName) const override
    {
        auto regionHeaders = pandaFile_.GetRegionHeaders();
        for (auto regionHeader : regionHeaders) {
            auto methodIdx = pandaFile_.GetMethodIndex(&regionHeader);

            for (auto methodId : methodIdx) {
                MethodPtr methodPtr = reinterpret_cast<MethodPtr>(methodId.GetOffset());
                if (GetMethodFullName(reinterpret_cast<MethodPtr>(methodId.GetOffset()), false) == methodName) {
                    return methodPtr;
                }
            }
        }

        return nullptr;
    }

    MethodPtr FindMethodByName([[maybe_unused]] std::string const &methodName,
                               [[maybe_unused]] std::string const &methodSignature) const override
    {
        auto regionHeaders = pandaFile_.GetRegionHeaders();
        for (auto regionHeader : regionHeaders) {
            auto methodIdx = pandaFile_.GetMethodIndex(&regionHeader);

            for (auto methodId : methodIdx) {
                MethodPtr methodPtr = reinterpret_cast<MethodPtr>(methodId.GetOffset());
                if (GetMethodFullName(reinterpret_cast<MethodPtr>(methodId.GetOffset()), false) == methodName &&
                    GetSignature(methodPtr) == methodSignature) {
                    return methodPtr;
                }
            }
        }

        return nullptr;
    }

    MethodPtr GetGetterStringBuilderStringLength() const override
    {
        return FindMethodByName("std.core.StringBuilder::%%get-stringLength");
    }

    MethodPtr GetMethodStringBuilderAppendString([[maybe_unused]] ClassPtr klass) const override
    {
        return FindMethodByName("std.core.StringBuilder::append", "(Lstd/core/String;)Lstd/core/StringBuilder;");
    }
};
}  // namespace ark

#endif  // PANDA_ETS_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H
