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

/**
 * @file
 *
 * This file implements CFFI typecheck apis.
 */

#include "TypeCheckerImpl.h"

#include "TypeCheckUtil.h"

using namespace Codira;
using namespace Codira::AST;
using namespace TypeCheckUtil;

namespace {
void CheckAnnoC(const Decl& decl, const Annotation& anno, DiagnosticEngine& diag)
{
    if (!anno.args.empty()) {
        diag.Diagnose(*anno.args[0], DiagKind::sema_annotation_error_arg_num, "@C", "no");
    }
    auto generic = decl.GetGeneric();
    if (decl.astKind == ASTKind::FUNC_DECL && generic) {
        diag.Diagnose(*generic, generic->leftAnglePos, DiagKind::sema_cffi_cannot_have_type_param,
            std::string{CFUNC_NAME});
    }
    if (Utils::NotIn(decl.astKind, {ASTKind::FUNC_DECL, ASTKind::STRUCT_DECL})) {
        diag.Diagnose(anno, DiagKind::sema_illegal_use_of_annotation, DeclKindToString(decl), "@C");
    } else if (decl.outerDecl != nullptr || decl.scopeLevel > 0) {
        diag.Diagnose(anno, DiagKind::sema_illegal_scope_use_of_annotation, "@C");
    }
}

void CheckAnnoCallingConv(Decl& decl, const Annotation& anno, DiagnosticEngine& diag)
{
    if (decl.outerDecl != nullptr || decl.scopeLevel > 0) {
        diag.Diagnose(anno, DiagKind::sema_illegal_scope_use_of_annotation, "@CallingConv");
        return;
    }
    if (decl.astKind != ASTKind::FUNC_DECL || (!decl.TestAttr(Attribute::FOREIGN) && !decl.TestAttr(Attribute::C))) {
        // Will be merged with sema_unsupported_annotation_in_js_interop.
        // Will be replaced with anno->identifier.
        diag.Diagnose(anno, DiagKind::sema_only_cfunc_can_use_annotation, "@CallingConv");
        return;
    }
    if (anno.args.size() != 1) {
        diag.Diagnose(anno, DiagKind::sema_annotation_error_arg_num, "@CallingConv", "one");
        return;
    }
    if (auto refExpr = DynamicCast<RefExpr*>(anno.args.front()->expr.get()); refExpr) {
        std::unordered_map<std::string, Attribute> callingConvMap = {
            {"CDECL", Attribute::C},
            {"STDCALL", Attribute::STD_CALL},
        };
        if (callingConvMap.find(refExpr->ref.identifier) == callingConvMap.end()) {
            // Will be merged with sema_unsupported_annotation_in_js_interop.
            diag.Diagnose(anno, DiagKind::sema_annotation_calling_conv_not_support, refExpr->ref.identifier.Val());
            return;
        }
        decl.EnableAttr(callingConvMap[refExpr->ref.identifier]);
    } else {
        diag.Diagnose(anno, DiagKind::sema_annotation_invalid_args_type, "@CallingConv");
    }
}

void CheckAnnoFastNative(const Decl& decl, const Annotation& anno, DiagnosticEngine& diag)
{
    if (!anno.args.empty()) {
        diag.Diagnose(*anno.args[0], DiagKind::sema_annotation_error_arg_num, "@FastNative", "no");
    }
    if (decl.astKind != ASTKind::FUNC_DECL || !decl.TestAttr(Attribute::FOREIGN)) {
        // Will be merged with sema_illegal_scope_use_of_annotation.
        std::string prefix = decl.astKind == ASTKind::FUNC_DECL ? "non-foreign " : "";
        diag.Diagnose(anno, DiagKind::sema_illegal_use_of_annotation, prefix + DeclKindToString(decl), "@FastNative");
        return;
    }
    if (decl.outerDecl != nullptr || decl.scopeLevel > 0) {
        diag.Diagnose(anno, DiagKind::sema_illegal_scope_use_of_annotation, "@FastNative");
    }
}

void CheckAnnoFrozen(const Decl& decl, const Annotation& anno, DiagnosticEngine& diag)
{
    if (!anno.args.empty()) {
        diag.Diagnose(*anno.args[0], DiagKind::sema_annotation_error_arg_num, "@Frozen", "no");
    }
    if (decl.astKind != ASTKind::FUNC_DECL && decl.astKind != ASTKind::PROP_DECL) {
        // Will be merged with sema_illegal_scope_use_of_annotation.
        diag.Diagnose(anno, DiagKind::sema_illegal_use_of_annotation, DeclKindToString(decl), "@Frozen");
        return;
    }
    if (decl.outerDecl != nullptr && decl.outerDecl->astKind == ASTKind::FUNC_DECL) {
        diag.Diagnose(anno, DiagKind::sema_illegal_use_of_annotation, "local function", "@Frozen");
    }
}

using AnnoCheckMap = std::unordered_map<AnnotationKind, std::function<void(Decl&, Annotation&, DiagnosticEngine&)>>;

AnnoCheckMap& GetAnnoCheckMap()
{
    static AnnoCheckMap annoCheckMap = {
        {AnnotationKind::C, CheckAnnoC},
        {AnnotationKind::CALLING_CONV, CheckAnnoCallingConv},
        {AnnotationKind::FASTNATIVE, CheckAnnoFastNative},
        {AnnotationKind::FROZEN, CheckAnnoFrozen},
    };
    return annoCheckMap;
}

bool IsZeroSizedTy(const Ty& ty)
{
    std::unordered_set<const Ty*> traversesCache;
    std::function<bool(const Ty& ty)> isZeroSizedTy = [&isZeroSizedTy, &traversesCache](const Ty& ty) {
        if (ty.IsUnitOrNothing()) {
            return true;
        }
        if (!ty.IsStruct()) {
            return false;
        }
        if (traversesCache.find(&ty) != traversesCache.end()) {
            return false;
        }
        traversesCache.emplace(&ty);
        auto decl = Ty::GetDeclPtrOfTy(&ty);
        CODEC_ASSERT(decl != nullptr && decl->astKind == ASTKind::STRUCT_DECL);
        auto sd = StaticCast<StructDecl*>(decl);
        auto& body = sd->body;
        for (auto& member : body->decls) {
            CODEC_NULLPTR_CHECK(member);
            if (member->astKind != ASTKind::VAR_DECL || member->TestAttr(Attribute::STATIC)) {
                continue;
            }
            CODEC_ASSERT(Ty::IsTyCorrect(member->ty));
            if (!isZeroSizedTy(*member->ty)) {
                return false;
            }
        }
        return true;
    };
    return isZeroSizedTy(ty);
}

inline Range GetFuncBodyRange(const FuncBody& fb, const Type& type)
{
    if (auto begin = type.GetBegin(); !begin.IsZero()) {
        return MakeRange(begin, type.GetEnd());
    }
    auto fd = fb.funcDecl;
    CODEC_NULLPTR_CHECK(fd);
    return MakeRange(fd->identifier);
}
} // namespace

