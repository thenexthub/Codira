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

#include "EmitGlobalVariableIR.h"

#include "Base/ExprDispatcher/ExprDispatcher.h"
#include "CGModule.h"
#include "DIBuilder.h"
#include "EmitExpressionIR.h"
#include "IRBuilder.h"
#include "IRGenerator.h"
#include "Codira/CHIR/Value.h"

namespace Codira {
namespace CodeGen {
class GlobalVariableGeneratorImpl : public IRGeneratorImpl {
public:
    GlobalVariableGeneratorImpl(CGModule& cgMod, const std::vector<CHIR::GlobalVar*>& chirGVs)
        : cgMod(cgMod), chirGVs(chirGVs)
    {
    }

    void EmitIR() override;

private:
    CGModule& cgMod;
    const std::vector<CHIR::GlobalVar*> chirGVs;
};

template <> class IRGenerator<GlobalVariableGeneratorImpl> : public IRGenerator<> {
public:
    IRGenerator(CGModule& cgMod, const std::vector<CHIR::GlobalVar*>& chirGVs)
        : IRGenerator<>(std::make_unique<GlobalVariableGeneratorImpl>(cgMod, chirGVs))
    {
    }
};

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
void GlobalVariableGeneratorImpl::EmitIR()
{
    IRBuilder2 irBuilder(cgMod);
    std::set<CHIR::GlobalVar*> quickGVs(chirGVs.begin(), chirGVs.end());
    for (auto chirGV : cgMod.GetCGContext().GetCHIRPackage().GetGlobalVars()) {
        auto rawGV = llvm::cast<llvm::GlobalVariable>(cgMod.GetOrInsertGlobalVariable(chirGV)->GetRawValue());
        cgMod.diBuilder->CreateGlobalVar(*chirGV);
        if (quickGVs.find(chirGV) == quickGVs.end()) {
            continue;
        }
        const auto align = cgMod.GetLLVMModule()->getDataLayout().getPrefTypeAlignment(rawGV->getType());
        rawGV->setAlignment(llvm::MaybeAlign(align));
        if (auto literal = chirGV->GetInitializer()) {
            auto literalValue = HandleLiteralValue(irBuilder, *literal);
            if (literal->GetType()->IsString()) {
                cgMod.GetCGContext().AddCODEString(
                    rawGV->getName().str(), StaticCast<CHIR::StringLiteral*>(literal)->GetVal());
            } else {
                rawGV->setInitializer(llvm::cast<llvm::Constant>(literalValue));
            }
            if (chirGV->TestAttr(CHIR::Attribute::READONLY)) {
                rawGV->addAttribute(llvm::Attribute::ReadOnly);
                rawGV->setConstant(true);
            }
        } else {
            auto chirType = StaticCast<CHIR::RefType*>(chirGV->GetType())->GetBaseType();
            rawGV->setInitializer(llvm::cast<llvm::Constant>(irBuilder.CreateNullValue(*chirType)));
        }
        if (!chirGV->GetParentCustomTypeDef()) {
            auto fieldMeta = llvm::MDTuple::get(
                cgMod.GetLLVMContext(), {llvm::MDString::get(cgMod.GetLLVMContext(), MangleType(*chirGV->GetType()))});
            rawGV->setMetadata(GC_GLOBAL_VAR_TYPE, fieldMeta);
        }
    }
}
#endif

void EmitGlobalVariableIR(CGModule& cgMod, const std::vector<CHIR::GlobalVar*>& chirGVs)
{
    IRGenerator<GlobalVariableGeneratorImpl>(cgMod, chirGVs).EmitIR();
}
} // namespace CodeGen
} // namespace Codira
