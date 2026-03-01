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

import {BusinessError} from "@ohos.base";

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

export type LinkedListForEachCb<T> = (value: T, index: int, linkedList: LinkedList<T>) => void;

/**
 * Represents a node in the linked list.
 */
export final class ListNode<T> {
    element: T;
    prev?: ListNode<T>;
    next?: ListNode<T>;

    constructor(element: T) {
        this.element = element;
    }
}

/**
 * Iterator for the LinkedList class.
 */
final class LinkedListIterator_T<T> implements IterableIterator<T> {
    private node: ListNode<T> | undefined;

    constructor(node: ListNode<T> | undefined) {
        this.node = node;
    }

    override next(): IteratorResult<T> {
        if (this.node === undefined) {
            return new IteratorResult<T>();
        }
        let element = this.node!.element;
        this.node = this.node!.next;
        return new IteratorResult<T>(element);
    }

    override $_iterator(): IterableIterator<T> {
        return this;
    }
}

/**
 * A doubly linked list implementation.
 */
export class LinkedList<T> implements Iterable<T>, JsonReplacer {
    private elementNum: int;
    head?: ListNode<T>;
    private tail?: ListNode<T>;

    public jsonReplacer(): Record<String, Any> {
        let arrayObj: Record<String, Any> = {};
        this.forEach((value, index) => {
            if (Array.isArray(value)) {
                for (let i = 0; i < (value as Array<Any>).length; i++) {
                    if ((value as Array<Any>)[i] === undefined) {
                        (value as Array<Any>)[i] = null;
                    }
                }
            }
            arrayObj[String(index)] = value;
        });
        return arrayObj;
    }

    public constructor() {
        this.head = undefined;
        this.tail = undefined;
        this.elementNum = 0;
    }

    public toString(): string {
        return "[object LinkedList]";
    }

    /**
     * Gets the number of elements in the list.
     */
    public get length(): int {
        return this.elementNum;
    }

    /**
     * Adds an element to the end of the list.
     * @param element The element to add.
     * @returns True if the element was added successfully.
     */
    public add(element: T): boolean {
        this.insert(this.elementNum, element);
        return true;
    }

    /**
     * Adds an element to the beginning of the list.
     * @param element The element to add.
     */
    public addFirst(element: T): void {
        this.insert(0, element);
    }

    /**
     * Removes all elements from the list.
     */
    public clear(): void {
        this.head = undefined;
        this.tail = undefined;
        this.elementNum = 0;
    }

    /**
     * Creates a shallow copy of the list.
     * @returns A new LinkedList instance with the same elements.
     */
    public clone(): LinkedList<T> {
        let cloneList: LinkedList<T> = new LinkedList<T>();
        let currentNode: ListNode<T> | undefined = this.head;
        while (currentNode !== undefined) {
            cloneList.add(currentNode!.element);
            currentNode = currentNode!.next;
        }
        return cloneList;
    }

    /**
     * Converts the list to an array.
     * @returns An array containing all elements of the list.
     */
    public convertToArray(): Array<T> {
        let array = new Array<T>(this.elementNum);
        array.length = 0;

        let currentNode: ListNode<T> | undefined = this.head;
        while (currentNode !== undefined) {
            array.push(currentNode!.element);
            currentNode = currentNode!.next;
        }
        return array;
    }

    /**
    * Iterates over each element in the linked list, executing the provided callback function for each element.
    *
    * @param callbackfn - A function that accepts up to two arguments.
    * @param callbackfn.value - The current element being processed in the linked list.
    * @param callbackfn.index - The index of the current element being processed in the linked list.
    * @param callbackfn.linkedList - The linked list that forEach is being applied to.
    */
    public forEach(callbackfn: LinkedListForEachCb<T>): void {
        let index: int = 0;
        let currentNode: ListNode<T> | undefined = this.head;
        while (currentNode !== undefined) {
            callbackfn(currentNode!.element, index++, this);
            currentNode = currentNode?.next;
        }
    }

