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
* This file declares the method of determining the reference type.
*/

#ifndef CODIRA_AST_REFERENCETYPE_H
#define CODIRA_AST_REFERENCETYPE_H

#include "Codira/AST/Types.h"

namespace Codira {
namespace AST {
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
inline bool IsReferenceType(const AST::Ty& ty)
{
    // With CODENATIVE-BE, we translate
    // 1) Classes,
    // 2) Option<Ref>,
    // 3) Arrays
    // as reference types.
    bool isRefOption = false;

    if (ty.IsCoreOptionType()) {
        auto& enumTy = static_cast<const AST::EnumTy&>(ty);
        auto elemTy = enumTy.typeArgs[0];
        bool elemTyIsOption = elemTy->IsCoreOptionType();
        // notice that Option<Option<Ref>> is not reference type.
        isRefOption = IsReferenceType(*elemTy) && !elemTyIsOption;
    }
    return ty.IsClassLike() || ty.IsArray() || isRefOption;
}
#endif
} // namespace AST
} // namespace Codira
#endif // CODIRA_AST_REFERENCETYPE_H
