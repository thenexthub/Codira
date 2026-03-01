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

#ifndef CODIRA_FIND_DECL_USAGE
#define CODIRA_FIND_DECL_USAGE

#include <unordered_set>
#include "Codira/AST/Node.h"
#include "Codira/AST/Walker.h"
#include "Constants.h"

namespace ark {
using namespace Codira;
using namespace AST;
bool CheckFunctionEqual(const FuncDecl& srcFunc, const FuncDecl& targetFunc);
bool CheckTypeEqual(Ty& src, Ty& target);
bool CheckDeclEqual(const Decl& source, const Decl& target);
std::unordered_set<Ptr<Node> > FindDeclUsage(const Decl &decl, Node &root, bool isRename = false);
}
#endif
