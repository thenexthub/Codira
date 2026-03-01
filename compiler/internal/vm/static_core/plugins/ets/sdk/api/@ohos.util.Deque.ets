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

import { BusinessError } from "@ohos.base";

const TypeErrorCodeId: int = 401;
const OutOfBoundsErrorCodeId: int = 10200001;
const EmptyContainerId: int = 10200010;

function createBusinessError(code: int, message: string) {
    let err = new BusinessError();
    err.code = code;
    err.name = 'BusinessError';
    err.message = message;
    return err;
}

export type DequeforEachCb<T> = (value: T, index: int, deque: Deque<T>) => void;

export default class Deque<T> implements Iterable<T>, JsonReplacer {
    private readonly DEFAULT_CAPACITY = 8;
    private capacity: int = this.DEFAULT_CAPACITY;
    private buffer: Array<T>;
    private front: int = 0;
    private rear: int = 0;

    public toString(): string {
        return "[object Deque]";
    }

    public jsonReplacer(): Record<String, Any> {
        const buf = this.buffer;
        const len = buf.length;
        let arrayObj: Record<String, Any> = {};
        for (let i = 0; i < len; i++) {
            arrayObj[String(i)] = buf[i];
        }
        return arrayObj;
    }

    /**
     * property: The number of elements in the ArrayList.
     */
    get length(): int {
        return (this.rear - this.front + this.capacity) % this.capacity;
    }

    /**
     * Returns the first element of the deque without removing it.
     *
     * @returns { T } The first element of the deque.
     */
    public getFirst(): T {
        this.checkEmptyContainer();
        return this.buffer[this.front];
    }

    /**
     * Returns the last element of the deque without removing it.
     *
     * @returns { T } The last element of the deque.
     */
    public getLast(): T {
        this.checkEmptyContainer();
        return this.buffer[(this.rear - 1 + this.capacity) % this.capacity];
    }

    /**
     * Removes and returns the first element from the deque.
     *
     * @returns { T } The first element of the deque.
     */
    public popFirst(): T {
        this.checkEmptyContainer();

        let firstElement: T = this.buffer[this.front];
        this.front = (this.front + 1) % this.capacity;

        return firstElement;
    }

    /**
     * Removes and returns the last element from the deque.
     *
     * @returns { T } The last element of the deque.
     */
    public popLast(): T {
        this.checkEmptyContainer();

        this.rear = (this.rear - 1 + this.capacity) % this.capacity;
        let element: T = this.buffer[this.rear];

        return element;
    }

    /**
     * Creates a new instance of Deque.
     */
    public constructor() {
        this.capacity = this.DEFAULT_CAPACITY;
        this.buffer = new Array<T>(this.capacity);
    }

    /**
     * Checks if the deque contains a specific element.
     *
     * @param element - The element to check for.
     * @returns True if the element is found, false otherwise.
     */
    public has(element: T): boolean {
        let i: int = this.front;
        while (i != this.rear) {
            if (this.buffer[i] === element) {
                return true;
            }
            i = (i + 1) % this.capacity;
        }

        return false;
    }

    /**
     * Executes a provided function once for each array element.
     *
     * @param callbackfn - The function to execute on each element, taking the value, index, and Deque as arguments.
     *
     */
    public forEach(callbackfn: DequeforEachCb<T>): void {
        let i: int = this.front, k = 0;
        while (i != this.rear) {
            callbackfn(this.buffer[i], k++, this);
            i = (i + 1) % this.capacity;
        }
    }

    /**
     * Inserts an element at the end of the deque.
     *
     * @param element - The element to insert.
     */
    public insertEnd(element: T): void {
        if (this.isFull()) {
            this.increaseCapacity();
        }

        this.buffer[this.rear] = element;
        this.rear = (this.rear + 1) % this.capacity;
    }

    /**
     * Inserts an element at the front of the deque.
     *
     * @param element - The element to insert.
     */
    public insertFront(element: T): void {
        if (this.isFull()) {
            this.increaseCapacity();
        }

        this.front = (this.front - 1 + this.capacity) % this.capacity;
        this.buffer[this.front] = element;
    }

