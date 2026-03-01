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

#include "Codira/CHIR/AST2CHIR/TranslateASTNode/Translator.h"
#include "Codira/CHIR/Expression/Terminator.h"

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
#include "AnnoFactoryInfo.h"
#include "Codira/Mangle/CHIRManglingUtils.h"
#endif

using namespace Codira::CHIR;
using namespace Codira;

static const std::string& GetIdentifierToPrint(const AST::Decl& decl)
{
    if (auto prop = DynamicCast<AST::PropDecl>(&decl)) {
        if (!prop->getters.empty()) {
            return prop->getters[0]->identifier;
        }
    }
    return decl.identifier;
}

using namespace AST;

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
// Annotation mangling rules here.
// 1) Annotation Factory func: _CAF + package + orignalDecl + "Hv"
// 2) each annotation var: _CAO + package + originalDecl + 64bit var index + "E"
// 3) each annotation var init: _CAO + package + originalDecl + 64bit var index + "iiHv"
GlobalVar* Translator::TranslateCustomAnnoInstanceSig(const Expr& expr, const Func& func, size_t i)
{
    auto name = func.GetIdentifierWithoutPrefix();
    CODEC_ASSERT(name.size() > 3UL);
    // change _CAF prefix to _CAO
    name[3UL] = 'O';
    name += MANGLE_COUNT_PREFIX;
    name += MangleUtils::DecimalToManglingNumber(std::to_string(i));
    auto varName = name + MANGLE_SUFFIX;
    auto gv = builder.CreateGlobalVar(INVALID_LOCATION,
        builder.GetType<RefType>(TranslateType(*expr.ty)), varName, varName, "", func.GetPackageName());
    gv->EnableAttr(Attribute::COMPILER_ADD);
    gv->EnableAttr(Attribute::CONST);
    gv->Set<LinkTypeInfo>(Linkage::INTERNAL);
    auto initName = std::move(name) + "iiHv";
    auto ty = builder.GetType<FuncType>(std::vector<Type*>{}, builder.GetVoidTy());
    auto init = builder.CreateFunc(INVALID_LOCATION, ty, initName, initName, "", func.GetPackageName());
    init->SetFuncKind(FuncKind::GLOBALVAR_INIT);
    init->Set<LinkTypeInfo>(Linkage::INTERNAL);
    init->EnableAttr(Attribute::CONST);
    init->EnableAttr(Attribute::COMPILER_ADD);
    init->EnableAttr(Attribute::INITIALIZER);
    init->EnableAttr(Attribute::NO_INLINE);
    init->EnableAttr(Attribute::NO_REFLECT_INFO);
    initFuncsForAnnoFactory.push_back(init);
    auto bg = builder.CreateBlockGroup(*init);
    init->InitBody(*bg);
    auto bl = builder.CreateBlock(bg);
    bg->SetEntryBlock(bl);
    gv->SetInitFunc(*init);
    return gv;
}

std::vector<GlobalVar*> Translator::TranslateAnnotationsArraySig(const ArrayLit& annos, const Func& func)
{
    std::vector<GlobalVar*> res;
    for (size_t i{0}; i < annos.children.size(); ++i) {
        if (isComputingAnnos && annos.children[i]->TestAttr(AST::Attribute::NO_REFLECT_INFO)) {
            continue;
        }
        res.push_back(TranslateCustomAnnoInstanceSig(*annos.children[i], func, i));
    }
    return res;
}

