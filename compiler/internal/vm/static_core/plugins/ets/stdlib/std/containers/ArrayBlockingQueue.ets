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
    * ArrayBlockingQueue implementation
    */
    export class ArrayBlockingQueue<T> implements BlockingQueue<T> {
        /**
        * Constructs a ArrayBlockingQueue with provided capaity.
        *
        * @param { int } capacity the capacity of the ArrayBlockingQueue
        */
        constructor(capacity: int) {
            if (capacity <= 0) {
                throw new RangeError("Invalid capacity for ArrayBlockingQueue's constructor.");
            }
            this.capacitySize = capacity;
            this.array = new Array<T>(this.capacitySize);
            this.mutex = ConcurrencyHelpers.mutexCreate();
            this.condVar = ConcurrencyHelpers.condVarCreate();
        }

        /**
        * Constructs a ArrayBlockingQueue with provided capaity from iterable.
        *
        * @param { int } capacity the capacity of the ArrayBlockingQueue
        * @param { Iterable<T> } iterable the iterable instance to initialize ArrayBlockingQueue
        */
        constructor(capacity: int, iterable: Iterable<T>) {
            if (capacity <= 0) {
                throw new RangeError("Invalid capacity for ArrayBlockingQueue's constructor.");
            }
            this.capacitySize = capacity;
            this.array = new Array<T>(this.capacitySize);
            this.mutex = ConcurrencyHelpers.mutexCreate();
            this.condVar = ConcurrencyHelpers.condVarCreate();
            if (iterable) {
                const entriesIter = iterable.$_iterator();
                for (let iterRes = entriesIter.next(); !iterRes.done; iterRes = entriesIter.next()) {
                    const val = iterRes.value;
                    if (val !== undefined) {
                        this.add(val as T);
                    }
                }
            }
        }

        /**
        * Inserts element at the end. If the queue is full, blocking.
        *
        * @param { T } el the element to insert
        */
        override push(el: T): void {
            ConcurrencyHelpers.mutexLock(this.mutex);
            this.waitUntilQueueIsNotFull();
            this.array[this.curTailIdx] = el;
            this.increaseTailIdx();
            ConcurrencyHelpers.condVarNotifyOne(this.condVar, this.mutex);
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }

        /**
        * Removes and returns the first element in the BlockingQueue. If the queue is empty, blocking.
        *
        * @returns { T } the fisrt element poped in the queue.
        */
        override pop(): T {
            ConcurrencyHelpers.mutexLock(this.mutex);
            this.waitUntilQueueIsNotEmpty();
            try {
                const val = this.array[this.curHeadIdx];
                this.increaseHeadIdx();
                return val;
            } finally {
                ConcurrencyHelpers.condVarNotifyOne(this.condVar, this.mutex);
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * The insert operation. Returns true upon success and false if no space is currently available, no blocking.
        *
        * @param { T } element the element inserted to the BlockingQueue
        *
        * @returns { boolean } True upon success and false if no space is currently available.
        */
        override add(element: T): boolean {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                if (this.isQueueFull) {
                    return false;
                }
                this.array[this.curTailIdx] = element;
                this.increaseTailIdx();
                return true;
            } finally {
                ConcurrencyHelpers.condVarNotifyOne(this.condVar, this.mutex);
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * The poll operation. Pop one element and return it if the queue is not empty.
        * Returns undefined when the queue is empty. No blocking.
        *
        * @returns { T | undefined } the deleted element or when the queue is empty.
        */
        override poll(): T | undefined {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                if (this.isQueueEmpty) {
                    return undefined;
                }
                const val = this.array[this.curHeadIdx];
                this.increaseHeadIdx();
                return val;
            } finally {
                ConcurrencyHelpers.condVarNotifyOne(this.condVar, this.mutex);
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * Returns the first element in a BlockingQueue.
        *
        * @returns { T | undefined } the first element or undefined when the queue is empty.
        */
        override getFirst(): T | undefined {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                if (this.isQueueEmpty) {
                    return undefined;
                }
                const value = this.array[this.curHeadIdx];
                return value;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * Returns the end element in a ArrayBlockingQueue.
        *
        * @returns { T | undefined } the end element or undefined when the queue is empty.
        */
        public getEnd(): T | undefined {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                if (this.isQueueEmpty) {
                    return undefined;
                }
                let tailIdx = this.curTailIdx;
                if (this.curTailIdx == 0) {
                    tailIdx = this.capacitySize;
                }
                const value = this.array[tailIdx - 1];
                return value;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * Returns size of a ArrayBlockingQueue.
        *
        * @returns { int } size
        */
        override get size(): int {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                let size: int = 0;
                if (this.curTailIdx >= this.curHeadIdx && !this.isQueueFull) {
                    size = this.curTailIdx - this.curHeadIdx;
                } else {
                    size = this.curTailIdx - this.curHeadIdx + this.capacitySize;
                }
                return size;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * Checks if the BlockingQueue is empty.
        *
        * @returns { boolean } True if a BlockingQueue has no elements, otherwise false.
        */
        override isEmpty(): boolean {
            return this.size == 0;
        }

        /**
        * Returns the capacity of a BlockingQueue.
        *
        * @returns { int } the capacity of the BlockingQueue.
        */
        override get capacity(): int {
            return this.capacitySize;
        }

        /**
        * Returns the number of additional elements that this queue can ideally accept without blocking.
        *
        * @returns { int } the capacity of the BlockingQueue.
        */
        override remainingCapacity(): int {
            return this.capacitySize - this.size;
        }


        private waitUntilQueueIsNotEmpty() {
            while (this.isQueueEmpty) {
                // Currently, the queue is empty. Need to await.
                // This method is used for waiting until the ArrayBlockingQueue
                // has at least one element.
                ConcurrencyHelpers.condVarWait(this.condVar, this.mutex);
            }
        }

        private waitUntilQueueIsNotFull() {
            while (this.isQueueFull) {
                // Currently, the queue is full. Need to await.
                // This method is used for waiting until the ArrayBlockingQueue
                // is not full.
                ConcurrencyHelpers.condVarWait(this.condVar, this.mutex);
            }
        }

        private increaseTailIdx(): void {
            this.curTailIdx++;
            this.curTailIdx = this.curTailIdx % this.capacitySize;
            if (this.isQueueEmpty) {
                this.isQueueEmpty = false;
            }
            if (this.curTailIdx == this.curHeadIdx) {
                this.isQueueFull = true;
            }
        }

        private increaseHeadIdx(): void {
            this.curHeadIdx++;
            this.curHeadIdx = this.curHeadIdx % this.capacitySize;
            if (this.isQueueFull) {
                this.isQueueFull = false;
            }
            if (this.curHeadIdx == this.curTailIdx) {
                this.isQueueEmpty = true;
            }
        }

        private array: Array<T>;                        // Array, where all ArrayBlockingQueue elements will be located
        private curHeadIdx: int = 0;                    // Index of the first element in a ArrayBlockingQueue
        private curTailIdx: int = 0;                    // Index of the last element in a ArrayBlockingQueue
        private mutex: Object;                          // push/pop Mutex
        private condVar: Object;                        // push/pop CondVar
        private capacitySize: int = 0;
        private isQueueFull: boolean = false;
        private isQueueEmpty: boolean = true;
    }
}
