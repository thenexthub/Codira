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

#include "Codira/AST/Node.h"
#include "Codira/CHIR/AST2CHIR/TranslateASTNode/Translator.h"
#include "Codira/CHIR/AST2CHIR/Utils.h"
#include "Codira/AST/Walker.h"
#include "Codira/CHIR/Package.h"

using namespace Codira::CHIR;
using namespace Codira;

Ptr<Value> Translator::Visit(const AST::LambdaExpr& lambdaExpr)
{
    CODEC_ASSERT(lambdaExpr.funcBody && lambdaExpr.funcBody->body);
    CODEC_ASSERT(!lambdaExpr.mangledName.empty());
    auto lambdaTrans = SetupContextForLambda(*lambdaExpr.funcBody->body);
    auto funcTy = RawStaticCast<FuncType*>(TranslateType(*lambdaExpr.ty));
    // Create lambda body and parameters.
    CODEC_ASSERT(currentBlock->GetTopLevelFunc());
    BlockGroup* body = builder.CreateBlockGroup(*currentBlock->GetTopLevelFunc());
    const auto& loc = TranslateLocation(lambdaExpr);
    auto mangledName = lambdaExpr.mangledName;
    if (funcTy->IsCFunc()) {
        mangledName += CFFI_FUNC_SUFFIX;
    }
    // codedb need src code name to show the stack, or core dump will occurred in some case
    auto lambda = CreateAndAppendExpression<Lambda>(loc, funcTy, funcTy, currentBlock, true, mangledName, "$lambda");
    lambda->InitBody(*body);

    std::vector<DebugLocation> paramLoc;
    for (auto& astParam : lambdaExpr.funcBody->paramLists[0]->params) {
        paramLoc.emplace_back(TranslateLocationWithoutScope(builder.GetChirContext(), astParam->begin, astParam->end));
    }
    auto paramTypes = funcTy->GetParamTypes();
    CODEC_ASSERT(paramTypes.size() == paramLoc.size());
    for (size_t i = 0; i < paramTypes.size(); ++i) {
        builder.CreateParameter(paramTypes[i], paramLoc[i], *lambda);
    }

    if (auto lambdaBody = lambda->GetBody(); lambdaBody && lambdaExpr.TestAttr(AST::Attribute::MOCK_SUPPORTED)) {
        lambdaBody->EnableAttr(CHIR::Attribute::NO_INLINE);
    }

    // lambda never has default parameter value
    return lambdaTrans.TranslateLambdaBody(lambda, *lambdaExpr.funcBody, {});
}

Translator Translator::Copy() const
{
    return {builder, chirTy, opts, gim, globalSymbolTable, localConstVars, localConstFuncs, increKind,
        deserializedVals, annoFactoryFuncs, maybeUnreachable, isComputingAnnos, initFuncsForAnnoFactory, typeManager};
}

Translator Translator::SetupContextForLambda(const AST::Block& body)
{
    // Copy local symbols, and update symbol for let decl which needs deref before used in lambda.
    Translator trans = Copy();
    // Collect local variables which is captured by current funcBody.
    std::unordered_set<Ptr<const AST::Node>> usedCapturedDecls;
    AST::ConstWalker(&body, [&usedCapturedDecls](auto node) {
        if (auto target = node->GetTarget();
            (Is<AST::VarDecl>(target) && target->TestAttr(AST::Attribute::IS_CAPTURE)) || Is<AST::FuncDecl>(target)) {
            usedCapturedDecls.emplace(target);
        }
        return AST::VisitAction::WALK_CHILDREN;
    }).Walk();
    std::vector<std::pair<const Codira::AST::Node*, Value*>> capturedSymbol;
    for (auto [node, symbol] : localValSymbolTable.GetALL()) {
        if (node->astKind != AST::ASTKind::VAR_DECL) {
            trans.SetSymbolTable(*node, *symbol);
            continue;
        }
        auto vd = StaticCast<AST::VarDecl>(node);
        if (!vd->TestAttr(AST::Attribute::IS_CAPTURE) || vd->isVar) {
            trans.SetSymbolTable(*node, *symbol);
            continue;
        }
        // Ignore local variables which is not used in current funcBody.
        if (usedCapturedDecls.count(node) == 0) {
            continue;
        }
        capturedSymbol.emplace_back(node, symbol);
    }
    std::sort(capturedSymbol.begin(), capturedSymbol.end(),
        [](auto& p1, auto& p2) { return AST::CompNodeByPos(p1.first, p2.first); });
    for (auto [node, symbol] : capturedSymbol) {
        trans.SetSymbolTable(*node, *GetDerefedValue(symbol));
    }
    // Copy block group status and current block for new lambda translator.
    trans.blockGroupStack = blockGroupStack;
    trans.currentBlock = currentBlock;
    // Copy 'exprValueTable' for desugared mapping expressions' value.
    trans.exprValueTable = exprValueTable;
    return trans;
}

Ptr<Value> Translator::TranslateLambdaBody(
    Ptr<Lambda> lambda, const AST::FuncBody& funcBody, const BindingConfig& config)
{
    // NOTE: This method must be called with new translator.
    auto blockGroup = lambda->GetBody();
    blockGroupStack.emplace_back(blockGroup);
    auto entry = builder.CreateBlock(blockGroup);
    blockGroup->SetEntryBlock(entry);
    BindingFuncParam(*funcBody.paramLists[0], *lambda->GetBody(), config);
    // Set return value.
    auto retType = lambda->GetReturnType();
    auto retVal =
        CreateAndAppendExpression<Allocate>(DebugLocation(), builder.GetType<RefType>(retType), retType, entry)
            ->GetResult();
    lambda->SetReturnValue(*retVal);
    // Translate body.
    auto block = Visit(funcBody);
    CreateAndAppendTerminator<GoTo>(StaticCast<Block*>(block.get()), entry);
    blockGroupStack.pop_back();
    return lambda->GetResult();
}
