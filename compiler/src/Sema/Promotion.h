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
 * This file declares a class providing functions for promoting a subtype to a designated supertype if possible.
 * It also provides utility functions that handle type substitutions.
 */

#ifndef CODIRA_SEMA_PROMOTION_H
#define CODIRA_SEMA_PROMOTION_H

#include "Codira/AST/Types.h"
#include "Codira/Sema/TypeManager.h"

namespace Codira {
class Promotion {
public:
    explicit Promotion(TypeManager& tyMgr) : tyMgr(tyMgr)
    {
    }
    MultiTypeSubst GetPromoteTypeMapping(AST::Ty& from, AST::Ty& target);
    MultiTypeSubst GetDowngradeTypeMapping(AST::Ty& target, AST::Ty& upfrom);
    std::set<Ptr<AST::Ty>> Promote(AST::Ty& from, AST::Ty& target);
    // will return empty if any type arg of target (the subtype) unused in upfrom (the supertype)
    // e.g. downgrading to Future<T> from Any
    std::set<Ptr<AST::Ty>> Downgrade(AST::Ty& target, AST::Ty& upfrom);

private:
    TypeManager& tyMgr;
    std::set<Ptr<AST::Ty>> PromoteHandleIdealTys(AST::Ty& from, AST::Ty& target) const;
    std::set<Ptr<AST::Ty>> PromoteHandleFunc(AST::Ty& from, AST::Ty& target);
    std::set<Ptr<AST::Ty>> PromoteHandleTuple(AST::Ty& from, AST::Ty& target);
    std::set<Ptr<AST::Ty>> PromoteHandleTyVar(AST::Ty& from, AST::Ty& target);
    std::set<Ptr<AST::Ty>> PromoteHandleNominal(AST::Ty& from, const AST::Ty& target);
};
} // namespace Codira
#endif // CODIRA_SEMA_PROMOTION_H
