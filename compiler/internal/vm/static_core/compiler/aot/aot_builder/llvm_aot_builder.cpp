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

#include "llvm_aot_builder.h"

namespace ark::compiler {

std::unordered_map<std::string, size_t> LLVMAotBuilder::GetSectionsAddresses(const std::string &cmdline,
                                                                             const std::string &fileName)
{
    switch (arch_) {
        case Arch::AARCH32:
            return GetSectionsAddressesImpl<Arch::AARCH32>(cmdline, fileName);
        case Arch::AARCH64:
            return GetSectionsAddressesImpl<Arch::AARCH64>(cmdline, fileName);
        case Arch::X86:
            return GetSectionsAddressesImpl<Arch::X86>(cmdline, fileName);
        case Arch::X86_64:
            return GetSectionsAddressesImpl<Arch::X86_64>(cmdline, fileName);
        default:
            LOG(ERROR, COMPILER) << "LLVMAotBuilder: Unsupported arch";
            UNREACHABLE();
    }
}

template <Arch ARCH>
std::unordered_map<std::string, size_t> LLVMAotBuilder::GetSectionsAddressesImpl(const std::string &cmdline,
                                                                                 const std::string &fileName)
{
    ElfBuilder<ARCH> builder;
    // stringTable_ is the only field modified by PrepareElfBuilder not idempotently
    auto oldStringTableSize = stringTable_.size();

    PrepareElfBuilder(builder, cmdline, fileName);
    builder.Build(fileName);

    stringTable_.resize(oldStringTableSize);

    auto textSection = builder.GetTextSection();
    auto roDataSections = builder.GetRoDataSections();
    auto aotSection = builder.GetAotSection();
    auto gotSection = builder.GetGotSection();

    static constexpr auto FIRST_ENTRYPOINT_OFFSET =
        static_cast<int32_t>(RuntimeInterface::IntrinsicId::COUNT) * PointerSize(ARCH);

    std::unordered_map<std::string, size_t> sectionAddresses {
        {textSection->GetName(), textSection->GetOffset()},
        {aotSection->GetName(), aotSection->GetOffset()},
        // At runtime the .text section is placed right after the .aot_got section
        // without any padding, so we must use the first entrypoint address as a start
        // of the .aot_got section.
        {gotSection->GetName(), textSection->GetOffset() - FIRST_ENTRYPOINT_OFFSET}};
    for (auto section : *roDataSections) {
        sectionAddresses.emplace(section->GetName(), section->GetOffset());
    }
    return sectionAddresses;
}

}  // namespace ark::compiler