    /**
     * Gets the element at the specified index.
     * @param index The index of the element to get.
     * @returns The element at the specified index, or undefined if the index is out of range.
     */
    public get(index: int): T {
        this.checkIndexType(index);
        this.checkIndex(index, this.elementNum - 1);
        let currentNode: ListNode<T> | undefined = this.getNode(index);
        return currentNode!.element;
    }

    /**
     * Retrieves the first element in the linked list.
     *
     * @returns The first element of type `T`.
     */
    public getFirst(): T {
        this.checkEmptyContainer();
        return this.head!.element;
    }

    /**
     * Gets the index of the first occurrence of the specified element.
     * @param element The element to locate.
     * @returns The index of the first occurrence of the element, or -1 if not found.
     */
    public getIndexOf(element: T): int {
        let index: int = 0;
        let currentNode: ListNode<T> | undefined = this.head;
        while (currentNode !== undefined) {
            if (currentNode!.element == element) {
                return index;
            }
            currentNode = currentNode!.next;
            index++;
        }
        return -1;
    }

    /**
     * Retrieves the last element in the linked list.
     *
     * @returns The last element of type `T`.
     */
    public getLast(): T {
        this.checkEmptyContainer();
        return this.tail!.element;
    }

    /**
     * Gets the index of the last occurrence of the specified element.
     * @param element The element to locate.
     * @returns The index of the last occurrence of the element, or -1 if not found.
     */
    public getLastIndexOf(element: T): int {
        let index: int = this.elementNum - 1;
        let currentNode: ListNode<T> | undefined = this.tail;
        while (currentNode !== undefined) {
            if (currentNode!.element == element) {
                return index;
            }
            currentNode = currentNode!.prev;
            index--;
        }
        return -1;
    }

    /**
     * Determines whether the list contains the specified element.
     * @param element The element to locate.
     * @returns True if the element is found, otherwise false.
     */
    public has(element: T): boolean {
        let currentNode: ListNode<T> | undefined = this.head;
        while (currentNode !== undefined) {
            if (currentNode!.element == element) {
                return true;
            }
            currentNode = currentNode!.next;
        }
        return false;
    }

    /**
     * Inserts an element at the specified index.
     * @param index The index at which to insert the element.
     * @param element The element to insert.
     * @throws BusinessError if the index is out of range.
     */
    public insert(index: int, element: T): void {
        this.checkIndexType(index);
        this.checkIndex(index, this.length);

        let newNode: ListNode<T> = new ListNode<T>(element);
        if (this.elementNum == 0) {
            this.head = this.tail = newNode;

            this.elementNum++;
            return;
        }

        if (index == 0) {
            this.head!.prev = newNode;
            newNode.next = this.head;
            this.head = newNode;

            this.elementNum++;
            return;
        }

        if (index == this.elementNum) {
            this.tail!.next = newNode;
            newNode.prev = this.tail;
            this.tail = newNode;

            this.elementNum++;
            return;
        }
        let currentNode: ListNode<T> | undefined = this.getNode(index);
        newNode.next = currentNode;
        newNode.prev = currentNode!.prev;
        currentNode!.prev!.next = newNode;
        currentNode!.prev = newNode;
        this.elementNum++;
    }

    /**
     * Removes the first occurrence of the specified element.
     * @param element The element to remove.
     * @returns True if the element was removed, otherwise false.
     */
    public remove(element: T): boolean {
        let currentNode: ListNode<T> | undefined = this.head;
        while (currentNode !== undefined) {
            if (currentNode!.element == element) {
                if (currentNode === this.head) {
                    this.removeByIndex(0);
                    return true;
                }
                if (currentNode === this.tail) {
                    this.removeByIndex(this.elementNum - 1);
                    return true;
                }
                currentNode!.prev!.next = currentNode!.next;
                currentNode!.next!.prev = currentNode!.prev;
                this.elementNum--;
                return true;
            }
            currentNode = currentNode!.next;
        }
        return false;
    }

