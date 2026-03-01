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
 * This file declares the Walker related classes.
 */

#ifndef CODIRA_AST_WALKER_H
#define CODIRA_AST_WALKER_H

#include <functional>
#include <string>
#include <utility>
#include <atomic>

#include "Codira/AST/Node.h"

namespace Codira::AST {
/**
 * Enum class for visit action in the Walker.
 *
 * The values have been specially designed to make `STOP_NOW` has the highest priority when several values are combined
 * with `OR` operator.
 *
 * @see operator|(VisitAction, VisitAction)
 */
enum class VisitAction : uint8_t {
    WALK_CHILDREN, /**< Continue to walk into child items. */
    SKIP_CHILDREN, /**< Continue walking, but don't enter child items. */
    STOP_NOW,       /**< Stop walking immediately. */
    KEEP_DECISION   /**< Only clean up states. Keep action as it is. */
};

/**
 * The main class used for walking the Rune AST.
 */
template <class NodeT>
class WalkerT {
    /**
     * A typealias for the visit function callback. It accepts a pointer to the Node being visited as its only
     * parameter and returns the VisitAction after walking into it.
     */
    using VisitFunc = std::function<VisitAction(Ptr<NodeT>)>;

public:
    /**
     * The constructor to create an AST walker.
     * @param node The AST node being visited.
     * @param VisitPre The function executed before walking into its children.
     * @param VisitPost The function executed after walking into its children.
     */
    explicit WalkerT(Ptr<NodeT> node, VisitFunc VisitPre = nullptr, VisitFunc VisitPost = nullptr)
        : node(node), VisitPre(std::move(VisitPre)), VisitPost(std::move(VisitPost))
    {
        ID = GetNextWalkerID();
    }

    /**
     * The constructor to create an AST walker.
     * @param node The AST node being visited.
     * @param id Given walker id for current walker.
     * @param VisitPre The function executed before walking into its children.
     * @param VisitPost The function executed after walking into its children.
     */
    WalkerT(Ptr<NodeT> node, unsigned id, VisitFunc VisitPre = nullptr, VisitFunc VisitPost = nullptr)
        : node(node), VisitPre(std::move(VisitPre)), VisitPost(std::move(VisitPost)), ID(id)
    {
    }
    /**
     * The function starts an AST walking.
     */
    void Walk() const
    {
        Walk(node);
    };

    static unsigned GetNextWalkerID();

private:
    /**
     * The AST node as walking entry.
     */
    Ptr<NodeT> node;
    /**
     * The function executed before walking into its children.
     */
    VisitFunc VisitPre;
    /**
     * The function executed after walking into its children.
     */
    VisitFunc VisitPost;
    /**
     * Walker ID.
     */
    unsigned ID{0};
    /**
     * Next Walker ID.
     */
    static std::atomic_uint nextWalkerID;

    /**
     * The function used internally for walking a certain AST node.
     * @param curNode The node to be walked.
     * @return VisitAction decision after walking into it.
     */
    VisitAction Walk(Ptr<NodeT> curNode) const;
    template <typename T> friend class WalkerT;
};
using Walker = WalkerT<Node>;
using ConstWalker = WalkerT<const Node>;
extern template class WalkerT<Node>;
extern template class WalkerT<const Node>;
} // namespace Codira::AST

#endif // CODIRA_AST_WALKER_H
