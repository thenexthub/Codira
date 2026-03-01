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

#include "DiagsInterop.h"
#include "Diags.h"
#include "Utils.h"
#include "Codira/AST/Utils.h"

namespace Codira::Interop::Java {
using namespace Sema;

namespace {
Range MakeJavaImplJavaNameRange(const ClassLikeDecl& decl)
{
    if (!HasPredefinedJavaName(decl)) {
        return MakeRangeForDeclIdentifier(decl);
    }

    for (auto& anno : decl.annotations) {
        if (anno->kind != AnnotationKind::JAVA_IMPL) {
            continue;
        }

        return MakeRange(anno->GetBegin(), decl.identifier.End());
    }
    return MakeRange(decl.identifier.Begin(), decl.identifier.End());
}
} // namespace

void DiagJavaImplRedefinitionInJava(DiagnosticEngine& diag, const ClassLikeDecl& decl, const ClassLikeDecl& prevDecl)
{
    if (decl.TestAttr(Attribute::IS_BROKEN)) {
        return;
    }
    auto prevDeclFqName = GetJavaFQSourceCodeName(prevDecl);
    CODEC_ASSERT(GetJavaFQSourceCodeName(decl) == prevDeclFqName);
    auto rangePrev = MakeJavaImplJavaNameRange(prevDecl);
    auto rangeNext = MakeJavaImplJavaNameRange(decl);

    auto builder =
        diag.DiagnoseRefactor(DiagKindRefactor::sema_java_impl_redefinition, prevDecl, rangeNext, prevDeclFqName);
    builder.AddNote(decl, rangePrev, "'" + prevDeclFqName + "' is previously declared here");
}

void DiagJavaMirrorChildMustBeAnnotated(DiagnosticEngine& diag, const ClassLikeDecl& decl)
{
    Ptr<Decl> parentDecl;

    for (auto& parentType : decl.inheritedTypes) {
        auto pty = parentType->ty;
        if (auto parent = DynamicCast<ClassTy>(pty)) {
            parentDecl = parent->decl;
        } else if (auto parentI = DynamicCast<InterfaceTy>(pty)) {
            parentDecl = parentI->decl;
        }
        if (parentDecl && IsMirror(*parentDecl)) {
            break;
        }
    }

    diag.DiagnoseRefactor(DiagKindRefactor::sema_java_mirror_subtype_must_be_annotated, decl, parentDecl->identifier);
}

void DiagJavaDeclCannotInheritPureCodiraType(DiagnosticEngine& diag, ClassLikeDecl& decl)
{
    CODEC_ASSERT(IsMirror(decl) || IsImpl(decl));

    auto kind = IsMirror(decl) ? DiagKindRefactor::sema_java_mirror_cannot_inherit_pure_codira_type
                               : DiagKindRefactor::sema_java_impl_cannot_inherit_pure_codira_type;

    auto builder = diag.DiagnoseRefactor(kind, decl);

    for (const auto& superType : decl.inheritedTypes) {
        auto superDecl = Ty::GetDeclOfTy(superType->ty);
        CODEC_ASSERT(superDecl);
        if (!IsMirror(*superDecl) && !IsImpl(*superDecl) && !superDecl->ty->IsObject() && !superDecl->ty->IsAny()) {
            builder.AddNote(*superType, "'" + superType->ToString() + "'" + " is not a java-compatible type");
        }
    }
}

void DiagJavaDeclCannotBeExtendedWithInterface(DiagnosticEngine& diag, ExtendDecl& decl)
{
    auto& ty = *decl.extendedType->ty;
    CODEC_ASSERT(IsMirror(ty) || IsImpl(ty));
    CODEC_ASSERT(!decl.inheritedTypes.empty());
    auto kind = IsMirror(ty) ? DiagKindRefactor::sema_java_mirror_cannot_be_extended_with_interface
                             : DiagKindRefactor::sema_java_impl_cannot_be_extended_with_interface;

    diag.DiagnoseRefactor(kind, decl);
}

const std::string& GetVarKindName(const Decl& varDecl)
{
    static const std::string GLOBAL_VAR_NAME = "global variable";
    static const std::string MEMBER_VAR_NAME = "member variable";
    static const std::string ENUM_CONSTRUCTOR_PARAMETER = "enum constructor parameter";

    if (varDecl.TestAttr(Attribute::GLOBAL)) {
        return GLOBAL_VAR_NAME;
    } else if (varDecl.outerDecl && varDecl.outerDecl->TestAttr(Attribute::IN_ENUM)) {
        return ENUM_CONSTRUCTOR_PARAMETER;
    } else if (varDecl.outerDecl) {
        return MEMBER_VAR_NAME;
    } else {
        CODEC_ABORT();
        static const std::string UNDEFINED = "";
        return UNDEFINED;
    }
}

const std::string& GetOuterDeclKindName(const Decl& outerDecl)
{
    static const std::string CLASS_NAME = "class";
    static const std::string ENUM_NAME = "enum";
    static const std::string STRUCT_NAME = "struct";

    switch (outerDecl.astKind) {
        case ASTKind::CLASS_DECL:
            return CLASS_NAME;
        case ASTKind::STRUCT_DECL:
            return STRUCT_NAME;
        case ASTKind::ENUM_DECL:
            return ENUM_NAME;
        default:
            CODEC_ABORT();
            static const std::string UNDEFINED = "";
            return UNDEFINED;
    }
}

void DiagUsageOfJavaTypes(
    DiagnosticEngine& diag, const Decl& varDecl, std::vector<Ptr<Decl>>&& javaDecls, Ptr<Decl> nonJavaOuterDecl)
{
    if (javaDecls.empty()) {
        return;
    }

    auto primaryDiagJavaDecl = javaDecls.back();
    javaDecls.pop_back();

    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_variable_of_java_type, varDecl, GetVarKindName(varDecl),
        primaryDiagJavaDecl->identifier);

    for (auto javaDecl : javaDecls) {
        builder.AddNote("Also uses java interoperability type '" + javaDecl->identifier + "'");
    }

    if (nonJavaOuterDecl) {
        builder.AddNote(*nonJavaOuterDecl,
            "Declared inside non java interoperability " + GetOuterDeclKindName(*nonJavaOuterDecl) + " '" +
                nonJavaOuterDecl->identifier + "'");
    }
}

void DiagJavaTypesAsGenericParam(DiagnosticEngine& diag, const Node& expr, std::vector<Ptr<Decl>>&& javaDecls)
{
    if (javaDecls.empty()) {
        return;
    }

    auto primaryDiagJavaDecl = javaDecls.back();
    javaDecls.pop_back();

    auto genericDecl = expr.GetTarget();
    CODEC_ASSERT(genericDecl);

    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_generic_parameter_of_java_type, expr,
        genericDecl->identifier, primaryDiagJavaDecl->identifier);

    for (auto javaDecl : javaDecls) {
        builder.AddNote("Also uses java interoperability type '" + javaDecl->identifier + "'");
    }
}

} // namespace Codira::Interop::Java
