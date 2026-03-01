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


#ifndef MRT_LIGHTQUEUE_H
#define MRT_LIGHTQUEUE_H

#include <stdbool.h>
#include <stdlib.h>
#include "mid.h"
#include "macro_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief Bidirectional cyclic linked list node
 */
struct Dulink {
    struct Dulink *prev;
    struct Dulink *next;
};

/**
 * @brief Init Dulink
 * @par Initialize the target bidirectional linked list.
 * The nodes before and after the linked list point to themselves.
 * @param  dulink           [IN]  dulink
 */
MRT_INLINE static void DulinkInit(struct Dulink *dulink)
{
    dulink->next = dulink->prev = dulink;
}

/**
 * @brief Check whether the linked list is empty.
 * @param  dulink           [IN]  dulink
 */
MRT_INLINE static bool DulinkIsEmpty(struct Dulink *dulink)
{
    return dulink->next == dulink;
}

/**
 * @brief Remove a linked list node.
 * @par Remove a linked list node.
 * @param  item             [IN]  node
 */
MRT_INLINE static void DulinkRemove(struct Dulink *item)
{
    item->prev->next = item->next;
    item->next->prev = item->prev;
}

/**
 * @brief Add a node to the linked list.
 * @par Insert the target node behind the selected node.
 * @param  item             [IN]  node
 * @param  where            [IN]  where
 */
MRT_INLINE static void DulinkAdd(struct Dulink *item, struct Dulink *where)
{
    item->next = where->next;
    item->prev = where;
    where->next = item;
    item->next->prev = item;
}

/**
 * @brief Find the start position of the structure that contains a node according to the member structure of the node.
 * @param  item             [IN]  Pointer to a node
 * @param  type             [IN]  Type of the structure that contains the node
 * @param  member           [IN]  Name of a node in a structure
 */
#define DULINK_ENTRY(item, type, member) \
    ((type *)((char *)(item) - (unsigned long long)(&((type *)0)->member)))

/**
 * @brief Add to the end of the linked list
 * @par Add to the end of the linked list
 * @param  dulink           [IN]  head of link
 * @param  object           [IN]  node
 */
MRT_INLINE static void DulinkPushtail(struct Dulink *dulink, void *object)
{
    struct Dulink *newNode = (struct Dulink *)object;
    /* dulink is the head node, and its prev is the tail node. */
    dulink->prev->next = newNode;
    newNode->prev = dulink->prev;
    newNode->next = dulink;
    dulink->prev = newNode;
}

/**
 * @brief Add to the header of the linked list.
 * @par Insert the target node into the header of the linked list.
 * @param  dulink           [IN]  Head node of the linked list
 * @param  object           [IN]  node
 */
MRT_INLINE static void DulinkPushHead(struct Dulink *dulink, void *object)
{
    struct Dulink *node = (struct Dulink *)object;
    node->next = dulink->next;
    node->prev = dulink;
    dulink->next = node;
    node->next->prev = node;
}

/**
 * @ingroup list
 * @brief Delete the tail node of the linked list
 * @param  dulink           [IN]  Linked list of the tail node to be deleted
 */
MRT_INLINE static void DulinkPopTail(struct Dulink *dulink)
{
    struct Dulink *tail = dulink->prev;
    tail->prev->next = tail->next;
    tail->next->prev = tail->prev;
}

/**
 * @brief Delete the head node of the linked list
 * @param  dulink           [IN]  Linked list of the head node to be deleted
 */
MRT_INLINE static void DulinkPopHead(struct Dulink *dulink)
{
    struct Dulink *head = dulink->next;
    head->prev->next = head->next;
    head->next->prev = head->prev;
}

/**
 * @brief Traverse the bidirectional linked list.
 * @param  item             [IN]  Temporary node during traversal
 * @param  head             [IN]  Address of the head node of the linked list
 */
#define DULINK_FOR_EACH_ITEM(item, head) \
    for ((item) = (head)->next; (item) != (head); (item) = (item)->next)

/**
 * @brief Obtain the index node of the linked list according to the forward index.
 * @attention The caller must ensure that the index is reasonable.
 * @param  head             [IN]  head
 * @param  index            [IN]  index
 */
MRT_INLINE static struct Dulink *DulinkGetIndexByNext(struct Dulink* head, int index)
{
    struct Dulink* link = head;
    for (int i = 0; i < index; ++i) {
        link = link->next;
    }
    return link;
}

/**
 * @brief Obtains the index node from the tail of the linked list based on the reverse index.
 * @attention The caller must ensure that the index is reasonable.
 * @param  head             [IN]  head
 * @param  index            [IN]  index
 */
