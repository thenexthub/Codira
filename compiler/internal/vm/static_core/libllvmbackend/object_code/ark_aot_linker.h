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

#ifndef LIBLLVMBACKEND_OBJECT_CODE_ARK_AOT_LINKER_H
#define LIBLLVMBACKEND_OBJECT_CODE_ARK_AOT_LINKER_H

#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Pass.h>
#include <llvm/Target/TargetMachine.h>

#include "created_object_file.h"
#include "llvm_ark_interface.h"
#include "llvm_compiler_options.h"
#include "transforms/llvm_optimizer.h"

// NB: Include carefully. May lead to macro collisions between LLVM and Panda headers.

namespace ark::llvmbackend {

class PandaSectionMemoryManager : public llvm::SectionMemoryManager {
public:
    bool allowStubAllocation() const override;

    uint8_t *allocateCodeSection(uintptr_t size, unsigned int alignment, unsigned int sectionId,
                                 llvm::StringRef sectionName) override;

    uint8_t *allocateDataSection(uintptr_t size, unsigned int alignment, unsigned int sectionId,
                                 llvm::StringRef sectionName, bool readOnly) override;

    const std::unordered_map<std::string, SectionReference> &GetSections() const;

private:
    void RememberAllocation(llvm::StringRef sectionName, uint8_t *memory, uintptr_t size, size_t alignment);

private:
    std::unordered_map<std::string, SectionReference> sections_;
};

class ArkAotLinker {
public:
    using RoDataSections = llvm::SmallVector<SectionReference, 4U>;

    explicit ArkAotLinker(size_t functionHeaderSize);

    ArkAotLinker(const ArkAotLinker &) = delete;
    ArkAotLinker &operator=(const ArkAotLinker &) = delete;
    ArkAotLinker(ArkAotLinker &&) = delete;
    ArkAotLinker &operator=(ArkAotLinker &&) = delete;

    ~ArkAotLinker();

    llvm::Error LoadObject(std::unique_ptr<CreatedObjectFile> objectFile);

    SectionReference GetLinkedSection(const std::string &name) const;

    SectionReference GetLinkedFunctionSection(const std::string &fullFunctionName) const;

    RoDataSections GetLinkedRoDataSections() const;

    llvm::Error RelocateSections(const std::unordered_map<std::string, size_t> &sectionAddresses,
                                 const std::unordered_map<std::string, size_t> &methodOffsets, uint32_t moduleId);

private:
    void RelocateFunctionSection(const SectionReference &sectionReference,
                                 const std::unordered_map<std::string, size_t> &sectionAddresses,
                                 const std::unordered_map<std::string, size_t> &methodOffsets);

    void RelocateSection(const SectionReference &sectionReference,
                         const std::unordered_map<std::string, size_t> &sectionAddresses, uint32_t moduleId);

private:
    size_t functionHeaderSize_;
    llvm::SmallVector<std::unique_ptr<CreatedObjectFile>, 1> objects_;

    PandaSectionMemoryManager memoryManager_;
    llvm::RuntimeDyld runtimeDyLd_ {memoryManager_, memoryManager_};
};

}  // namespace ark::llvmbackend

#endif  //  LIBLLVMBACKEND_OBJECT_CODE_ARK_AOT_LINKER_H