    /**
     * Returns an iterator for the deque.
     *
     * @returns {IterableIterator<T>} An iterator for the deque.
     */
    public override $_iterator(): IterableIterator<T> {
        return new DequeValuesIterator_T<T>(this, this.front, this.rear, this.capacity, this.buffer);
    }

    /**
     * Checks if the deque is full.
     *
     * @returns {boolean} True if the deque is full, false otherwise.
     */
    private isFull(): boolean {
        return ((this.front - 1 + this.capacity) % this.capacity) == this.rear;
    }

    /**
     * Checks if the deque is empty.
     *
     * @returns {boolean} True if the deque is empty, false otherwise.
     */
    private isEmpty(): boolean {
        return this.front == this.rear;
    }

    /**
     * Increases the capacity of the deque.
     *
     * @throws {BusinessError} If the current capacity has reached the maximum limit.
     */
    private increaseCapacity(): void {
        let newCap: int = min(this.capacity * 2, Int.MAX_VALUE);

        let newBuffer = new Array<T>(newCap);

        let i: int = this.front, k = 0;
        while (i != this.rear) {
            newBuffer[k++] = this.buffer[i];
            i = (i + 1) % this.capacity;
        }

        this.rear = this.length.toInt();
        this.front = 0;
        this.capacity = newCap;
        this.buffer = newBuffer;
    }

    /**
     * Gets the element at the specified index.
     *
     * @param index - The index of the element to retrieve.
     *
     * @returns The element at the specified index.
     * @throws { BusinessError } 401 - The type of "index" must be small integer.
     * @throws { BusinessError } 10200001 - The value of "index" is out of range. It must be >= 0 && <= ${length - 1}.
     *  Received value is: ${index}
     */
    public $_get(index: int): T {
        this.checkEmptyContainer();
        if (index > Int.MAX_VALUE) {
            throw createBusinessError(TypeErrorCodeId, `The type of \"index\" must be small integer.`);
        }
        this.checkIndex(index);

        return this.buffer[this.getRealIndex(index)];
    }

    /**
     * Sets the element at the specified index.
     *
     * @param index - The index of the element to set.
     *
     * @param val - The value to set at the specified index.
     * @throws { BusinessError } 10200001 - The value of "index" is out of range. It must be >= 0 && <= ${length - 1}.
     *  Received value is: ${index}
     */
    public $_set(index: int, val: T): void {
        this.checkIndex(index);

        this.buffer[this.getRealIndex(index)] = val;
    }

    private getRealIndex(index: int): int {
        return (this.front + index) % this.capacity;
    }

    private checkEmptyContainer(): void {
        if (this.isEmpty()) {
            throw createBusinessError(EmptyContainerId, `Container is empty`);
        }
    }

    private checkIndex(index: int): void {
        if (index < 0) {
            throw createBusinessError(OutOfBoundsErrorCodeId, `The value of \"index\" is out of range. It must be >= 0 && <= ${this.length - 1}. Received value is: ${index}`);
        }
    }
}

class DequeValuesIterator_T<T> implements IterableIterator<T> {
    private parent: Deque<T>
    private idx: int = 0;
    private capacity: int
    private buffer: Array<T>;
    private rear: int = 0;

    constructor(parent: Deque<T>, front: int, rear: int, capacity: int, buffer: Array<T>) {
        this.parent = parent
        this.idx = front;
        this.rear = rear
        this.capacity = capacity
        this.buffer = buffer
    }

    override next(): IteratorResult<T> {
        if (this.parent.length == 0 || this.rear == this.idx) {
            return new IteratorResult<T>()
        }

        let val = new IteratorResult<T>(this.buffer[this.idx]);
        this.idx = (this.idx + 1) % this.capacity;

        return val;
    }

    override $_iterator(): IterableIterator<T> {
        return this;
    }
}
