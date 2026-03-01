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
 * This file declares the CHIRMangle.
 */

#include "Codira/Mangle/CHIRMangler.h"

#include "Codira/AST/Node.h"

using namespace Codira;
using namespace CHIR;
using namespace AST;
using namespace MangleUtils;


std::optional<std::string> CHIRMangler::MangleEntryFunction(const FuncDecl& funcDecl) const
{
    // Change user main function to user.main, so that the function entry can be changed to RuntimeMain.
    // Initialization of light-weight thread scheduling can be performed in runtime.main.
    if (enableCompileTest && funcDecl.identifier == TEST_ENTRY_NAME) {
        return USER_MAIN_MANGLED_NAME;
    }
    if (!enableCompileTest && (funcDecl.identifier == MAIN_INVOKE && funcDecl.IsStaticOrGlobal())) {
        return USER_MAIN_MANGLED_NAME;
    }
    return std::nullopt;
}

std::string CHIRMangler::MangleCFuncSignature(const AST::FuncTy& cFuncTy) const
{
    std::string mangled = MangleType(*cFuncTy.retTy);
    for (auto paramTy : cFuncTy.paramTys) {
        mangled += MangleType(*paramTy);
    }
    return mangled;
}
