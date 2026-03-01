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
 * InstantiatedExtendRecorder is the class to check used extend decl for each sema type.
 * NOTE: this should be used before instantiated pointer rearrange.
 */

#ifndef CODIRA_SEMA_INSTANTIATED_EXTEND_RECORDER_H
#define CODIRA_SEMA_INSTANTIATED_EXTEND_RECORDER_H

#include "GenericInstantiationManagerImpl.h"

#include "Promotion.h"
#include "Codira/AST/Node.h"
#include "Codira/Sema/TypeManager.h"

namespace Codira {
class GenericInstantiationManager::InstantiatedExtendRecorder {
public:
    InstantiatedExtendRecorder(GenericInstantiationManagerImpl& gim, TypeManager& tyMgr);
    ~InstantiatedExtendRecorder() = default;

    void operator()(AST::Node& node);

private:
    /** Walker function to record extend. */
    AST::VisitAction RecordUsedExtendDecl(AST::Node& node);
    void RecordExtendForRefExpr(const AST::RefExpr& re);
    void RecordExtendForMemberAccess(const AST::MemberAccess& ma);
    void RecordImplExtendDecl(AST::Ty& ty, AST::FuncDecl& fd, Ptr<AST::Ty> upperTy);

    GenericInstantiationManagerImpl& gim;
    TypeManager& typeManager;
    Promotion promotion;

    unsigned recorderId;
    std::function<AST::VisitAction(Ptr<AST::Node>)> extendRecorder;
};
} // namespace Codira
#endif
