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

#include "InheritanceChecker.h"

#include "NativeFFI/Java/AfterTypeCheck/Utils.h"
#include "Codira/AST/Clone.h"

namespace Codira::Interop::Java {

namespace {

Ptr<Annotation> GetForeignNameAnnotation(const Decl& decl)
{
    auto it = std::find_if(decl.annotations.begin(), decl.annotations.end(),
        [](const auto& anno) { return anno->kind == AnnotationKind::FOREIGN_NAME; });
    return it != decl.annotations.end() ? it->get() : nullptr;
}

std::string GetAnnoValue(Ptr<Annotation> anno)
{
    CODEC_ASSERT(anno);
    CODEC_ASSERT(anno->args.size() == 1);

    auto litExpr = DynamicCast<LitConstExpr>(anno->args[0]->expr.get());
    CODEC_ASSERT(litExpr);
    return litExpr->stringValue;
}

void DiagConflictingForeignName(
    DiagnosticEngine& diag, const Decl& declWithAnno, const Decl& otherDecl, const Decl& checkingDecl)
{
    auto anno = GetForeignNameAnnotation(declWithAnno);
    CODEC_ASSERT(anno);

    auto builder = [&diag, &anno, &declWithAnno]() {
        if (!anno->TestAttr(Attribute::COMPILER_ADD)) {
            auto declWithAnnoRange = MakeRange(anno->GetBegin(), declWithAnno.identifier.End());
            return diag.DiagnoseRefactor(DiagKindRefactor::sema_foreign_name_conflicting_annotation, declWithAnno,
                declWithAnnoRange, declWithAnno.identifier);
        } else {
            return diag.DiagnoseRefactor(DiagKindRefactor::sema_foreign_name_conflicting_derived_annotation,
                declWithAnno, MakeRange(declWithAnno.identifier), declWithAnno.identifier, GetAnnoValue(anno));
        }
    }();

    auto otherAnno = GetForeignNameAnnotation(otherDecl);
    if (otherAnno && !otherAnno->TestAttr(Attribute::COMPILER_ADD)) {
        auto otherDeclRange = MakeRange(otherAnno->GetBegin(), otherDecl.identifier.End());
        builder.AddNote(
            otherDecl, otherDeclRange, "Other declaration '" + otherDecl.identifier + "' has a different @ForeignName");
    } else if (otherAnno) {
        builder.AddNote(otherDecl, MakeRange(otherDecl.identifier),
            "Other declaration '" + otherDecl.identifier + "' has a different derived @ForeignName '" +
                GetAnnoValue(otherAnno) + "'");
    } else {
        auto otherDeclRange = MakeRange(otherDecl.identifier);
        builder.AddNote(
            otherDecl, otherDeclRange, "Other declaration '" + otherDecl.identifier + "' doesn't have a @ForeignName");
    }

    builder.AddNote(checkingDecl, MakeRange(checkingDecl.identifier),
        "While checking declaration '" + checkingDecl.identifier + "'");
}

bool NeedCheck(const MemberSignature& parent, const MemberSignature& child)
{
    if (child.decl->outerDecl->TestAttr(Attribute::IMPORTED)) {
        return false;
    }
    if (!parent.decl->IsFuncOrProp()) {
        return false;
    }
    CODEC_ASSERT(child.decl->IsFuncOrProp());

    if (!parent.decl->outerDecl->TestAnyAttr(Attribute::JAVA_MIRROR, Attribute::JAVA_MIRROR_SUBTYPE)) {
        return false;
    }
    if (!child.decl->outerDecl->TestAnyAttr(Attribute::JAVA_MIRROR, Attribute::JAVA_MIRROR_SUBTYPE)) {
        // @JavaMirror anottation might be missing here, will report it later
        return false;
    }
    if (parent.decl->outerDecl == child.decl->outerDecl) {
        return false;
    }

    return true;
}

} // namespace

void CheckForeignName(DiagnosticEngine& diag, TypeManager& typeManager, const MemberSignature& parent,
    const MemberSignature& child, const Decl& checkingDecl)
{
    if (!NeedCheck(parent, child)) {
        return;
    }

    auto childAnno = GetForeignNameAnnotation(*child.decl);
    auto parentAnno = GetForeignNameAnnotation(*parent.decl);
    if (!childAnno && !parentAnno) {
        return;
    }

    if (!typeManager.IsSubtype(child.structTy, parent.structTy)) {
        if (!childAnno && parentAnno) {
            DiagConflictingForeignName(diag, *parent.decl, *child.decl, checkingDecl);
        } else if (!parentAnno && childAnno) {
            DiagConflictingForeignName(diag, *child.decl, *parent.decl, checkingDecl);
        } else if (GetAnnoValue(childAnno) != GetAnnoValue(parentAnno)) {
            DiagConflictingForeignName(diag, *parent.decl, *child.decl, checkingDecl);
        }
        return;
    }

    if (childAnno && !childAnno->TestAttr(Attribute::COMPILER_ADD)) {
        auto range = MakeRange(childAnno->GetBegin(), child.decl->identifier.End());
        diag.DiagnoseRefactor(DiagKindRefactor::sema_foreign_name_appeared_in_child, *child.decl, range);
    } else if (childAnno && !parentAnno) {
        DiagConflictingForeignName(diag, *child.decl, *parent.decl, checkingDecl);
    } else if (!childAnno && parentAnno && child.replaceOther) {
        // NOTE: When replaceOther is true, then this method is overriding some other parent one
        // And if there is no ForeignName, then that parent also hadn't ForeignName. But current
        // parent do have it
        DiagConflictingForeignName(diag, *parent.decl, *child.decl, checkingDecl);
    } else if (!childAnno && parentAnno) {
        auto clonedAnno = ASTCloner::Clone(parentAnno);
        clonedAnno->EnableAttr(Attribute::COMPILER_ADD);
        CopyBasicInfo(child.decl, clonedAnno.get());
        child.decl->annotations.emplace_back(std::move(clonedAnno));
    } else if (GetAnnoValue(childAnno) != GetAnnoValue(parentAnno)) {
        DiagConflictingForeignName(diag, *parent.decl, *child.decl, checkingDecl);
    }
}

} // namespace Codira::Interop::Java
