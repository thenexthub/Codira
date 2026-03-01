/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef COMPILER_AOT_AOT_BUILDER_LLVM_AOT_BUILDER_H
#define COMPILER_AOT_AOT_BUILDER_LLVM_AOT_BUILDER_H

#include "aot_builder.h"

namespace ark::compiler {

class LLVMAotBuilder : public AotBuilder {
public:
    std::unordered_map<std::string, size_t> GetSectionsAddresses(const std::string &cmdline,
                                                                 const std::string &fileName);

    const std::vector<CompiledMethod> &GetMethods()
    {
        return methods_;
    }

    const std::vector<compiler::MethodHeader> &GetMethodHeaders()
    {
        return methodHeaders_;
    }

    /**
     * Put method header for a given method.
     * The method will be patched later (see AdjustMethod below).
     */
    void AddMethodHeader(Method *method, size_t methodIndex)
    {
        auto &methodHeader = methodHeaders_.emplace_back();
        methods_.emplace_back(arch_, method, methodIndex);
        methodHeader.methodId = method->GetFileId().GetOffset();
        methodHeader.codeOffset = UNPATCHED_CODE_OFFSET;
        methodHeader.codeSize = UNPATCHED_CODE_SIZE;
        currentBitmap_->SetBit(methodIndex);
    }

    /// Adjust a method's header according to the supplied method
    void AdjustMethodHeader(const CompiledMethod &method, size_t index)
    {
        MethodHeader &methodHeader = methodHeaders_[index];
        methodHeader.codeOffset = currentCodeSize_;
        methodHeader.codeSize = method.GetOverallSize();
        currentCodeSize_ += methodHeader.codeSize;
        currentCodeSize_ = RoundUp(currentCodeSize_, GetCodeAlignment(arch_));
    }

    /// Adjust a method stored in this aot builder according to the supplied method
    void AdjustMethod(const CompiledMethod &method, size_t index)
    {
        ASSERT(methods_.at(index).GetMethod() == method.GetMethod());
        ASSERT(methodHeaders_.at(index).codeSize == method.GetOverallSize());
        methods_.at(index).SetCode(method.GetCode());
        methods_.at(index).SetCodeInfo(method.GetCodeInfo());
    }

private:
    static constexpr auto UNPATCHED_CODE_OFFSET = std::numeric_limits<decltype(MethodHeader::codeOffset)>::max();
    static constexpr auto UNPATCHED_CODE_SIZE = std::numeric_limits<decltype(MethodHeader::codeSize)>::max();

    template <Arch ARCH>
    std::unordered_map<std::string, size_t> GetSectionsAddressesImpl(const std::string &cmdline,
                                                                     const std::string &fileName);
};
}  // namespace ark::compiler
#endif  // COMPILER_AOT_AOT_BUILDER_LLVM_AOT_BUILDER_H
