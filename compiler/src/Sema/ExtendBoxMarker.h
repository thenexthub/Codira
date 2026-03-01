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
 *  This file declares a class for extend box marker.
 */

#ifndef CODIRA_SEMA_EXTEND_BOX_MARKER_H
#define CODIRA_SEMA_EXTEND_BOX_MARKER_H

#include <functional>
#include <mutex>

#include "Codira/AST/Node.h"
#include "Codira/AST/Walker.h"
#include "Codira/Sema/TypeManager.h"

namespace Codira {
class ExtendBoxMarker {
public:
    static std::function<AST::VisitAction(Ptr<AST::Node>)> GetMarkExtendBoxFunc(TypeManager& typeMgr);
    /**
     * Return true if @p selectorTy and @p patternTy must be treated as unboxing downcast.
     * NOTE: for special case:
     *       interface I {}; class A <: I {}
     *       func f(any: Any) {
     *           let i : I = (any as I).getOrThrow()
     *           let v = i as A
     *       }
     *       main() {
     *           f(A())
     *       }
     * If do not downcast with box, the conversion 'i as A' will be failed since the instance is actually 'BOX_A'.
     * So, when converting interface to class, we always need to check both boxed condition and non-box condition.
     * And for another case:
     * If selector's type is interface and the pattern's type is class,
     * the pattern must be boxed even extension relation checking failed,
     * since the instance may be subclass of pattern's type which has extend relation with the interface.
     * eg. open class A {}; class B <: A {}
     *     interface I {}; extend B <: I {}
     *     let ins : I = B()
     * then 'ins is A' should be true.
     */
    static bool MustUnboxDownCast(const AST::Ty& selectorTy, const AST::Ty& patternTy)
    {
        return selectorTy.IsInterface() && patternTy.IsClass();
    }
    static std::mutex mtx; // Used to keep marker working as thread-safe.

private:
    static AST::VisitAction MarkBoxPointHandleVarDecl(AST::VarDecl& vd);
    static AST::VisitAction MarkBoxPointHandleAssignExpr(AST::AssignExpr& ae);
    static AST::VisitAction MarkBoxPointHandleCallExpr(AST::CallExpr& ce);
    static AST::VisitAction MarkBoxPointHandleIfExpr(AST::IfExpr& ie);
    static AST::VisitAction MarkBoxPointHandleWhileExpr(const AST::WhileExpr& we);
    static void MarkBoxPointHandleCondition(AST::Expr& e);
    static AST::VisitAction MarkBoxPointHandleReturnExpr(AST::ReturnExpr& re);
    static AST::VisitAction MarkBoxPointHandleArrayLit(AST::ArrayLit& lit);
    static AST::VisitAction MarkBoxPointHandleTryExpr(AST::TryExpr& te);
    static AST::VisitAction MarkBoxPointHandleMatchExpr(AST::MatchExpr& me);
    static AST::VisitAction MarkBoxPointHandleArrayExpr(AST::ArrayExpr& ae);
    static AST::VisitAction MarkBoxPointHandleTupleLit(AST::TupleLit& tl);
    static bool IsTypePatternNeedBox(Ptr<AST::Pattern> pattern, AST::Ty& selectorTy);
    static bool NeedAutoBox(Ptr<AST::Ty> child, Ptr<AST::Ty> interface, bool isUpcast = true);
    static void CheckBlockNeedBox(const AST::Block& block, AST::Ty& ty, AST::Node& nodeToCheck);

    static TypeManager* typeManager;
};
} // namespace Codira
#endif