Attribute TypeChecker::TypeCheckerImpl::GetDefaultABI()
{
    // This should be consistent with BackendType. All BackendType should have a default ABI.
    static std::unordered_map<Triple::BackendType, Attribute> defaultABI = {
        {Triple::BackendType::CODENATIVE, Attribute::C}};
    if (defaultABI.find(backendType) != defaultABI.end()) {
        return defaultABI[backendType];
    }
    // Will only appear when the definitions between defaultABI and Triple::BackendType are inconsistent.
    return Attribute::C;
}

void TypeChecker::TypeCheckerImpl::SetForeignABIAttr(Decl& decl)
{
    // Will unify the judgment of foreign, foreign, C, and backend.
    // Update W/R in class Attribute.
    if (HasModifier(decl.modifiers, TokenKind::FOREIGN)) {
        auto defaultABI = GetDefaultABI();
        for (auto& anno : decl.annotations) {
            if (anno->kind == AnnotationKind::C) {
                defaultABI = Attribute::C;
            }
        }
        decl.EnableAttr(defaultABI);
    }
}

void TypeChecker::TypeCheckerImpl::CheckInvalidRefInCFunc(
    const ASTContext& ctx, const AST::RefExpr& re, const AST::Decl& target) const
{
    if (!IsInCFunc(ctx, re) || target.TestAnyAttr(Attribute::CONSTRUCTOR, Attribute::ENUM_CONSTRUCTOR)) {
        return;
    }
    bool isNonStaticMember = !target.TestAttr(Attribute::STATIC) &&
        target.TestAnyAttr(Attribute::IN_CLASSLIKE, Attribute::IN_STRUCT, Attribute::IN_ENUM, Attribute::IN_EXTEND);
    if (isNonStaticMember) {
        diag.Diagnose(re, DiagKind::sema_cfunc_cannot_capture_this, "this");
    }
}