void Translator::TranslateAnnotationsArrayBody(const Decl& decl, Func& func)
{
    auto& annoInsts = func.Get<AnnoFactoryInfo>();
    auto annoArrSize =
        CreateAndAppendConstantExpression<IntLiteral>(builder.GetInt64Ty(), *currentBlock, annoInsts.size())
            ->GetResult();
    auto objectTy = builder.GetType<RefType>(builder.GetObjectTy());
    auto rawArrayTy = builder.GetType<RawArrayType>(objectTy, 1);
    auto rawArray = CreateAndAppendExpression<RawArrayAllocate>(
        builder.GetType<RefType>(rawArrayTy), objectTy, annoArrSize, currentBlock);
    auto arrayGeneric = builder.GetStructType("std.core", "Array");
    auto genericType = StaticCast<GenericType*>(arrayGeneric->GetGenericArgs()[0]);
    std::unordered_map<const GenericType*, Type*> table{{genericType, objectTy}};
    auto arrayType = ReplaceRawGenericArgType(*arrayGeneric, table, builder);
    auto array = CreateAndAppendExpression<Allocate>(builder.GetType<RefType>(arrayType), arrayType, currentBlock);
    auto arrayMethods = arrayGeneric->GetStructDef()->GetMethods();
    auto arrayInit = std::find_if(arrayMethods.begin(), arrayMethods.end(),
        [](auto method) { return method->IsConstructor() && method->GetNumOfParams() == 4UL; });
    CODEC_ASSERT(arrayInit != arrayMethods.end());
    auto zero = CreateAndAppendConstantExpression<IntLiteral>(builder.GetInt64Ty(), *currentBlock, 0);
    // call array init with RawArrayAllocate
    auto callContext = FuncCallContext {
        .args = std::vector<Value*>{array->GetResult(), rawArray->GetResult(), zero->GetResult(), annoArrSize},
        .thisType = builder.GetType<RefType>(arrayType)
    };
    auto retType = ReplaceRawGenericArgType(*(*arrayInit)->GetFuncType()->GetReturnType(), table, builder);
    CreateAndAppendExpression<Apply>(retType, *arrayInit, callContext, currentBlock);
    // load all Annotation Instances from lifted gv and store them into the return value of annoFactoryFunc
    for (size_t i{0}; i < annoInsts.size(); ++i) {
        auto gv = annoInsts[i];
        auto load =
            CreateAndAppendExpression<Load>(StaticCast<RefType>(gv->GetType())->GetBaseType(), gv, currentBlock);
        auto typecast = CreateAndAppendExpression<TypeCast>(objectTy, load->GetResult(), currentBlock);
        CreateAndAppendExpression<StoreElementRef>(
            builder.GetUnitTy(), typecast->GetResult(), rawArray->GetResult(), std::vector<uint64_t>{i}, currentBlock);
    }
    func.SetReturnValue(*array->GetResult());
    CreateAndAppendTerminator<Exit>(currentBlock);

    // translate Annotation instance init funcs
    // no need to cache currentBlock here, because the current function has been completed
    size_t annoChildrenId{0};
    for (size_t i{0}; i < annoInsts.size(); ++i) {
        while (isComputingAnnos &&
            decl.annotationsArray->children[annoChildrenId]->TestAttr(AST::Attribute::NO_REFLECT_INFO)) {
            ++annoChildrenId;
        }
        auto gv = annoInsts[i];
        auto gvInit = gv->GetInitFunc();
        blockGroupStack.push_back(gvInit->GetBody());
        currentBlock = gvInit->GetEntryBlock();
        TranslateSubExprToLoc(*decl.annotationsArray->children[annoChildrenId++], gv);
        CreateAndAppendTerminator<Exit>(currentBlock);
        blockGroupStack.pop_back();
    }
#ifndef NDEBUG
    while (isComputingAnnos && annoChildrenId < decl.annotationsArray->children.size() &&
        decl.annotationsArray->children[annoChildrenId]->TestAttr(AST::Attribute::NO_REFLECT_INFO)) {
        ++annoChildrenId;
    }
    CODEC_ASSERT(decl.annotationsArray->children.size() == annoChildrenId);
#endif

    // call the Annotation instance func in the package init func, for the sake of consteval
    auto pkgInit = builder.GetChirContext().GetCurPackage()->GetPackageInitFunc();
    blockGroupStack.push_back(pkgInit->GetBody());
    currentBlock = pkgInit->GetEntryBlock()->GetSuccessors()[1];
    currentBlock->GetTerminator()->RemoveSelfFromBlock();
    for (size_t i{0}; i < annoInsts.size(); ++i) {
        CreateAndAppendExpression<Apply>(
            builder.GetVoidTy(), annoInsts[i]->GetInitFunc(), FuncCallContext{}, currentBlock);
    }
    CreateAndAppendTerminator<Exit>(currentBlock);
    blockGroupStack.pop_back();
}
#endif

void Translator::TranslateAnnoFactoryFuncBody([[maybe_unused]] const AST::Decl& decl, Func& func)
{
    auto body = builder.CreateBlockGroup(func);
    blockGroupStack.emplace_back(body);
    func.InitBody(*body);
    func.EnableAttr(Attribute::COMPILER_ADD);
    // create body
    auto bodyBlock = CreateBlock();
    body->SetEntryBlock(bodyBlock);
    currentBlock = bodyBlock;
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    TranslateAnnotationsArrayBody(decl, func);
#endif
    blockGroupStack.pop_back();
}

