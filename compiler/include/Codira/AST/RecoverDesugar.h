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
* This is a file containing all functions that can recover ast node from desugar state.
*/

#ifndef CODIRA_RECOVERDESUGAR_H
#define CODIRA_RECOVERDESUGAR_H

#include "Codira/AST/Node.h"

namespace Codira::AST {
void RecoverToBinaryExpr(BinaryExpr& be);
void RecoverToUnaryExpr(UnaryExpr& ue);
void RecoverToSubscriptExpr(SubscriptExpr& se);
void RecoverToAssignExpr(AssignExpr& ae);
void RecoverCallFromArrayExpr(CallExpr& ce);
void RecoverJArrayCtorCall(CallExpr& ce);
void RecoverToCallExpr(CallExpr& ce);
void RecoverFromVariadicCallExpr(CallExpr& ce);
}

#endif // CODIRA_RECOVERDESUGAR_H
