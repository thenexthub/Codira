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
 * This file declares class for auto boxing extend and option.
 */
#ifndef CODIRA_SEMA_AUTO_BOX_H
#define CODIRA_SEMA_AUTO_BOX_H

#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Node.h"
#include "Codira/AST/Walker.h"
#include "Codira/Mangle/BaseMangler.h"
#include "Codira/Modules/ImportManager.h"

namespace Codira {
class AutoBoxing {
public:
    explicit AutoBoxing(TypeManager& typeManager) : typeManager(typeManager)
    {
    }
    ~AutoBoxing() = default;

    void AddOptionBox(AST::Package& pkg);

private:
    /** Boxing functions for option box. */
    void TryOptionBox(AST::EnumTy& target, AST::Expr& expr);
    bool NeedBoxOption(AST::Ty& child, AST::Ty& target);
    AST::VisitAction AddOptionBoxHandleReturnExpr(const AST::ReturnExpr& re);
    AST::VisitAction AddOptionBoxHandleVarDecl(const AST::VarDecl& vd);
    AST::VisitAction AddOptionBoxHandleAssignExpr(const AST::AssignExpr& ae);
    AST::VisitAction AddOptionBoxHandleCallExpr(AST::CallExpr& ce);
    AST::VisitAction AddOptionBoxHandleIfExpr(const AST::IfExpr& ie);
    AST::VisitAction AddOptionBoxHandleTryExpr(AST::TryExpr& te);
    AST::VisitAction AddOptionBoxHandleArrayLit(AST::ArrayLit& lit);
    AST::VisitAction AddOptionBoxHandleMatchExpr(AST::MatchExpr& me);
    AST::VisitAction AddOptionBoxHandleArrayExpr(AST::ArrayExpr& ae);
    AST::VisitAction AddOptionBoxHandleTupleList(const AST::TupleLit& tl);
    void AddOptionBoxHandleBlock(AST::Block& block, AST::Ty& ty);

    TypeManager& typeManager;
};
} // namespace Codira

#endif