void TypeChecker::TypeCheckerImpl::CheckInvalidRefInCFunc(const ASTContext& ctx, const AST::RefExpr& re) const
{
    if (!IsInCFunc(ctx, re)) {
        return;
    }
    if (re.isThis) {
        diag.Diagnose(re, DiagKind::sema_cfunc_cannot_capture_this, "this");
    } else if (re.isSuper) {
        diag.Diagnose(re, DiagKind::sema_cfunc_cannot_capture_this, "super");
    }
}

bool TypeChecker::TypeCheckerImpl::IsInCFunc(const ASTContext& ctx, const AST::RefExpr& re) const
{
    Symbol* curFuncSym = ScopeManager::GetCurSymbolByKind(SymbolKind::FUNC_LIKE, ctx, re.scopeName);
    while (curFuncSym && curFuncSym->scopeName.length() > 0) {
        if (auto le = DynamicCast<LambdaExpr*>(curFuncSym->node);
            le && Ty::IsTyCorrect(le->ty) && le->ty->IsCFunc()) {
            return true;
        }
        curFuncSym = ScopeManager::GetCurSymbolByKind(SymbolKind::FUNC_LIKE, ctx, curFuncSym->scopeName);
    }
    return false;
}

void TypeChecker::TypeCheckerImpl::CheckCTypeMember(const Decl& decl)
{
    if (decl.astKind != ASTKind::VAR_DECL || Ty::IsInitialTy(decl.ty) || decl.outerDecl == nullptr ||
        !Ty::IsTyCorrect(decl.outerDecl->ty)) {
        return;
    }
    if (Ty::IsCStructType(*decl.outerDecl->ty)) {
        const auto& vd = StaticCast<const VarDecl&>(decl);
        if (vd.ty->IsUnit()) {
            diag.Diagnose(vd.identifier.Begin(), vd.type ? vd.type->end : vd.identifier.End(),
                DiagKind::sema_cstruct_cannot_have_unit_fields, vd.identifier.Val());
            return;
        }
        if (Ty::IsMetCType(*vd.ty)) {
            return;
        }
        diag.Diagnose(vd.identifier.Begin(), vd.type ? vd.type->end : vd.identifier.End(),
            DiagKind::sema_illegal_member_of_cstruct, vd.identifier.Val(), vd.outerDecl->identifier.Val());
    }
}

void TypeChecker::TypeCheckerImpl::CheckUnsafeInvoke(const CallExpr& ce)
{
    if (ce.resolvedFunction == nullptr || !Ty::IsTyCorrect(ce.resolvedFunction->ty) || !IsUnsafeBackend(backendType)) {
        return;
    }
    auto funcTy = StaticCast<FuncTy*>(ce.resolvedFunction->ty);
    bool isUnsafe = ce.resolvedFunction->TestAttr(Attribute::UNSAFE) ||
        ce.resolvedFunction->TestAttr(Attribute::FOREIGN) || funcTy->isC;
    if (!isUnsafe) {
        return;
    }
    if (!ce.TestAttr(Attribute::UNSAFE)) {
        diag.Diagnose(ce, DiagKind::sema_unsafe_function_invoke_failed);
    }
}

