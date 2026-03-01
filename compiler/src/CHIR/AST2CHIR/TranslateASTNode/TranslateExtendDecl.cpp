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
#include "Codira/CHIR/Type/ExtendDef.h"
#include "Codira/Modules/ModulesUtils.h"

using namespace Codira::CHIR;
using namespace Codira;

Ptr<Value> Translator::Visit(const AST::ExtendDecl& decl)
{
    CODEC_NULLPTR_CHECK(decl.extendedType);
    auto extendDef = StaticCast<ExtendDef*>(GetNominalSymbolTable(decl));

    // step 1: set annotation info
    CreateAnnotationInfo<ExtendDef>(decl, *extendDef, extendDef);

    // step 2: set extended type
    auto extendedTy = chirTy.TranslateType(*decl.extendedType->ty);
    if (extendedTy->IsRef()) {
        extendedTy = StaticCast<RefType*>(extendedTy)->GetBaseType();
    }
    extendDef->SetExtendedType(*extendedTy);

    // step 3: set member func
    for (auto& member : decl.members) {
        if (member->IsCommonMatchedWithPlatform()) {
            /**
             * Source Definitions:
             *   // common.code
             *   common extend A {
             *     common func foo:Unit {}
             *   }
             *
             *   // platform.code
             *   platform extend A {
             *     platform func foo:Unit { println("hello") }
             *   }
             *
             * After Sema Merge:
             *   platform extend A {
             *     common func foo:Unit;     // Declaration from common extend
             *     platform func foo:Unit {  // Implementation from platform extend
             *       println("hello")
             *     }
             *   }
             * Note:
             * The common declaration of `foo` should be skiped because it is already covered by the
             * platform-specific implementation. This ensures that the platform implementation is used, avoiding
             * redundancy.
             */
            continue;
        }
        if (member->astKind == AST::ASTKind::FUNC_DECL) {
            auto func = VirtualCast<FuncBase*>(GetSymbolTable(*member));
            extendDef->AddMethod(func);
            auto funcDecl = StaticCast<AST::FuncDecl*>(member.get());
            for (auto& param : funcDecl->funcBody->paramLists[0]->params) {
                if (param->desugarDecl != nullptr) {
                    extendDef->AddMethod(VirtualCast<FuncBase>(GetSymbolTable(*param->desugarDecl)));
                    auto it = genericFuncMap.find(param->desugarDecl.get().get());
                    if (it != genericFuncMap.end()) {
                        for (auto instFunc : it->second) {
                            CODEC_NULLPTR_CHECK(instFunc->outerDecl);
                            CODEC_ASSERT(instFunc->outerDecl == &decl);
                            extendDef->AddMethod(VirtualCast<FuncBase*>(GetSymbolTable(*instFunc)));
                        }
                    }
                }
            }
            auto it = genericFuncMap.find(funcDecl);
            if (it != genericFuncMap.end()) {
                for (auto instFunc : it->second) {
                    CODEC_NULLPTR_CHECK(instFunc->outerDecl);
                    CODEC_ASSERT(instFunc->outerDecl == &decl);
                    extendDef->AddMethod(VirtualCast<FuncBase*>(GetSymbolTable(*instFunc)));
                }
            }
            CreateAnnoFactoryFuncsForFuncDecl(StaticCast<AST::FuncDecl>(*member), extendDef);
        } else if (member->astKind == AST::ASTKind::PROP_DECL) {
            AddMemberPropDecl(*extendDef, *RawStaticCast<const AST::PropDecl*>(member.get()));
        } else {
            CODEC_ABORT();
        }
    }

    // step 4: set implemented interface
    for (auto& superType : decl.GetStableSuperInterfaceTys()) {
        auto someTy = TranslateType(*superType);
        auto realType = StaticCast<RefType*>(someTy)->GetBaseType();
        extendDef->AddImplementedInterfaceTy(*StaticCast<ClassType*>(realType));
    }

    // step 5: fill upper bounds
    if (decl.TestAttr(AST::Attribute::GENERIC)) {
        CODEC_NULLPTR_CHECK(decl.generic);
        auto genericDecl = decl.generic.get();
        for (auto& genericTy : genericDecl->typeParameters) {
            chirTy.FillGenericArgType(*StaticCast<AST::GenericsTy*>(genericTy->ty));
        }
    }

    // step 6: collect annotation info of the type and members for annotation target check
    CollectTypeAnnotation(decl, *extendDef);
    return nullptr;
}
