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
 * This file declares the MacroExpander related classes, which provides macro expand capabilities.
 */

#ifndef CODIRA_MACROEXPAND_H
#define CODIRA_MACROEXPAND_H

#include <codira/AST/Node.h>
#include <list>

#include "Codira/AST/Node.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Macro/InvokeUtil.h"
#include "Codira/Macro/MacroCommon.h"

namespace Codira {
class MacroExpansion {
public:
    MacroExpansion(CompilerInstance* ci) : ci(ci)
    {
    }
    void Execute(AST::Package& package);
    void Execute(std::vector<OwnedPtr<AST::Package>>& packages);
    // String format of macro generated Tokens, for pretty print.
    std::vector<std::string> tokensEvalInMacro;

private:
    Ptr<AST::Package> curPackage{nullptr};
    CompilerInstance* ci{nullptr};
    MacroCollector macroCollector;

    /**
     * Collect macro placeholder nodes in a package.
     */
    void CollectMacros(AST::Package& package);

    /**
     * Evaluate macro.
     */
    void EvaluateMacros();

    /**
     * Process macro information after macro expansion.
     */
    void ProcessMacros(AST::Package& package);
    /**
     * Replace AST after macro expansion.
     */
    void ReplaceAST(AST::Package& package);

    /**
     * Replace AST helper.
     */
    void ReplaceEachMacro(MacroCall& macCall);

    /**
     * Check attribute if replaced node is enum case member.
     */
    void CheckReplacedEnumCaseMember(MacroCall& macroNode, PtrVector<AST::Decl>& newNodes) const;
    /**
     * Check node Consistency: if all nodes are T.
     */
    template <typename T>
    void CheckNodesConsistency(
        PtrVector<T>& nodes, PtrVector<AST::Node>& newNodes, VectorTarget<OwnedPtr<T>>& target) const;
    void ReplaceDecls(
        MacroCall& macroNode, PtrVector<AST::Node>& newNodes, VectorTarget<OwnedPtr<AST::Decl>>& target) const;
    void ReplaceParams(MacroCall& macroNode, PtrVector<AST::Node>& newNodes,
        VectorTarget<OwnedPtr<AST::FuncParam>>& target) const;
    /**
     * Check FuncParamList legality.
     */
    void CheckReplacedFuncParamList(
        const MacroCall& macroNode, const VectorTarget<OwnedPtr<AST::FuncParam>>& target) const;
    /**
     * Replace each macro node to target position.
     */
    void ReplaceEachMacroHelper(MacroCall& macroNode, PtrVector<AST::Node>& newNodes) const;
    /**
     * Replace File Node.
     */
    void ReplaceEachFileNode(const AST::File& file);
};
} // namespace Codira

#endif
