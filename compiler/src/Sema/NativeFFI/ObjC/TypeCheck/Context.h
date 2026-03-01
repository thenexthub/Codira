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
 * This file declares context for typechecking pipeline of Objective-C mirror/subtype declarations.
 */

#ifndef CODIRA_SEMA_OBJ_C_TYPECHECK_CONTEXT
#define CODIRA_SEMA_OBJ_C_TYPECHECK_CONTEXT

#include "NativeFFI/ObjC/Utils/TypeMapper.h"
#include "Codira/AST/Node.h"
#include "Codira/Basic/DiagnosticEngine.h"

namespace Codira::Interop::ObjC {

struct TypeCheckContext {
    explicit TypeCheckContext(AST::Decl& target, DiagnosticEngine& diag, TypeMapper& typeMapper)
        : target(target), diag(diag), typeMapper(typeMapper)
    {
    }

    AST::Decl& target;
    DiagnosticEngine& diag;
    TypeMapper& typeMapper;
};

} // namespace Codira::Interop::ObjC

#endif // CODIRA_SEMA_OBJ_C_TYPECHECK_CONTEXT
