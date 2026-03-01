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
 * This file declares core context for the core handlers of Codira <-> Objective-C interopability.
 */

#ifndef CODIRA_SEMA_DESUGAR_OBJ_C_INTEROP_INTEROP_CONTEXT
#define CODIRA_SEMA_DESUGAR_OBJ_C_INTEROP_INTEROP_CONTEXT

#include "Codira/AST/Node.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Mangle/BaseMangler.h"
#include "Codira/Modules/ImportManager.h"
#include "Codira/Sema/TypeManager.h"
#include "NativeFFI/ObjC/Utils/ASTFactory.h"
#include "NativeFFI/ObjC/Utils/InteropLibBridge.h"
#include "NativeFFI/ObjC/Utils/NameGenerator.h"
#include "NativeFFI/ObjC/Utils/TypeMapper.h"

namespace Codira::Interop::ObjC {

struct InteropContext {
    explicit InteropContext(
        AST::Package& pkg, TypeManager& typeManager, ImportManager& importManager, DiagnosticEngine& diag,
        const BaseMangler& mangler,
        const std::string& codeLibOutputPath)
        : pkg(pkg), diag(diag), typeManager(typeManager), importManager(importManager), bridge(importManager, diag),
          typeMapper(bridge, typeManager), mangler(mangler), nameGenerator(mangler),
          factory(bridge, typeManager, nameGenerator, typeMapper, importManager),
          codeLibOutputPath(codeLibOutputPath)
    {
    }

    AST::Package& pkg;
    std::vector<Ptr<AST::ClassLikeDecl>> mirrors;
    std::vector<Ptr<AST::FuncDecl>> mirrorTopLevelFuncs;
    std::vector<Ptr<AST::ClassDecl>> impls;
    std::vector<OwnedPtr<AST::Decl>> genDecls;

    DiagnosticEngine& diag;
    TypeManager& typeManager;
    ImportManager& importManager;
    InteropLibBridge bridge;
    TypeMapper typeMapper;
    const BaseMangler& mangler;
    NameGenerator nameGenerator;
    ASTFactory factory;
    const std::string& codeLibOutputPath;
};

} // namespace Codira::Interop::ObjC

#endif // CODIRA_SEMA_DESUGAR_OBJ_C_INTEROP_INTEROP_CONTEXT