    /**
     * Removes the element at the specified index.
     * @param index The index of the element to remove.
     * @returns The removed element, or undefined if the index is out of range.
     * @throws BusinessError if the index is out of range.
     */
    public removeByIndex(index: int): T | undefined {
        this.checkIndexType(index);
        this.checkEmptyContainer();
        this.checkIndex(index, this.length - 1);

        let removedNode: ListNode<T> | undefined;
        if (index == 0) {
            removedNode = this.head!;
            this.head = this.head!.next;
            if (this.head !== undefined) {
                this.head!.prev = undefined;
            }
        } else if (index == this.elementNum - 1) {
            removedNode = this.tail!;
            this.tail = this.tail!.prev;
            this.tail!.next = undefined;
        } else {
            removedNode = this.getNode(index);
            removedNode!.prev!.next = removedNode!.next;
            removedNode!.next!.prev = removedNode!.prev;
        }
        this.elementNum--;
        return removedNode!.element;
    }

    /**
     * Removes the first element of the list.
     * @returns The removed element, or undefined if the list is empty.
     */
    public removeFirst(): T | undefined {
        this.checkEmptyContainer();
        return this.removeByIndex(0);
    }

    /**
     * Removes the first occurrence of the specified element.
     * @param element The element to remove.
     * @returns True if the element was removed, otherwise false.
     */
    public removeFirstFound(element: T): boolean {
        return this.remove(element);
    }

    /**
     * Removes the last element of the list.
     * @returns The removed element, or undefined if the list is empty.
     */
    public removeLast(): T | undefined {
        this.checkEmptyContainer();
        return this.removeByIndex(this.elementNum - 1);
    }

    /**
     * Removes the last occurrence of the specified element.
     * @param element The element to remove.
     * @returns True if the element was removed, otherwise false.
     */
    public removeLastFound(element: T): boolean {
        let currentNode: ListNode<T> | undefined = this.tail;
        while (currentNode !== undefined) {
            if (currentNode!.element == element) {
                if (currentNode === this.head) {
                    this.removeByIndex(0);
                    return true;
                }
                if (currentNode === this.tail) {
                    this.removeByIndex(this.elementNum - 1);
                    return true;
                }
                currentNode!.prev!.next = currentNode!.next;
                currentNode!.next!.prev = currentNode!.prev;
                this.elementNum--;
                return true;
            }
            currentNode = currentNode!.prev;
        }
        return false;
    }

    /**
     * Replaces the element at the specified index with a new element.
     * @param index The index of the element to replace.
     * @param element The new element.
     * @returns The replaced element, or undefined if the index is out of range.
     */
    public set(index: int, element: T): T {
        this.checkIndexType(index);
        this.checkEmptyContainer();
        this.checkIndex(index, this.length - 1);

        let currentNode: ListNode<T> | undefined = this.getNode(index);
        currentNode!.element = element;
        return element;
    }

    /**
     * Gets the node at the specified index.
     * @param index The index of the node to get.
     * @returns The node at the specified index, or undefined if the index is out of range.
     */
    private getNode(index: int): ListNode<T> | undefined {
        if (index >= 0 && index < this.elementNum) {
            let currentNode: ListNode<T>;
            if (index < this.elementNum / 2) {
                currentNode = this.head!;
                for (let i = 0; i < index; i++) {
                    currentNode = currentNode.next!;
                }
            } else {
                currentNode = this.tail!;
                for (let i = this.elementNum - 1; i > index; i--) {
                    currentNode = currentNode.prev!;
                }
            }
            return currentNode;
        }
        return undefined;
    }

    /**
     * Returns an iterator for the list.
     * @returns An iterator for the list.
     */
    public override $_iterator(): IterableIterator<T> {
        return new LinkedListIterator_T<T>(this.head);
    }

    private checkIndexType(index: int): void {
        if (index > Int.MAX_VALUE) {
            throw createBusinessError(TypeErrorCodeId, `The type of "index" must be small integer. Received value is: ${index}`);
        }
    }

    private checkIndex(index: int, length: int): void {
        if (index > length || index < 0) {
            throw createBusinessError(OutOfBoundsErrorCodeId, `The value of \"index\" is out of range. It must be >= 0 && <= ${length}. Received value is: ${index}`);
        }
    }

    private checkEmptyContainer(): void {
        if (this.elementNum == 0) {
            throw createBusinessError(EmptyContainerId, `Container is empty`);
        }
    }
}
