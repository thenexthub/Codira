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

#include "GenerateJavaMirror.h"
#include "Codira/AST/Utils.h"
#include "Codira/AST/ASTCasting.h"
#include "Codira/Utils/CastingTemplate.h"
#include "Utils.h"
#include "TypeCheckUtil.h"
#include "Codira/AST/Match.h"


namespace Codira::Interop::Java {

    void PrepareTypeCheck(Package& pkg, const ImportManager& importManager, TypeManager& typeManager)
    {
        for (auto& file : pkg.files) {
            for (auto& decl : file->decls) {
                if (IsJObject(*decl, pkg.fullPackageName)) {
                    if (auto cd = DynamicCast<ClassDecl*>(decl.get())) {
                        InsertJavaRefGetterStubWithBody(*cd);
                    }
                }
                if (IsMirror(*decl)) {
                    if (auto cd = DynamicCast<ClassDecl*>(decl.get())) {
                        InsertMirrorVarProp(*cd, Attribute::JAVA_MIRROR);
                    } else if (auto id = As<ASTKind::INTERFACE_DECL>(decl.get())) {
                        RemoveAbstractAttributeForJavaHasDefaultMethods(*id);
                        InsertJavaHasDefaultMethodStubs(*id, importManager, typeManager);
                    }
                }
            }
        }
    }
}
