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

#ifndef COMPILER_AOT_COMPILED_METHOD_H
#define COMPILER_AOT_COMPILED_METHOD_H

#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/span.h"
#include "compiler/code_info/code_info.h"
#include "compiler/optimizer/code_generator/callconv.h"

#include <cstdint>
#include <vector>

namespace ark {
class Method;
}  // namespace ark

namespace ark::compiler {
class CompiledMethod {
public:
    CompiledMethod(Arch arch, Method *method, size_t index) : arch_(arch), method_(method), index_(index) {}
    NO_COPY_OPERATOR(CompiledMethod);
    DEFAULT_COPY_CTOR(CompiledMethod);
    DEFAULT_MOVE_SEMANTIC(CompiledMethod);
    ~CompiledMethod() = default;

    void SetCode(Span<const uint8_t> data)
    {
        code_.reserve(data.size());
        std::copy(data.begin(), data.end(), std::back_inserter(code_));
    }

    void SetCodeInfo(Span<const uint8_t> data)
    {
        codeInfo_.reserve(data.size());
        std::copy(data.begin(), data.end(), std::back_inserter(codeInfo_));
    }

    Method *GetMethod()
    {
        return method_;
    }

    const Method *GetMethod() const
    {
        return method_;
    }

    Span<const uint8_t> GetCode() const
    {
        return Span(code_);
    }

    Span<const uint8_t> GetCodeInfo() const
    {
        return Span(codeInfo_);
    }

    size_t GetOverallSize() const
    {
        return RoundUp(CodePrefix::STRUCT_SIZE, GetCodeAlignment(arch_)) + RoundUp(code_.size(), CodeInfo::ALIGNMENT) +
               RoundUp(codeInfo_.size(), CodeInfo::SIZE_ALIGNMENT);
    }

    size_t GetIndex() const
    {
        return index_;
    }

#ifdef PANDA_COMPILER_DEBUG_INFO
    const CfiInfo &GetCfiInfo() const
    {
        return cfiInfo_;
    }

    CfiInfo &GetCfiInfo()
    {
        return cfiInfo_;
    }

    void SetCfiInfo(const CfiInfo &cfiInfo)
    {
        cfiInfo_ = cfiInfo;
    }
#endif

private:
    Arch arch_ {RUNTIME_ARCH};
    Method *method_ {nullptr};
    size_t index_ {0};
    std::vector<uint8_t> code_;
    std::vector<uint8_t> codeInfo_;
#ifdef PANDA_COMPILER_DEBUG_INFO
    CfiInfo cfiInfo_;
#endif
};
}  // namespace ark::compiler

#endif  // COMPILER_AOT_COMPILED_METHOD_H
