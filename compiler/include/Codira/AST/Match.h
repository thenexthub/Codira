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
 * This file declares AST match apis, and some useful macros.
 */

#ifndef CODIRA_AST_MATCH_H
#define CODIRA_AST_MATCH_H

#include "Codira/AST/NodeX.h"
#include "Codira/AST/ASTCasting.h"

namespace Codira::AST {
/**
 * ASTKind to Node type mapping.
 */
template <ASTKind Kind> struct NodeKind {};
#define ASTKIND(KIND, VALUE, NODE, SIZE)                                                                               \
    template <> struct NodeKind<ASTKind::KIND> {                                                                       \
        using Type = AST::NODE;                                                                                        \
    };
#include "Codira/AST/ASTKind.inc"
#undef ASTKIND

/**
 * Convert Node to certain ASTKind, use static_cast as possible.
 * @param node Node to be convert.
 * @return The AST Node of kind @p Kind.
 */
template <ASTKind Kind> auto As(Ptr<Node> node)
{
    return DynamicCast<typename NodeKind<Kind>::Type*>(node.get());
}

/**
 * Convert Node to certain ASTKind, use static_cast.
 * @param node Node to be convert.
 * @return The AST Node of kind @p Kind.
 */
template <ASTKind Kind, typename NodeT> inline auto StaticAs(NodeT node)
{
    return StaticCast<typename NodeKind<Kind>::Type*>(node);
}
} // namespace Codira::AST
#endif // CODIRA_AST_MATCH_H
