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

package std.containers;

export namespace containers {
/**
 * BlockingQueue interface
 */
    export interface BlockingQueue<T> {
        /**
        * Inserts element at the end. If the queue is full, blocking.
        *
        * @param { T } element the element pushed to the BlockingQueue
        */
        push(element: T): void;

        /**
        * Removes and returns the first element in the BlockingQueue. If the queue is empty, blocking.
        *
        * @returns { T } the fisrt element poped in the queue.
        */
        pop(): T;

        /**
        * The insert operation​. Returns true upon success and false if no space is currently available,​ no blocking.
        *
        * @param { T } element the element inserted to the BlockingQueue
        *
        * @returns { boolean } True upon success and false if no space is currently available.
        */
        add(element: T): boolean;

        /**
        * The poll operation. Pop one element and return it if the queue is not empty.
        * Returns undefined when the queue is empty. No blocking.
        *
        * @returns { T | undefined } the deleted element or undefined when the queue is empty.
        */
        poll(): T | undefined;

        /**
        * Returns the first element in a BlockingQueue.
        *
        * @returns { T | undefined } the first element or undefined when the queue is empty.
        */
        getFirst(): T | undefined;

        /**
        * Checks if the BlockingQueue is empty.
        *
        * @returns { boolean } True if a BlockingQueue has no elements, otherwise false.
        */
        isEmpty(): boolean;

        /**
        * Returns the size of a BlockingQueue.
        *
        * @returns { int } the size of the BlockingQueue.
        */
        get size(): int;

        /**
        * Returns the capacity of a BlockingQueue.
        *
        * @returns { int } the capacity of the BlockingQueue.
        */
        get capacity(): int;

        /**
        * Returns the number of additional elements that this queue can ideally accept without blocking.
        *
        * @returns { int } the remainingcapacity of the BlockingQueue.
        */
        remainingCapacity(): int;
    }
}
