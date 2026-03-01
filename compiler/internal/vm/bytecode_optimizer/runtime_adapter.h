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

#ifndef BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H
#define BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H

#include "compiler/optimizer/ir/runtime_interface.h"
#include "libpandafile/bytecode_instruction.h"
#include "libpandafile/class_data_accessor.h"
#include "libpandafile/code_data_accessor.h"
#include "libpandafile/field_data_accessor.h"
#include "libpandafile/file.h"
#include "libpandafile/file-inl.h"
#include "libpandafile/file_items.h"
#include "libpandafile/method_data_accessor.h"
#include "libpandafile/proto_data_accessor.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "libpandafile/type_helper.h"

namespace panda {
using compiler::RuntimeInterface;

class BytecodeOptimizerRuntimeAdapter : public RuntimeInterface {
public:
    explicit BytecodeOptimizerRuntimeAdapter(const panda_file::File &panda_file) : panda_file_(panda_file) {}

    ~BytecodeOptimizerRuntimeAdapter() override = default;

    BinaryFilePtr GetBinaryFileForMethod([[maybe_unused]] MethodPtr method) const override
    {
        return const_cast<panda_file::File *>(&panda_file_);
    }

    uint32_t ResolveOffsetByIndex(MethodPtr parent_method, uint16_t index) const override
    {
        return panda_file_.ResolveOffsetByIndex(MethodCast(parent_method), index).GetOffset();
    }

    MethodId GetMethodId(MethodPtr method) const override
    {
        return static_cast<MethodId>(reinterpret_cast<uintptr_t>(method));
    }

    size_t GetMethodTotalArgumentsCount(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal());
        panda_file::CodeDataAccessor cda(panda_file_, mda.GetCodeId().value());

        return cda.GetNumArgs();
    }

    size_t GetMethodArgumentsCount([[maybe_unused]] MethodPtr caller, MethodId id) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, panda_file::File::EntityId(id));

        ASSERT(!mda.IsExternal());
        panda_file::CodeDataAccessor cda(panda_file_, mda.GetCodeId().value());

        return cda.GetNumArgs();
    }

    size_t GetMethodRegistersCount(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal());
        panda_file::CodeDataAccessor cda(panda_file_, mda.GetCodeId().value());

        return cda.GetNumVregs();
    }

    const uint8_t *GetMethodCode(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal());
        panda_file::CodeDataAccessor cda(panda_file_, mda.GetCodeId().value());

        return cda.GetInstructions();
    }

    size_t GetMethodCodeSize(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal());
        panda_file::CodeDataAccessor cda(panda_file_, mda.GetCodeId().value());

        return cda.GetCodeSize();
    }

    compiler::SourceLanguage GetMethodSourceLanguage(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal());

        auto source_lang = mda.GetSourceLang();
        ASSERT(source_lang.has_value());

        return static_cast<compiler::SourceLanguage>(source_lang.value());
    }

    size_t GetClassIdForMethod(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        return static_cast<size_t>(mda.GetClassId().GetOffset());
    }

    std::string GetClassNameFromMethod(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        auto string_data = panda_file_.GetStringData(mda.GetClassId());

        return std::string(reinterpret_cast<const char *>(string_data.data));
    }

    std::string GetMethodName(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        auto string_data = panda_file_.GetStringData(mda.GetNameId());

        return std::string(reinterpret_cast<const char *>(string_data.data));
    }

    std::string GetMethodFullName(MethodPtr method, bool /* with_signature */) const override
    {
        auto class_name = GetClassNameFromMethod(method);
        auto method_name = GetMethodName(method);

        return class_name + "::" + method_name;
    }

private:
    static compiler::DataType::Type ToCompilerType(panda_file::Type type)
    {
        switch (type.GetId()) {
            case panda_file::Type::TypeId::VOID:
                return compiler::DataType::VOID;
            case panda_file::Type::TypeId::I32:
                return compiler::DataType::INT32;
            case panda_file::Type::TypeId::U32:
                return compiler::DataType::UINT32;
            case panda_file::Type::TypeId::I64:
                return compiler::DataType::INT64;
            case panda_file::Type::TypeId::U64:
                return compiler::DataType::UINT64;
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

    const panda_file::File &panda_file_;
};
}  // namespace panda

#endif  // BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H
