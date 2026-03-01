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

#include "ark_aot_linker.h"
#include "created_object_file.h"

#include "transforms/transform_utils.h"

#include <llvm/Transforms/Scalar.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/Debug.h>

#define DEBUG_TYPE "ark-aot-linker"

// NB: Do not include anything panda's. It may lead to macro collisions.

namespace ark::llvmbackend {

bool PandaSectionMemoryManager::allowStubAllocation() const
{
    return false;
}

uint8_t *PandaSectionMemoryManager::allocateCodeSection(uintptr_t size, unsigned int alignment, unsigned int sectionId,
                                                        llvm::StringRef sectionName)
{
    auto memory = SectionMemoryManager::allocateCodeSection(size, alignment, sectionId, sectionName);
    RememberAllocation(sectionName, memory, size, alignment);
    return memory;
}

uint8_t *PandaSectionMemoryManager::allocateDataSection(uintptr_t size, unsigned int alignment, unsigned int sectionId,
                                                        llvm::StringRef sectionName, bool readOnly)
{
    auto memory = SectionMemoryManager::allocateDataSection(size, alignment, sectionId, sectionName, readOnly);
    RememberAllocation(sectionName, memory, size, alignment);
    return memory;
}

const std::unordered_map<std::string, SectionReference> &PandaSectionMemoryManager::GetSections() const
{
    return sections_;
}

void PandaSectionMemoryManager::RememberAllocation(llvm::StringRef sectionName, uint8_t *memory, uintptr_t size,
                                                   size_t alignment)
{
    static constexpr std::array SECTIONS_PREFIXES_TO_REMEMBER = {
        TEXT_SECTION_PREFIX,     // .text.
        RO_DATA_SECTION_PREFIX,  // .rodata
        AOT_GOT_SECTION,         // .aot_got
    };

    if (!llvm::any_of(SECTIONS_PREFIXES_TO_REMEMBER,
                      [sectionName](auto prefix) { return sectionName.startswith(prefix); })) {
        LLVM_DEBUG(llvm::dbgs() << "Not remembering allocation for '" << sectionName << "'\n");
        return;
    }

    SectionReference sectionReference {memory, size, sectionName.str(), alignment};
    [[maybe_unused]] auto insertionResult = sections_.insert({sectionName.str(), sectionReference});
    // Check if we've inserted section reference.
    // If we did not, then there is a section duplicate, which we cannot handle
    ASSERT(insertionResult.second);
    LLVM_DEBUG(llvm::dbgs() << "Remembered allocation for '" << sectionName << "', its address = " << memory
                            << ", its size = " << size << '\n');
}

ArkAotLinker::~ArkAotLinker() = default;

llvm::Error ArkAotLinker::LoadObject(std::unique_ptr<CreatedObjectFile> objectFile)
{
    runtimeDyLd_.loadObject(*objectFile->GetObjectFile());
    objects_.push_back(std::move(objectFile));
    if (runtimeDyLd_.hasError()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "Could not load object file, error = '" + runtimeDyLd_.getErrorString() + "'");
    }
    return llvm::Error::success();
}

SectionReference ArkAotLinker::GetLinkedSection(const std::string &name) const
{
    const auto &sections = memoryManager_.GetSections();
    ASSERT(sections.find(name) != sections.end() && "Attempt to access an unknown linked section");
    return sections.at(name);
}

SectionReference ArkAotLinker::GetLinkedFunctionSection(const std::string &fullFunctionName) const
{
    return GetLinkedSection(".text." + fullFunctionName + ".");
}

ArkAotLinker::RoDataSections ArkAotLinker::GetLinkedRoDataSections() const
{
    RoDataSections references;

    for (const auto &file : objects_) {
        for (const auto &section : file->GetRoDataSections()) {
            references.push_back(GetLinkedSection(section.GetName()));
        }
    }
    return references;
}

llvm::Error ArkAotLinker::RelocateSections(const std::unordered_map<std::string, size_t> &sectionAddresses,
                                           const std::unordered_map<std::string, size_t> &methodOffsets,
                                           uint32_t moduleId)
{
    for (const auto &objectFile : objects_) {
        for (auto section : objectFile->GetObjectFile()->sections()) {
            llvm::StringRef name = cantFail(section.getName());

            bool functionSection = name.startswith(TEXT_SECTION_PREFIX);
            bool roDataSection = name.startswith(RO_DATA_SECTION_PREFIX);
            bool aotGotSection = name.startswith(AOT_GOT_SECTION);

            if (functionSection) {
                RelocateFunctionSection(memoryManager_.GetSections().at(name.str()), sectionAddresses, methodOffsets);
            }
            if (roDataSection || aotGotSection) {
                RelocateSection(memoryManager_.GetSections().at(name.str()), sectionAddresses, moduleId);
            }
        }
    }
    runtimeDyLd_.resolveRelocations();
    if (runtimeDyLd_.hasError()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "Could not relocate sections, error = '" + runtimeDyLd_.getErrorString() + "'");
    }
    return llvm::Error::success();
}

void ArkAotLinker::RelocateFunctionSection(const SectionReference &sectionReference,
                                           const std::unordered_map<std::string, size_t> &sectionAddresses,
                                           const std::unordered_map<std::string, size_t> &methodOffsets)
{
    if (sectionReference.GetMemory() == nullptr) {
        return;
    }
    auto sectionStart = sectionAddresses.find(".text");
    if (sectionStart == sectionAddresses.end()) {
        return;
    }

    // Section name = ".text." + fullMethodName
    constexpr auto METHOD_NAME_START = std::string_view(".text.").length();
    std::string methodName = sectionReference.GetName().substr(METHOD_NAME_START);
    methodName.pop_back();  // remove addition dot
    auto methodOffset = methodOffsets.find(methodName);
    if (methodOffset == methodOffsets.end()) {
        return;
    }

    size_t targetAddress = sectionStart->second + methodOffset->second + functionHeaderSize_;
    runtimeDyLd_.mapSectionAddress(sectionReference.GetMemory(), targetAddress);
    LLVM_DEBUG(llvm::dbgs() << "Relocated section '" << sectionReference.GetName()
                            << "' (size = " << sectionReference.GetSize() << ") to " << targetAddress << '\n');
}

void ArkAotLinker::RelocateSection(const SectionReference &sectionReference,
                                   const std::unordered_map<std::string, size_t> &sectionAddresses, uint32_t moduleId)
{
    if (sectionReference.GetMemory() == nullptr) {
        return;
    }

    auto toFind = sectionReference.GetName();
    if (toFind.find(RO_DATA_SECTION_PREFIX) == 0) {
        toFind += std::to_string(moduleId);
    }

    auto sectionStart = sectionAddresses.find(toFind);
    if (sectionStart == sectionAddresses.end()) {
        return;
    }

    runtimeDyLd_.mapSectionAddress(sectionReference.GetMemory(), sectionStart->second);
    LLVM_DEBUG(llvm::dbgs() << "Relocated section '" << sectionReference.GetName()
                            << "' (size = " << sectionReference.GetSize() << ") to " << sectionStart->second << '\n');
}

ArkAotLinker::ArkAotLinker(size_t functionHeaderSize) : functionHeaderSize_(functionHeaderSize)
{
    runtimeDyLd_.setProcessAllSections(true);
}

}  // namespace ark::llvmbackend