MRT_INLINE static struct Dulink *DulinkGetIndexByPrev(struct Dulink* head, int index)
{
    struct Dulink* link = head;
    for (int i = 0; i < index; ++i) {
        link = link->prev;
    }
    return link;
}

/**
 * @brief Inserting a linked list of nodes in batches
 * @par Insert the linked list starting with beg and ending with end to the head node.
 * @param  head             [IN]  head of link
 * @param  beg              [IN]  head of nodes
 * @param  end              [IN]  end of nodes
 */
MRT_INLINE static void DulinkInsertHead(struct Dulink* head, struct Dulink* beg, struct Dulink* end)
{
    head->next->prev = end;
    end->next = head->next;
    head->next = beg;
    beg->prev = head;
}

/**
 * @brief Move linked list elements in batches
 * @par Add the continuous elements in the header part of the linked list corresponding to the
 * fromHead header node to the linked list corresponding to the toHead header node. If the
 * value of countOfMoveOrReserve is a positive number, the value indicates the number of items
 * to be moved in fromHead. Otherwise, the value of countOfMoveOrReserve indicates the number
 * of items to be retained in fromHead. Note that 0 indicates that all elements in fromHead
 * are added to to toHead.
 * @attention The caller must ensure that countOfMoveOrReserve is reasonable.
 * This interface cannot be invoked when the fromHead linked list is empty.
 * @param  toHead               [IN]  toHead
 * @param  fromHead             [IN]  fromHead
 * @param  countOfMoveOrReserve [IN]  countOfMoveOrReserve
 */
MRT_INLINE static void DulinkMove(struct Dulink* toHead, struct Dulink* fromHead, int countOfMoveOrReserve)
{
    struct Dulink* beg = fromHead->next;
    struct Dulink* end = nullptr;
    if (countOfMoveOrReserve > 0) {
        end = DulinkGetIndexByNext(fromHead, countOfMoveOrReserve);
    } else {
        end = DulinkGetIndexByPrev(fromHead, (-countOfMoveOrReserve) + 1);
    }
    fromHead->next = end->next;
    end->next->prev = fromHead;
    DulinkInsertHead(toHead, beg, end);
}

/**
 * @brief Bidirectional linked list node. The difference between link and
 * dulink is that link is a non-cyclic linked list.
 */
struct Link {
    struct Link *prev;
    struct Link *next;
};

/**

 * @brief Initialize the target bidirectional linked list.
 * @param  link         [IN]  link
 */
MRT_INLINE static void LinkInit(struct Link *link)
{
    link->next = nullptr;
    link->prev = nullptr;
}

/**
 * @brief Remove a linked list node.
 * @param  item         [IN]  node
 */
MRT_INLINE static void LinkRemove(struct Link *item)
{
    if (item->prev != nullptr) {
        item->prev->next = item->next;
    }

    if (item->next != nullptr) {
        item->next->prev = item->prev;
    }
}

/**
 * @brief Add to the header of the linked list.
 * @param  link         [IN]  head
 * @param  object       [IN]  node
 */
MRT_INLINE static void LinkPushHead(struct Link *link, void *object)
{
    struct Link *node = (struct Link *)object;
    node->next = link->next;
    node->prev = link;
    link->next = node;
    if (node->next != nullptr) {
        node->next->prev = node;
    }
}

/**
 * @brief Add to the end of the linked list
 * @param  link         [IN]  end
 * @param  object       [IN]  node
 */
static inline void LinkPushTail(struct Link *linkTail, void *object)
{
    struct Link *node = (struct Link *)object;
    node->next = nullptr;
    linkTail->next = node;
    node->prev = linkTail;
}

/**
 * @brief Find the start position of the structure that contains a node according to
 * the member structure of the node.
 * @param  item         [IN]  Pointer to the node.
 * @param  type         [IN]  Type of the structure that contains the node.
 * @param  member       [IN]  Name of a node in a structure.
 */
#define LINK_ENTRY(item, type, member) \
    ((type *)((char *)(item) - (unsigned long long)(&((type *)0)->member)))


/**
 * @brief Check whether the linked list is empty.
 * @param  dulink       [IN]  dulink
 * @retval If this parameter is left blank, true is returned. Otherwise, false is returned.
 */
MRT_INLINE static bool LinkIsEmpty(struct Link *link)
{
    return ((link->next == nullptr) && (link->prev == nullptr));
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* MRT_LIGHTQUEUE_H */

