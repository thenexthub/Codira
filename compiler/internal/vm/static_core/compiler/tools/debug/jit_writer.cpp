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

#include "jit_writer.h"

#ifdef PANDA_COMPILER_DEBUG_INFO

namespace ark::compiler {
class JitCodeDataProvider : public ElfSectionDataProvider {
public:
    explicit JitCodeDataProvider(JitDebugWriter *jitDebugWriter) : jitDebugWriter_(jitDebugWriter) {}

    void FillData(Span<uint8_t> stream, size_t streamBegin) const override
    {
        const size_t codeOffset = CodeInfo::GetCodeOffset(jitDebugWriter_->GetArch());
        CodePrefix prefix;
        size_t currPos;
        for (size_t i = 0; i < jitDebugWriter_->methods_.size(); i++) {
            auto &method = jitDebugWriter_->methods_[i];
            auto &methodHeader = jitDebugWriter_->methodHeaders_[i];
            prefix.codeSize = method.GetCode().size();
            prefix.codeInfoOffset = codeOffset + RoundUp(method.GetCode().size(), CodeInfo::ALIGNMENT);
            prefix.codeInfoSize = method.GetCodeInfo().size();
            // Prefix
            currPos = streamBegin + methodHeader.codeOffset;
            const char *data = reinterpret_cast<char *>(&prefix);
            CopyToSpan(stream, data, sizeof(prefix), currPos);
            currPos += sizeof(prefix);

            // Code
            currPos += codeOffset - sizeof(prefix);
            data = reinterpret_cast<const char *>(method.GetCode().data());
            CopyToSpan(stream, data, method.GetCode().size(), currPos);
            currPos += method.GetCode().size();

            // CodeInfo
            currPos += RoundUp(method.GetCode().size(), CodeInfo::ALIGNMENT) - method.GetCode().size();
            data = reinterpret_cast<const char *>(method.GetCodeInfo().data());
            CopyToSpan(stream, data, method.GetCodeInfo().size(), currPos);
        }
    }

    size_t GetDataSize() const override
    {
        return jitDebugWriter_->currentCodeSize_;
    }

private:
    JitDebugWriter *jitDebugWriter_;
};

void JitDebugWriter::Start()
{
    auto &fileHeader = fileHeaders_.emplace_back();
    fileHeader.classesOffset = classHeaders_.size();
    fileHeader.fileChecksum = 0;
    fileHeader.fileOffset = 0;
    fileHeader.fileNameStr = AddString("jit_code");
    fileHeader.methodsOffset = methodHeaders_.size();
}

void JitDebugWriter::End()
{
    ASSERT(!fileHeaders_.empty());
    auto &fileHeader = fileHeaders_.back();
    fileHeader.classesCount = classHeaders_.size() - fileHeader.classesOffset;
    if (fileHeader.classesCount == 0) {
        fileHeaders_.pop_back();
        return;
    }
    fileHeader.methodsCount = methodHeaders_.size() - fileHeader.methodsOffset;
    // We should keep class headers sorted, since AOT manager uses binary search to find classes.
    std::sort(classHeaders_.begin() + fileHeader.classesOffset, classHeaders_.end(),
              [](const auto &a, const auto &b) { return a.classId < b.classId; });
}

bool JitDebugWriter::Write()
{
    switch (GetArch()) {
#ifdef PANDA_TARGET_64
        case Arch::AARCH64:
            return WriteImpl<Arch::AARCH64>();
        case Arch::X86_64:
            return WriteImpl<Arch::X86_64>();
#endif
        case Arch::AARCH32:
            return WriteImpl<Arch::AARCH32>();
        default:
            LOG(ERROR, COMPILER) << "JitDebug: Unsupported arch";
            return false;
    }
}

template <Arch ARCH>
bool JitDebugWriter::WriteImpl()
{
    ElfBuilder<ARCH, true> builder;

    // In gdb you may use '(gdb) info functions jitted' for find all jit-entry
    builder.SetCodeName("(jitted) " + methodName_);

    auto codeProvider = std::make_unique<JitCodeDataProvider>(this);
    builder.GetTextSection()->SetDataProvider(std::move(codeProvider));

    builder.SetFrameData(GetFrameData());
    builder.Build("jitted_code");

    auto elfSize {builder.GetFileSize()};
    auto memRange {codeAllocator_->AllocateCodeUnprotected(elfSize)};
    auto elfData {reinterpret_cast<uint8_t *>(memRange.GetData())};
    if (elfData == nullptr) {
        return false;
    }

    builder.HackAddressesForJit(elfData);

    elf_ = {elfData, elfSize};
    builder.Write(elf_);
    codeAllocator_->ProtectCode(memRange);

    code_ = builder.GetTextSectionData();
    return true;
}
}  // namespace ark::compiler

#endif