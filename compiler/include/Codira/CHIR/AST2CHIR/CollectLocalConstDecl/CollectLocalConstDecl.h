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

#ifndef CODIRA_CHIR_COLLECT_LOCAL_CONST_DECL_H
#define CODIRA_CHIR_COLLECT_LOCAL_CONST_DECL_H

#include <vector>

#include "Codira/AST/Node.h"

namespace Codira::CHIR {
class CollectLocalConstDecl {
public:
    /**
    * @brief collect local const decls
    *
    * @param decls AST decls
    * @param rootIsGlobalDecl 1st param is global or local decl
    */
    void Collect(const std::vector<Ptr<const AST::Decl>>& decls, bool rootIsGlobalDecl);
    const std::vector<const AST::VarDecl*>& GetLocalConstVarDecls() const;
    const std::vector<const AST::FuncDecl*>& GetLocalConstFuncDecls() const;

private:
    std::vector<const AST::VarDecl*> localConstVarDecls;
    std::vector<const AST::FuncDecl*> localConstFuncDecls;
};
}

#endif