void TypeChecker::TypeCheckerImpl::CheckLegalityOfUnsafeAndInout(Node& root)
{
    Walker(&root, [this](auto node) {
        CODEC_ASSERT(node);
        if (node->astKind == ASTKind::CALL_EXPR) {
            CheckUnsafeInvoke(static_cast<const CallExpr&>(*node));
        } else if (node->astKind == ASTKind::FUNC_ARG) {
            const auto& fa = static_cast<const FuncArg&>(*node);
            // If the object modified by inout isn't a RefExpr or MemberAccess node or doesn't meet CType constraint,
            // it's wrong and will be diagnosed in the previous stage.
            if (fa.withInout && Ty::IsTyCorrect(fa.expr->ty) &&
                Utils::In(fa.expr->astKind, {ASTKind::REF_EXPR, ASTKind::MEMBER_ACCESS}) &&
                Ty::IsMetCType(*fa.expr->ty) && IsZeroSizedTy(*fa.expr->ty)) {
                diag.DiagnoseRefactor(DiagKindRefactor::sema_inout_modify_cstring_or_zerosized, fa, "zero-sized type");
            }
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
}

void TypeChecker::TypeCheckerImpl::UnsafeCheck(const FuncBody& fb)
{
    if (fb.funcDecl && (fb.funcDecl->TestAttr(Attribute::FOREIGN) || fb.funcDecl->TestAttr(Attribute::C))) {
        CODEC_ASSERT(!fb.paramLists.empty() && "function body's param list cannot be empty!");
        for (auto& arg : fb.paramLists[0]->params) {
            CheckCFuncParam(*arg);
        }
        CODEC_NULLPTR_CHECK(fb.retType);
        if (!Ty::IsTyCorrect(fb.retType->ty)) {
            return;
        }
        if (!Ty::IsMetCType(*fb.retType->ty)) {
            auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_invalid_cfunc_return_type, *fb.retType,
                GetFuncBodyRange(fb, *fb.retType));
            builder.AddNote("return type is " + fb.retType->ty->String());
        } else if (Is<VArrayTy>(fb.retType->ty)) {
            // VArray not allowed as CFunc return type.
            diag.DiagnoseRefactor(
                DiagKindRefactor::sema_varray_in_cfunc, *fb.retType, GetFuncBodyRange(fb, *fb.retType));
        }
    }
}

void TypeChecker::TypeCheckerImpl::CheckCFuncParam(const AST::FuncParam& fp)
{
    if (!Ty::IsTyCorrect(fp.ty)) {
        return;
    }
    if (fp.isNamedParam) {
        diag.Diagnose(fp, DiagKind::sema_cfunc_cannot_have_named_args);
    }
    if (fp.ty->IsUnit()) {
        diag.Diagnose(fp.identifier.Begin(), fp.type->end, DiagKind::sema_cfunc_cannot_have_unit_args);
    } else if (!Ty::IsMetCType(*fp.ty)) {
        diag.Diagnose(fp.identifier.Begin(), fp.type->end, DiagKind::sema_invalid_cfunc_arg_type);
    }
}

void TypeChecker::TypeCheckerImpl::CheckCFuncParamType(const AST::Type& type)
{
    if (type.ty->IsUnit()) {
        diag.Diagnose(type.begin, type.end, DiagKind::sema_cfunc_cannot_have_unit_args);
    } else if (!Ty::IsMetCType(*type.ty)) {
        diag.Diagnose(type.begin, type.end, DiagKind::sema_invalid_cfunc_arg_type);
    }
}

void TypeChecker::TypeCheckerImpl::PreCheckAnnoForCFFI(Decl& decl)
{
    if (HasModifier(decl.modifiers, TokenKind::FOREIGN) && !decl.HasAnno(AnnotationKind::C)) {
        // Check node.
        auto modifier = decl.modifiers.crbegin();
        if (decl.astKind == ASTKind::CLASS_DECL) {
            diag.Diagnose(*modifier, DiagKind::sema_unexpected_wrapper, "class declaration");
        }
        if (decl.astKind == ASTKind::INTERFACE_DECL) {
            diag.Diagnose(*modifier, DiagKind::sema_unexpected_wrapper, "interface declaration");
        }
        // Will be merged with sema_unexpected_wrapper.
        if (decl.astKind == ASTKind::VAR_DECL) {
            diag.Diagnose(decl, DiagKind::sema_native_var_error);
        }
    }
    auto isAnnoC = [](const OwnedPtr<Annotation>& anno) { return anno->kind == AnnotationKind::C; };
    auto found = std::find_if(decl.annotations.begin(), decl.annotations.end(), isAnnoC);
    if (found != decl.annotations.end()) {
        decl.EnableAttr(Attribute::C);
        if (auto fd = DynamicCast<FuncDecl*>(&decl); fd && fd->funcBody) {
            fd->funcBody->EnableAttr(Attribute::C);
        }
    }
    for (auto& anno : decl.annotations) {
        auto annoCheckMap = GetAnnoCheckMap();
        auto checkerIt = annoCheckMap.find(anno->kind);
        if (checkerIt!= annoCheckMap.end()) {
            checkerIt->second(decl, *anno, diag);
        }
    }

    if (decl.astKind == ASTKind::FUNC_DECL) {
        if (decl.TestAttr(AST::Attribute::FOREIGN)) {
            decl.EnableAttr(Attribute::NO_MANGLE);
        }
        if (auto fd = StaticCast<FuncDecl*>(&decl); fd && fd->TestAttr(Attribute::FOREIGN)) {
            fd->linkage = Linkage::INTERNAL;
        }
    }
}
