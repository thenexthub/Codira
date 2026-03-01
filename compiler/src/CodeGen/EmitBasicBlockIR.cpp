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

#include "EmitBasicBlockIR.h"

#include <deque>

#include "CGModule.h"
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
#include "CODENative/CODENativeCGCFFI.h"
#endif
#include "EmitExpressionIR.h"
#include "IRBuilder.h"
#include "IRGenerator.h"
#include "Utils/BlockScopeImpl.h"
#include "Codira/CHIR/Value.h"

namespace Codira {
namespace CodeGen {
class BasicBlockGeneratorImpl : public IRGeneratorImpl {
public:
    BasicBlockGeneratorImpl(CGModule& cgMod, const CHIR::Block& entryBB)
        : cgMod(cgMod),
          entryBB(entryBB),
          functionToEmitIR(cgMod.GetOrInsertCGFunction(entryBB.GetTopLevelFunc())->GetRawFunction())
    {
    }

    void EmitIR() override;

private:
    void CreateBasicBlocks(const CHIR::Block& chirBB);
    void CreateBasicBlocksIRs(const CHIR::Block& chirBB);
    void CreateLandingPad(const CHIR::Block& chirBB) const;

private:
    CGModule& cgMod;
    const CHIR::Block& entryBB;
    llvm::Function* functionToEmitIR;
    // This DFS auxiliary set specifically addresses cycle detection and handling within Block structures.
    std::set<const CHIR::Block*> auxSet;
};

template <> class IRGenerator<BasicBlockGeneratorImpl> : public IRGenerator<> {
public:
    IRGenerator(CGModule& cgMod, const CHIR::Block& chirBB)
        : IRGenerator<>(std::make_unique<BasicBlockGeneratorImpl>(cgMod, chirBB))
    {
    }
};

void BasicBlockGeneratorImpl::EmitIR()
{
    // Emit all basicBlock to function.
    auxSet.clear();
    CreateBasicBlocks(entryBB);

    // Emit expressions for each basicBlock.
    auxSet.clear();
    CreateBasicBlocksIRs(entryBB);
}

void BasicBlockGeneratorImpl::CreateBasicBlocks(const CHIR::Block& chirBB)
{
    if (auxSet.find(&chirBB) != auxSet.end()) {
        return;
    }

    if (!cgMod.GetMappedBB(&chirBB)) {
        auto bbName = PREFIX_FOR_BB_NAME + chirBB.GetIdentifierWithoutPrefix();
        auto bb = llvm::BasicBlock::Create(cgMod.GetLLVMContext(), bbName, functionToEmitIR);
        cgMod.SetOrUpdateMappedBB(&chirBB, bb);
    }
    CreateLandingPad(chirBB);
    auxSet.emplace(&chirBB);

    for (auto succChirBB : chirBB.GetSuccessors()) {
        CreateBasicBlocks(*succChirBB);
    }
}

void BasicBlockGeneratorImpl::CreateBasicBlocksIRs(const CHIR::Block& chirBB)
{
    if (auxSet.find(&chirBB) != auxSet.end()) {
        return;
    }

    CODEC_ASSERT(cgMod.GetMappedBB(&chirBB));
    EmitExpressionIR(cgMod, chirBB.GetExpressions());
    auxSet.emplace(&chirBB);

    for (auto succChirBB : chirBB.GetSuccessors()) {
        CreateBasicBlocksIRs(*succChirBB);
    }
}

void BasicBlockGeneratorImpl::CreateLandingPad(const CHIR::Block& chirBB) const
{
    if (!chirBB.IsLandingPadBlock()) {
        return;
    }

    IRBuilder2 irBuilder(cgMod);
    CodeGenBlockScope codeGenBlockScope(irBuilder, chirBB);
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    auto landingPad = irBuilder.CreateLandingPad(CGType::GetLandingPadType(cgMod.GetLLVMContext()), 0);
    if (chirBB.GetExceptions().empty()) {
        landingPad->addClause(llvm::Constant::getNullValue(irBuilder.getInt8PtrTy()));
    } else {
        for (auto exceptClass : chirBB.GetExceptions()) {
            auto exceptName = exceptClass->GetClassDef()->GetIdentifierWithoutPrefix();
            auto typeInfo = irBuilder.CreateTypeInfo(*exceptClass);
            auto clause = irBuilder.CreateBitCast(typeInfo, irBuilder.getInt8PtrTy());
            landingPad->addClause(static_cast<llvm::Constant*>(clause));
        }
    }
#endif
}

void EmitBasicBlockIR(CGModule& cgMod, const CHIR::Block& chirBB)
{
    IRGenerator<BasicBlockGeneratorImpl>(cgMod, chirBB).EmitIR();
}
} // namespace CodeGen
} // namespace Codira
