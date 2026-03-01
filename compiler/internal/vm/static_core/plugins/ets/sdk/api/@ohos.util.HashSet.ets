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

import HashMap from '@ohos.util.HashMap';

export type HashSetCbFn<T> = (value: T, key: T, set: HashSet<T>) => void;

interface ReadonlyHashSet<T> extends Iterable<T> {
    /**
     * Returns number of unique elements in the HashSet
     *
     * @returns number of unique elements in the HashSet
     */
    get length(): int;

    /**
     * Checks if a value is in the HashSet
     *
     * @param value the value to find in the HashSet
     *
     * @returns true if the value is in the HashSet
     */
    has(value: T): boolean;

    /**
     * Executes a provided function once per each value in the HashSet, in insertion order
     *
     * @param callbackFn to apply; key is always same as value
     */
    forEach(callbackFn: (value: T, key: T, set: ReadonlyHashSet<T>) => void): void;

    /**
     * Returns elements from the HashSet as an values Iterator
     *
     * @returns ValueIterator with hash set values
     */
    values(): IterableIterator<T>;

    /**
     * Returns elements from the HashSet as an array of Entries
     *
     * @returns an array of Entries
     */
    entries(): IterableIterator<[T, T]>;
}

export class HashSet<T> implements ReadonlyHashSet<T> {
    private buckets: HashMap<T, T>;

    constructor() {
        this.buckets = new HashMap<T, T>();
    }

    public toString(): string {
        return '[object HashSet]';
    }

    /**
     * Returns number of unique elements in the HashSet
     *
     * @returns number of unique elements in the HashSet
     */
    get length(): int {
        return this.buckets.length.toInt();
    }

    /**
     * Checks if a value is in the HashSet
     *
     * @param value the value to find in the HashSet
     *
     * @returns true if the value is in the HashSet
     */
    has(value: T): boolean {
        return this.buckets.hasKey(value);
    }

    /**
     * Executes a provided function once per each value in the HashSet, in insertion order
     *
     * @param callbackFn to apply; key is always same as value
     */
    forEach(callbackFn: HashSetCbFn<T>): void {
        this.buckets.forEach((value: T, key: T) => {
            callbackFn(value, key, this);
        });
    }

    /**
     * Returns elements from the HashSet as an values Iterator
     *
     * @returns ValueIterator with hash set values
     */
    values(): IterableIterator<T> {
        return this.buckets.keys();
    }

    /**
     * Returns elements from the HashSet as an array of Entries
     *
     * @returns an array of Entries
     */
    entries(): IterableIterator<[T, T]> {
        return this.buckets.entries();
    }

    /**
     * Returns an iterator for the HashSet
     *
     * @returns An iterator for the HashSet
     */
    $_iterator(): IterableIterator<T> {
        return this.values();
    }

    /**
     * Adds a value to the HashSet
     *
     * @param value the value to add to the HashSet
     *
     * @returns true if the value was added, false otherwise
     */
    add(value: T): boolean {
        this.buckets.set(value, value);
        return true;
    }

    /**
     * Checks if the HashSet is empty
     *
     * @returns true if the HashSet is empty, false otherwise
     */
    isEmpty(): boolean {
        return this.buckets.length === 0;
    }

    /**
     * Removes a value from the HashSet
     *
     * @param value the value to remove from the HashSet
     *
     * @returns true if the value was removed, false otherwise
     */
    remove(value: T): boolean {
        if (this.isEmpty() || !this.buckets.hasKey(value)) {
            return false;
        }
        const res = this.buckets.remove(value);
        return res === undefined ? false : true;
    }

    /**
     * Clears all entries in the HashSet
     */
    clear(): void {
        this.buckets.clear();
    }
}
