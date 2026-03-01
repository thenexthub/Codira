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

import Deque from "@ohos.util.Deque";

export type QueueForEachCb<T> = (value: T, index: int, queue: Queue<T>) => void;

export class Queue<T> implements Iterable<T> {
    private buffer: Deque<T>;

    public toString(): string {
        return "[object Queue]";
    }

    /**
     * Gets the number of elements in the Queue.
     */
    get length(): int {
        return this.buffer.length.toInt();
    }

    /**
     * Retrieves the first element in the Queue without removing it.
     * @returns The first element in the Queue, or undefined if the Queue is empty.
     */
    public getFirst(): T {
        return this.buffer.getFirst();
    }

    /**
     * Removes and returns the first element in the Queue.
     * @returns The first element in the Queue, or undefined if the Queue is empty.
     */
    public pop(): T {
        return this.buffer.popFirst();
    }

    /**
     * Creates a new instance of Queue.
     */
    public constructor() {
        this.buffer = new Deque<T>();
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param callbackfn - The function to execute on each element, taking the value, index, and Queue as arguments.
     */
    public forEach(callbackfn: QueueForEachCb<T>): void {
        this.buffer.forEach(((val: T, idx) => callbackfn(val, idx.toInt(), this)));
    }

    /**
     * Adds an element to the end of the Queue.
     * @param element - The element to add.
     * 
     * @returns Returns true if the element is added successfully; returns false otherwise.
     */
    public add(element: T): boolean {
        try {
            this.buffer.insertEnd(element);
        } catch (e) {
            return false;
        }
        return true;
    }

    /**
     * Returns an iterator over the elements in the Queue.
     * @returns An iterator over the elements in the Queue.
     */
    public override $_iterator(): IterableIterator<T> {
        return this.buffer.$_iterator();
    }
}
