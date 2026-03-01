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
#include "Codira/CHIR/AST2CHIR/Utils.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/Modules/ModulesUtils.h"

using namespace Codira::CHIR;
using namespace Codira;

Ptr<Value> Translator::Visit(const AST::StructDecl& decl)
{
    auto def = GetNominalSymbolTable(decl);
    CODEC_ASSERT(def->GetCustomKind() == CustomDefKind::TYPE_STRUCT);
    auto structDef = StaticCast<StructDef*>(def.get());

    // step 1: set annotation info
    CreateAnnotationInfo<StructDef>(decl, *structDef, structDef);

    // set type
    auto chirType = StaticCast<StructType*>(chirTy.TranslateType(*decl.ty));
    structDef->SetType(*chirType);
    structDef->Set<LinkTypeInfo>(decl.TestAttr(AST::Attribute::GENERIC_INSTANTIATED)
            ? Linkage::INTERNAL
            : decl.linkage);

    // translate func and prop
    const auto& memberDecl = decl.GetMemberDeclPtrs();
    for (auto& member : memberDecl) {
        if (!ShouldTranslateMember(decl, *member)) {
            continue;
        }
        if (member->astKind == AST::ASTKind::VAR_DECL) {
            AddMemberVarDecl(*structDef, *RawStaticCast<const AST::VarDecl*>(member));
        } else if (member->astKind == AST::ASTKind::FUNC_DECL) {
            auto funcDecl = StaticCast<AST::FuncDecl*>(member);
            AddMemberMethodToCustomTypeDef(*funcDecl, *structDef);
        } else if (member->astKind == AST::ASTKind::PROP_DECL) {
            AddMemberPropDecl(*structDef, *RawStaticCast<const AST::PropDecl*>(member));
        } else if (member->astKind == AST::ASTKind::PRIMARY_CTOR_DECL) {
            // do nothing, primary constructor decl has been desugared to func decl
        } else {
            CODEC_ABORT();
        }
    }

    // translate var init
    Translator trans = Copy();
    structDef->SetVarInitializationFunc(trans.TranslateVarsInit(decl));

    // set implemented interface
    for (auto& superInterfaceTy : decl.GetStableSuperInterfaceTys()) {
        auto astType = TranslateType(*superInterfaceTy);
        // The implemented interface type must be of reference type.
        CODEC_ASSERT(astType->IsRef());
        auto realType = StaticCast<ClassType*>(StaticCast<RefType*>(astType)->GetBaseType());
        structDef->AddImplementedInterfaceTy(*realType);
    }

    // collect annotation info of the type and members for annotation target check
    CollectTypeAnnotation(decl, *def);
    return nullptr;
}
