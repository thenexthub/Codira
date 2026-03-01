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

#ifndef LIBABCKIT_SRC_ADAPTER_DYNAMIC_RUNTIME_ADAPTER_DYNAMIC_H
#define LIBABCKIT_SRC_ADAPTER_DYNAMIC_RUNTIME_ADAPTER_DYNAMIC_H

#include "compiler/optimizer/ir/runtime_interface.h"
#include "libabckit/src/wrappers/abcfile_wrapper.h"

namespace libabckit {

using ark::compiler::RuntimeInterface;

class AbckitRuntimeAdapterDynamic : public RuntimeInterface {
public:
    explicit AbckitRuntimeAdapterDynamic(const FileWrapper &abcFile) : abcFile_(abcFile) {}

    BinaryFilePtr GetBinaryFileForMethod([[maybe_unused]] MethodPtr method) const override
    {
        return const_cast<FileWrapper *>(&abcFile_);
    }

    uint32_t ResolveMethodIndex(MethodPtr parentMethod, uint16_t index) const override
    {
        return abcFile_.ResolveOffsetByIndex(parentMethod, index);
    }

    MethodId GetMethodId(MethodPtr method) const override
    {
        return static_cast<MethodId>(reinterpret_cast<uintptr_t>(method));
    }

    size_t GetMethodTotalArgumentsCount(MethodPtr method) const override
    {
        return abcFile_.GetMethodTotalArgumentsCount(method);
    }

    size_t GetMethodArgumentsCount([[maybe_unused]] MethodPtr caller, MethodId id) const override
    {
        return abcFile_.GetMethodArgumentsCount(caller, id);
    }

    size_t GetMethodRegistersCount(MethodPtr method) const override
    {
        return abcFile_.GetMethodRegistersCount(method);
    }

    const uint8_t *GetMethodCode(MethodPtr method) const override
    {
        return abcFile_.GetMethodCode(method);
    }

    size_t GetMethodCodeSize(MethodPtr method) const override
    {
        return abcFile_.GetMethodCodeSize(method);
    }

    ark::SourceLanguage GetMethodSourceLanguage(MethodPtr method) const override
    {
        return static_cast<ark::SourceLanguage>(abcFile_.GetMethodSourceLanguage(method));
    }

    uint32_t GetClassIdForMethod(MethodPtr method) const override
    {
        return abcFile_.GetClassIdForMethod(method);
    }

    std::string GetClassNameFromMethod(MethodPtr method) const override
    {
        return abcFile_.GetClassNameFromMethod(method);
    }

    std::string GetMethodName(MethodPtr method) const override
    {
        return abcFile_.GetMethodName(method);
    }

    std::string GetMethodFullName(MethodPtr method, bool /* with_signature */) const override
    {
        auto className = GetClassNameFromMethod(method);
        auto methodName = GetMethodName(method);

        return className + "::" + methodName;
    }

private:
    const FileWrapper &abcFile_;
};

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_ADAPTER_DYNAMIC_RUNTIME_ADAPTER_DYNAMIC_H
