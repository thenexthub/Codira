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
 * This file declares the JavaMangler used to AST Writer.
 */

#ifndef CODIRA_MODULES_ASTSERIALIZATION_JAVA_MANGLER_H
#define CODIRA_MODULES_ASTSERIALIZATION_JAVA_MANGLER_H

#include "Codira/AST/Types.h"
#include "Codira/Mangle/BaseMangler.h"

namespace Codira {
bool ContainJavaGenerics(const AST::Ty& ty)
{
    if (!AST::Ty::IsTyCorrect(&ty)) {
        return false;
    }
    if (AST::IsJClassOrInterface(ty) && !ty.typeArgs.empty()) {
        return true;
    }
    for (auto tyArg : ty.typeArgs) {
        if (ContainJavaGenerics(*tyArg)) {
            return true;
        }
    }
    return false;
}

class JavaMangler : public BaseMangler {
public:
    bool NeedRemangle(const AST::FuncDecl& funcDecl) const override
    {
        bool isMemberFunc = funcDecl.outerDecl &&
            funcDecl.TestAnyAttr(AST::Attribute::IN_CLASSLIKE, AST::Attribute::IN_ENUM, AST::Attribute::IN_STRUCT,
                AST::Attribute::IN_EXTEND);
        return exportIdMode &&
            (ContainJavaGenerics(*funcDecl.ty) || (isMemberFunc && ContainJavaGenerics(*funcDecl.outerDecl->ty)));
    }
};
} // namespace Codira
#endif
