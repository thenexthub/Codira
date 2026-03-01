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

#include "StructuralRuleGSER.h"
#include "Codira/Basic/Match.h"

using namespace Codira::CodeCheck;
using namespace Codira::Meta;
using namespace Codira::AST;

void StructuralRuleGSER::FindExtendSer(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker1(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const ExtendDecl &extendDecl) {
                for (auto &it : extendDecl.inheritedTypes) {
                    if ((it->ToString()).find("Serializable<") != std::string::npos) {
                        (void)extendSers.insert(extendDecl.extendedType->ToString());
                        return VisitAction::SKIP_CHILDREN;
                    }
                }
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker1.Walk();
}
