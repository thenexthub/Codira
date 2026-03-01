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

namespace {
    bool ShouldTranslateConstructor(const AST::EnumDecl& decl, const AST::Decl& ctor)
    {
        CODEC_ASSERT(ctor.astKind == AST::ASTKind::VAR_DECL || ctor.astKind == AST::ASTKind::FUNC_DECL);
        if (ctor.TestAttr(AST::Attribute::COMMON) && decl.platformImplementation) {
            return false;
        }
        return true;
    }
}

Ptr<Value> Translator::Visit(const AST::EnumDecl& decl)
{
    auto def = GetNominalSymbolTable(decl);
    CODEC_ASSERT(def->GetCustomKind() == CustomDefKind::TYPE_ENUM);
    auto enumDef = StaticCast<EnumDef*>(def.get());

    // step 1: set annotation info
    CreateAnnotationInfo<EnumDef>(decl, *enumDef, enumDef);

    // step 2: set type
    auto chirType = StaticCast<EnumType*>(TranslateType(*decl.ty));
    enumDef->SetType(*chirType);
    enumDef->Set<LinkTypeInfo>(decl.TestAttr(AST::Attribute::GENERIC_INSTANTIATED) ? Linkage::INTERNAL : decl.linkage);

    // step 3: set constructor
    // e.g. enum A { red | yellow | blue(Int32) }
    // `red`, `yellow` and `blue(Int32)` are called constructors
    // `red` and `yellow` are defined as `VarDecl`, `blue(Int32)` is defined as `FuncDecl`
    for (auto& ctor : decl.constructors) {
        if (!ShouldTranslateConstructor(decl, *ctor)) {
            continue;
        }
        switch (ctor->astKind) {
            case AST::ASTKind::VAR_DECL: {
                // default enum member store as {} -> EnumType
                enumDef->AddCtor(
                    {ctor->identifier, ctor->mangledName, builder.GetType<FuncType>(std::vector<Type*>{}, chirType)});
                break;
            }
            case AST::ASTKind::FUNC_DECL: {
                std::vector<Type*> paramTypes;
                CODEC_ASSERT(!ctor->ty->typeArgs.empty());
                for (size_t i = 0; i < ctor->ty->typeArgs.size() - 1; i++) {
                    if (ctor->ty->typeArgs[i] == decl.ty) {
                        paramTypes.emplace_back(chirType);
                    } else {
                        paramTypes.emplace_back(TranslateType(*ctor->ty->typeArgs[i]));
                    }
                }
                enumDef->
                    AddCtor({ctor->identifier, ctor->mangledName, builder.GetType<FuncType>(paramTypes, chirType)});
                break;
            }
            default: {
                CODEC_ABORT();
                break;
            }
        }
    }

    // step 4: set member func and prop
    for (auto& member : decl.members) {
        if (!ShouldTranslateMember(decl, *member)) {
            continue;
        }
        if (member->astKind == AST::ASTKind::FUNC_DECL) {
            auto funcDecl = StaticCast<AST::FuncDecl*>(member.get());
            AddMemberMethodToCustomTypeDef(*funcDecl, *enumDef);
        } else if (member->astKind == AST::ASTKind::PROP_DECL) {
            AddMemberPropDecl(*enumDef, *RawStaticCast<const AST::PropDecl*>(member.get()));
        } else {
            CODEC_ABORT();
        }
    }

    // step 5: set implemented interface
    for (auto& superInterfaceTy : decl.GetStableSuperInterfaceTys()) {
        auto astType = TranslateType(*superInterfaceTy);
        // The implemented interface type must be of reference type.
        CODEC_ASSERT(astType->IsRef());
        auto realType = StaticCast<ClassType*>(StaticCast<RefType*>(astType)->GetBaseType());
        enumDef->AddImplementedInterfaceTy(*realType);
    }

    // step 6: collect annotation info of the type and members for annotation target check
    CollectTypeAnnotation(decl, *def);
    return nullptr;
}
