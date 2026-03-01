/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "created_object_file.h"

#include "transforms/transform_utils.h"

#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>

#define DEBUG_TYPE "created-object-file"

namespace ark::llvmbackend {

CreatedObjectFile::CreatedObjectFile(std::unique_ptr<llvm::MemoryBuffer> objectFileBuffer,
                                     std::unique_ptr<llvm::object::ObjectFile> objectFile)
    : buffer_(std::move(objectFileBuffer)), objectFile_(std::move(objectFile))
{
    if (objectFile_ != nullptr) {
        // Generally, we need this index for Irtoc to collect used registers.
        // In other cases, it seems a single scan is pretty enough.
        for (auto section : objectFile_->sections()) {
            sectionIndex_[cantFail(section.getName())] = section;
        }
    }
}

llvm::Expected<std::unique_ptr<CreatedObjectFile>> CreatedObjectFile::CopyOf(llvm::MemoryBufferRef objectFileBuffer)
{
    auto copy = llvm::MemoryBuffer::getMemBufferCopy(objectFileBuffer.getBuffer());
    auto elf = llvm::object::ObjectFile::createObjectFile(copy->getMemBufferRef());
    if (!elf) {
        return elf.takeError();
    }
    auto file = cantFail(std::move(elf));
    return std::make_unique<CreatedObjectFile>(std::move(copy), std::move(file));
}

llvm::object::ObjectFile *CreatedObjectFile::GetObjectFile() const
{
    return objectFile_.get();
}

bool CreatedObjectFile::HasSection(const std::string &name) const
{
    return sectionIndex_.find(name) != sectionIndex_.end();
}

std::vector<uint8_t> SectionReference::ContentToVector()
{
    std::vector<uint8_t> vector;
    ASSERT(GetSize() != 0);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::copy(GetMemory(), GetMemory() + GetSize(), std::back_inserter(vector));
    return vector;
}

SectionReference CreatedObjectFile::GetSection(const std::string &name) const
{
    LLVM_DEBUG(llvm::dbgs() << "Getting section = '" << name << "'\n");
    ASSERT(sectionIndex_.find(name) != sectionIndex_.end() && "Attempt to access an unknown section");
    const auto &section = sectionIndex_.at(name);
    auto contents = cantFail(section.getContents());
    auto memory = reinterpret_cast<const uint8_t *>(contents.data());
    return SectionReference {memory, contents.size(), name, section.getAlignment()};
}

SectionReference CreatedObjectFile::GetSectionByFunctionName(const std::string &fullFunctionName) const
{
    auto sectionReference = GetSection(".text." + fullFunctionName + ".");
    LLVM_DEBUG(llvm::dbgs() << "Got section by function = " << fullFunctionName
                            << " section's size = " << sectionReference.GetSize() << "\n");
    return sectionReference;
}

std::unordered_map<std::string, CreatedObjectFile::StackMapSymbol> CreatedObjectFile::GetStackMapInfo() const
{
    std::unordered_map<std::string, StackMapSymbol> info;
    ASSERT(sectionIndex_.find(RELA_LLVM_STACKMAPS_SECTION) != sectionIndex_.end() &&
           "Attempt to access unknown section '.rela.llvm_stackmaps'");
    const auto &section = sectionIndex_.at(RELA_LLVM_STACKMAPS_SECTION);
    uint32_t counter = 0;
    for (auto relocation : section.relocations()) {
        const auto &symbol = relocation.getSymbol();
        info.insert({cantFail(symbol->getName()).str(), {counter++, cantFail(symbol->getValue())}});
    }
    return info;
}

std::unordered_map<std::string, uint32_t> CreatedObjectFile::GetFaultMapInfo() const
{
    std::unordered_map<std::string, uint32_t> info;
    const auto &section = sectionIndex_.at(".rela.llvm_faultmaps");
    uint32_t counter = 0;
    for (auto relocation : section.relocations()) {
        const auto &symbol = relocation.getSymbol();
        info.insert({cantFail(symbol->getName()).str(), counter++});
    }
    return info;
}

std::vector<SectionReference> CreatedObjectFile::GetRoDataSections() const
{
    std::vector<SectionReference> references;
    for (const auto &[name, section] : sectionIndex_) {
        const auto &sectionName = cantFail(section.getName());
        if (sectionName.startswith(RO_DATA_SECTION_PREFIX)) {
            auto contents = cantFail(section.getContents());
            auto memory = reinterpret_cast<const uint8_t *>(contents.data());
            references.emplace_back(memory, contents.size(), name.str(), section.getAlignment());
        }
    }

    return references;
}

void CreatedObjectFile::WriteTo(std::string_view output) const
{
    std::error_code errorCode;
    llvm::raw_fd_ostream dest(output, errorCode, llvm::sys::fs::FA_Write);
    static constexpr bool GEN_CRASH_DIAG = false;
    if (errorCode) {
        report_fatal_error(llvm::Twine("Unable to open file = '") + output + "' to dump object file, error_code = " +
                               llvm::Twine(errorCode.value()) + ", message = " + errorCode.message(),
                           GEN_CRASH_DIAG);
    }
    llvm::StringRef data;
    if (objectFile_ != nullptr) {
        data = objectFile_->getData();
    }
    LLVM_DEBUG(llvm::dbgs() << "Writing object file, size =  " << data.size() << " to " << output << "\n");
    dest << data;
    if (dest.has_error()) {
        auto error = dest.error();
        report_fatal_error(llvm::Twine("Unable to write object file to '") + output +
                               ", error_code = " + llvm::Twine(error.value()) + ", message = " + error.message(),
                           GEN_CRASH_DIAG);
    }
}
size_t CreatedObjectFile::Size() const
{
    ASSERT(objectFile_ != nullptr && "Attempt to get size of empty CreatedObjectFile");
    return objectFile_->getData().size();
}

}  // namespace ark::llvmbackend
