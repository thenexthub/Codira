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

#include "EmitExpressionIR.h"

#include "Base/CHIRExprWrapper.h"
#include "Base/ExprDispatcher/ExprDispatcher.h"
#include "CGModule.h"
#include "IRBuilder.h"
#include "IRGenerator.h"
#include "Codira/CHIR/Value.h"

using namespace Codira::CHIR;

namespace Codira::CodeGen {

class ExpressionGeneratorImpl : public IRGeneratorImpl {
public:
    ExpressionGeneratorImpl(CGModule& cgMod, const std::vector<CHIR::Expression*>& chirExprs)
        : cgMod(cgMod), chirExprs(chirExprs)
    {
        // All chirExprs should be in the same BasicBlock.
        for (auto it : chirExprs) {
            CODEC_ASSERT(it->GetParentBlock() == chirExprs[0]->GetParentBlock());
        }
    }

    void EmitIR() override;

private:
    CGModule& cgMod;
    const std::vector<Codira::CHIR::Expression*> chirExprs;
};

template <> class IRGenerator<ExpressionGeneratorImpl> : public IRGenerator<> {
public:
    IRGenerator(CGModule& cgMod, const std::vector<CHIR::Expression*>& chirExprs)
        : IRGenerator<>(std::make_unique<ExpressionGeneratorImpl>(cgMod, chirExprs))
    {
    }
};

void ExpressionGeneratorImpl::EmitIR()
{
    CODEC_ASSERT(!chirExprs.empty());
    IRBuilder2 irBuilder(cgMod);
    // Since we restricted all the expressions are in the same basic block,
    // we are able to just use the first expression to determine the insert
    // point.
    if (auto parentBB = chirExprs.front()->GetParentBlock()) {
        irBuilder.SetInsertPoint(cgMod.GetMappedBB(parentBB));
    }
    irBuilder.CreateGenericParaDeclare(*cgMod.GetOrInsertCGFunction(chirExprs.front()->GetTopLevelFunc()));
    for (auto chirExpr : chirExprs) {
        CHIRExprWrapper chirExprWrapper = CHIRExprWrapper(*chirExpr);
        if (chirExpr->GetParentBlock()) {
            irBuilder.EmitLocation(chirExprWrapper);
        }
        irBuilder.SetCHIRExpr(&chirExprWrapper);
        if (cgMod.GetCGContext().GetCompileOptions().enableCompileDebug) {
            irBuilder.CreateLoadInstForParameter(*chirExpr);
        }

        llvm::Value* rawRet = nullptr;
        switch (chirExpr->GetExprMajorKind()) {
            case ExprMajorKind::TERMINATOR:
                rawRet = HandleTerminatorExpression(irBuilder, *chirExpr);
                break;
            case ExprMajorKind::UNARY_EXPR: {
                auto unaryExpr = StaticCast<const CHIR::UnaryExpression*>(chirExpr);
                irBuilder.EmitLocation(CHIRExprWrapper(*chirExpr));
                rawRet = HandleUnaryExpression(irBuilder, CHIRUnaryExprWrapper(*unaryExpr));
                break;
            }
            case ExprMajorKind::BINARY_EXPR: {
                auto binaryExpr = StaticCast<const CHIR::BinaryExpression*>(chirExpr);
                irBuilder.EmitLocation(CHIRExprWrapper(*chirExpr));
                rawRet = HandleBinaryExpression(irBuilder, CHIRBinaryExprWrapper(*binaryExpr));
                break;
            }
            case ExprMajorKind::MEMORY_EXPR:
                rawRet = HandleMemoryExpression(irBuilder, *chirExpr);
                break;
            case ExprMajorKind::OTHERS:
                if (chirExpr->IsConstant()) {
                    rawRet = HandleConstantExpression(irBuilder, *StaticCast<const CHIR::Constant*>(chirExpr));
                } else {
                    rawRet = HandleOthersExpression(irBuilder, *chirExpr);
                }
                break;
            default: {
                CODEC_ASSERT(false && "Should not reach here.");
                break;
            }
        }
        if (auto rst = chirExpr->GetResult(); rst && rawRet) {
            CGType* cgType = CGType::GetOrCreate(cgMod, rst->GetType());
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
            auto& cgCtx = cgMod.GetCGContext();
            // A value that is neither a reference nor a RefType, and if it is passed by a pointer,
            // we need to yield a new CGType for it.
            if (!rst->GetType()->IsRef() && !rst->GetType()->IsGeneric() && rawRet->getType()->isPointerTy() &&
                !cgType->IsReference() && !cgType->IsOptionLikeRef()) {
                cgType = CGType::GetOrCreate(cgMod, CGType::GetRefTypeOf(cgCtx.GetCHIRBuilder(), *rst->GetType()),
                    CGType::TypeExtraInfo(rawRet->getType()->getPointerAddressSpace()));
            }
#endif
            CODEC_NULLPTR_CHECK(chirExpr->GetTopLevelFunc());
            bool isSRetArg =
                irBuilder.GetInsertCGFunction()->IsSRet() && chirExpr->GetTopLevelFunc()->GetReturnValue() == rst;
            cgMod.SetOrUpdateMappedCGValue(rst, std::make_unique<CGValue>(rawRet, cgType, isSRetArg));
        }
    }
}

void EmitExpressionIR(CGModule& cgMod, std::vector<CHIR::Expression*> chirExprs)
{
    IRGenerator<ExpressionGeneratorImpl>(cgMod, chirExprs).EmitIR();
}
} // namespace Codira::CodeGen
