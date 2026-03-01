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

const typeErrorCodeId: int = 401;

export type HashMapCbFn<K, V> = (value: V, key: K, map: HashMap<K, V>) => void;

interface ReadonlyHashMap<K, V> extends Iterable<[K, V]> {
    /**
     * Returns number of Entries with unique keys in the HashMap
     *
     * @returns number of Entries with unique keys in the HashMap
     */
    get length(): int;

    /**
     * Returns a value associated with key if present
     *
     * @param k the key to find in the HashMap/class
     *
     * @returns value if associated with key presents.
     */
    get(key: K): V | undefined;

    /**
     * Executes a provided function once per each key/value pair in the HashMap, in insertion order
     *
     * @param callbackFn to apply
     */
    forEach(callbackFn: (value: V, key: K, map: ReadonlyHashMap<K, V>) => void): void;

    /**
     * Returns elements from the HashMap as an keys Iterator
     *
     * @returns ValueIterator with hash map keys
     */
    keys(): IterableIterator<K>;

    /**
     * Returns elements from the HashMap as an values Iterator
     *
     * @returns ValueIterator with hash map values
     */
    values(): IterableIterator<V>;

    /**
     * Returns elements from the HashMap as an array of Entries
     *
     * @returns an array of Entries
     */
    entries(): IterableIterator<[K, V]>;
}

export default class HashMap<K, V> implements ReadonlyHashMap<K, V> {
    private buckets: Map<K, V>;

    constructor() {
        this.buckets = new Map<K, V>();
    }

    public toString(): string {
        return '[object HashMap]';
    }

    /**
     * Returns number of Entries with unique keys in the HashMap
     *
     * @returns number of Entries with unique keys in the HashMap
     */
    get length(): int {
        return this.buckets.size.toInt();
    }

    /**
     * Returns a value associated with key if present
     *
     * @param k the key to find in the HashMap/class
     *
     * @returns value if associated with key presents.
     */
    get(key: K): V | undefined {
        if (!this.buckets.has(key)) {
            return undefined;
        }
        return this.buckets.get(key);
    }

    /**
     * Executes a provided function once per each key/value pair in the HashMap, in insertion order
     *
     * @param callbackFn to apply
     */
    forEach(callbackFn: HashMapCbFn<K, V>): void {
        this.buckets.forEach((value: V, key: K) => {
            callbackFn(value, key, this);
        });
    }

    /**
     * Returns elements from the HashMap as an keys Iterator
     *
     * @returns ValueIterator with hash map keys
     */
    keys(): IterableIterator<K> {
        return this.buckets.keys();
    }

    /**
     * Returns elements from the HashMap as an values Iterator
     *
     * @returns ValueIterator with hash map values
     */
    values(): IterableIterator<V> {
        return this.buckets.values();
    }

    /**
     * Returns elements from the HashMap as an array of Entries
     *
     * @returns an array of Entries
     */
    entries(): IterableIterator<[K, V]> {
        return this.buckets.entries();
    }

    /**
     * Returns an iterator for the HashMap
     *
     * @returns An iterator for the HashMap
     */
    $_iterator(): IterableIterator<[K, V]> {
        return this.entries();
    }

    /**
     * Sets the value for the key in the HashMap
     *
     * @param key the key to set in the HashMap
     * @param value the value to set for the key
     *
     * @returns the HashMap object
     */
    set(key: K, value: V): Object {
        if (key === undefined) {
            throw new BusinessError(typeErrorCodeId, new Error('BusinessError',
                `The type of \"key\" must be Key of JS. Received value is: ${key}`, undefined))
        }
        this.buckets.set(key, value);
        return this;
    }

    /**
     * Checks if the key exists in the HashMap
     *
     * @param key the key to check in the HashMap
     *
     * @returns true if the key exists, false otherwise
     */
    hasKey(key: K): boolean {
        return this.buckets.has(key);
    }

    /**
     * Checks if the value exists in the HashMap
     *
     * @param value the value to check in the HashMap
     *
     * @returns true if the value exists, false otherwise
     */
    hasValue(value: V): boolean {
        const iter = this.buckets.values();
        let res = iter.next();
        while (!res.done) {
            if (res.value === value) {
                return true;
            }
            res = iter.next();
        }
        return false;
    }

    /**
     * Sets all entries from another HashMap into this HashMap
     *
     * @param map the HashMap to copy entries from
     */
    setAll(map: HashMap<K, V>): void {
        map.forEach((value: V, key: K) => {
            this.buckets.set(key, value);
        });
    }

    /**
     * Checks if the HashMap is empty
     *
     * @returns true if the HashMap is empty, false otherwise
     */
    isEmpty(): boolean {
        return this.buckets.size === 0;
    }

    /**
     * Replaces the value for the specified key in the HashMap
     *
     * @param key the key to replace the value for
     * @param newValue the new value to set for the key
     *
     * @returns true if the value was replaced, false otherwise
     */
    replace(key: K, newValue: V): boolean {
        if (!this.buckets.has(key)) {
            return false;
        }
        this.buckets.set(key, newValue);
        return true;
    }

    /**
     * Removes the entry for the specified key in the HashMap
     *
     * @param key the key to remove from the HashMap
     *
     * @returns the value associated with the key if it was removed, undefined otherwise
     */
    remove(key: K): V | undefined {
        if (!this.buckets.has(key) || this.isEmpty()) {
            return undefined;
        }
        const res = this.buckets.get(key);
        this.buckets.delete(key);
        return res;
    }

    /**
     * Clears all entries in the HashMap
     */
    clear(): void {
        this.buckets.clear();
    }
}
