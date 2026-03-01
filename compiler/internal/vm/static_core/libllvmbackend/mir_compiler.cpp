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

#include "mir_compiler.h"

#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/SmallVectorMemoryBuffer.h>

namespace ark::llvmbackend {

void InsertingPassManager::add(llvm::Pass *p)
{
    // Obtain id first before PassManager::add. It might free the pass.
    auto id = p->getPassID();
    if (befores_.find(id) != befores_.end()) {
        InsertingPassManager::add(befores_[id]);
    }
    llvm::legacy::PassManager::add(p);
}

llvm::Expected<std::unique_ptr<CreatedObjectFile>> MIRCompiler::CompileModule(llvm::Module &module)
{
    llvm::SmallVector<char, 0> rawBuffer;
    {
        llvm::raw_svector_ostream rawStream(rawBuffer);

        InsertingPassManager manager;

        if (insertPasses_) {
            insertPasses_(&manager);
        }
        // Some passes from addPassesToEmitMC require TargetTransformInfo
        manager.add(llvm::createTargetTransformInfoWrapperPass(targetMachine_->getTargetIRAnalysis()));
        if (targetMachine_->addPassesToEmitFile(manager, rawStream, nullptr, llvm::CGFT_ObjectFile)) {
            return llvm::make_error<llvm::StringError>("Target does not support MC emission",
                                                       llvm::inconvertibleErrorCode());
        }
        manager.run(module);
    }

    auto memBuffer = std::make_unique<llvm::SmallVectorMemoryBuffer>(std::move(rawBuffer),
                                                                     module.getModuleIdentifier() + "-object-buffer");

    return CreatedObjectFile::CopyOf(memBuffer->getMemBufferRef());
}

}  // namespace ark::llvmbackend