AnnoInfo Translator::CreateAnnoFactoryFuncSig(const AST::Decl& decl, CustomTypeDef* parent)
{
    // Since annotations must be consistent between common and platform sides,
    // the platform side can directly use the serialized annotations.
    auto annosArray = decl.annotationsArray.get();
    if (decl.TestAttr(AST::Attribute::IMPORTED) || !annosArray || annosArray->children.empty() ||
        decl.TestAttr(AST::Attribute::PLATFORM)) {
        return {"none"};
    }
    auto found = annotationFuncMap.find(&decl);
    if (found != annotationFuncMap.end()) {
        return {found->second}; // Property's getters and setters share the same annotation function.
    }
    Type* returnTy = nullptr;
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    returnTy = TranslateType(*annosArray->ty);
#endif
    auto funcType = builder.GetType<FuncType>(std::vector<Type*>{}, returnTy);
    const auto& loc = TranslateLocation(*decl.annotationsArray->children[0]);
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    std::string mangledName = CHIRMangling::GenerateAnnotationFuncMangleName(decl.mangledName);
#endif
    if (opts.chirDebugOptimizer) {
        std::string ms = "The annotation factory function of " + GetIdentifierToPrint(decl) + " in the line " +
            std::to_string(decl.begin.line) + " is " + mangledName + '\n';
        std::cout << ms;
    }
    auto func = builder.CreateFunc(loc, funcType, mangledName, mangledName, "", decl.fullPackageName);
    func->SetFuncKind(FuncKind::ANNOFACTORY_FUNC);
    func->EnableAttr(Attribute::CONST);
    annoFactoryFuncs.emplace_back(&decl, func);
    annotationFuncMap.emplace(&decl, mangledName);
    if (parent) {
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
        parent->AddMethod(func);
#endif
        func->EnableAttr(Attribute::STATIC);
    }

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    auto gvs = TranslateAnnotationsArraySig(*annosArray, *func);
    func->Set<AnnoFactoryInfo>(std::move(gvs));
    // Collect annotations whose parameter values are literal constants.
    std::vector<AnnoInfo::AnnoPair> annoPairs;
    for (auto& elem : annosArray->children) {
        auto callExpr = StaticCast<AST::CallExpr*>(elem.get().get());
        auto& callee = callExpr->resolvedFunction->funcBody;
        auto annoClassDecl = callee->parentClassLike;
        CODEC_ASSERT(annoClassDecl);
        auto annoClassDeclName = annoClassDecl->identifier.GetRawText();
        std::vector<std::string> paramValues;
        for (auto& arg : callExpr->args) {
            if (auto& argVal = arg->expr; argVal->astKind == AST::ASTKind::LIT_CONST_EXPR) {
                auto lit = StaticCast<AST::LitConstExpr*>(argVal.get().get());
                paramValues.emplace_back(lit->rawString);
            } else {
                return {mangledName, {}};
            }
        }
        annoPairs.emplace_back(annoClassDeclName, paramValues);
    }
    return {mangledName, annoPairs};
#endif
}

std::unordered_map<std::string, Ptr<Func>> Translator::jAnnoFuncMap;

void Translator::CreateParamAnnotationInfo(const AST::FuncParam& astParam, Parameter& chirParam, CustomTypeDef& parent)
{
    chirParam.SetAnnoInfo(CreateAnnoFactoryFuncSig(astParam, &parent));
}

void Translator::CreateAnnoFactoryFuncsForFuncDecl(const AST::FuncDecl& funcDecl, CustomTypeDef* parent)
{
    auto& params = funcDecl.funcBody->paramLists[0]->params;
    auto funcValue = GetSymbolTable(funcDecl);
    const AST::Decl& annotatedDecl = funcDecl.propDecl ? *funcDecl.propDecl : StaticCast<AST::Decl>(funcDecl);
    if (auto func = DynamicCast<Func>(funcValue)) {
        CreateAnnotationInfo<Func>(annotatedDecl, *func, parent);
        size_t offset = params.size() == func->GetNumOfParams() ? 0 : 1;
        for (size_t i = 0; i < params.size(); ++i) {
            CreateParamAnnotationInfo(*params[i], *func->GetParam(i + offset), *parent);
        }
    } else if (!funcDecl.TestAttr(AST::Attribute::IMPORTED) && funcValue->TestAttr(Attribute::NON_RECOMPILE)) {
        // Update annotation info for incremental created 'ImportedValue';
        auto importedFunc = DynamicCast<ImportedFunc>(funcValue);
        CODEC_NULLPTR_CHECK(importedFunc);
        CreateAnnotationInfo<ImportedFunc>(annotatedDecl, *importedFunc, parent);
        auto paramInfo = importedFunc->GetParamInfo();
        for (size_t i = 0; i < params.size(); ++i) {
            paramInfo[i].annoInfo = CreateAnnoFactoryFuncSig(*params[i], parent);
        }
        importedFunc->SetParamInfo(std::move(paramInfo));
    }
}
