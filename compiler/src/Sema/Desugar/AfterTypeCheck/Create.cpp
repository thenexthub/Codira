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

#include "Desugar/AfterTypeCheck.h"
#include "Codira/AST/Create.h"

#include "TypeCheckUtil.h"

using namespace Codira;
using namespace TypeCheckUtil;

namespace Codira::Sema::Desugar::AfterTypeCheck {
OwnedPtr<TypePattern> CreateRuntimePreparedTypePattern(
    TypeManager& typeManager, OwnedPtr<Pattern> pattern, OwnedPtr<Type>  type, Expr& selector)
{
    auto typePattern = CreateTypePattern(std::move(pattern), std::move(type), selector);
    typePattern->matchBeforeRuntime = typeManager.IsSubtype(selector.ty, typePattern->ty, true, false);
    typePattern->needRuntimeTypeCheck =
        !typePattern->matchBeforeRuntime && IsNeedRuntimeCheck(typeManager, *selector.ty, *typePattern->ty);
    return typePattern;
}
} // namespace Codira::Sema::Desugar::AfterTypeCheck
