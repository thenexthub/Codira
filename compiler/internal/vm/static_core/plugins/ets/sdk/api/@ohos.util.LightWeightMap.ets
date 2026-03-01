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

export type LightWeightMapCbFn<K, V> = (value: V, key: K, map: LightWeightMap<K, V>) => void;

interface ReadonlyLightWeightMap<K, V> extends Iterable<[K, V]> {
    /**
     * Returns number of Entries with unique keys in the LightWeightMap
     *
     * @returns number of Entries with unique keys in the LightWeightMap
     */
    get length(): int;

    /**
     * Returns a value associated with key if present
     *
     * @param k the key to find in the LightWeightMap/class
     *
     * @returns value if associated with key presents.
     */
    get(key: K): V | undefined;

    /**
     * Executes a provided function once per each key/value pair in the LightWeightMap, in insertion order
     *
     * @param callbackFn to apply
     */
    forEach(callbackFn: (value: V, key: K, map: ReadonlyLightWeightMap<K, V>) => void): void;

    /**
     * Returns elements from the LightWeightMap as an keys Iterator
     *
     * @returns ValueIterator with hash map keys
     */
    keys(): IterableIterator<K>;

    /**
     * Returns elements from the LightWeightMap as an values Iterator
     *
     * @returns ValueIterator with hash map values
     */
    values(): IterableIterator<V>;

    /**
     * Returns elements from the LightWeightMap as an array of Entries
     *
     * @returns an array of Entries
     */
    entries(): IterableIterator<[K, V]>;
}

class LightWeightMapEntry<K, V> {
    entryHashs: Array<int>;
    entryKeys: Array<K>;
    entryValues: Array<V>;
    actualLength: int = 0;

    private static readonly MIN_BUCKETS_COUNT = 8;
    private capacity: int = LightWeightMapEntry.MIN_BUCKETS_COUNT;

    constructor() {
        this.entryHashs = new Array<int>(this.capacity);
        this.entryKeys = new Array<K>(this.capacity);
        this.entryValues = new Array<V>(this.capacity);
    }

    set(index: int, hash: int, key: K, value: V): void {
        this.insert<int>(this.entryHashs, index, hash);
        this.insert<K>(this.entryKeys, index, key);
        this.insert<V>(this.entryValues, index, value);
        this.actualLength++;
    }

    increaseCapacityTo(minimumCapacity: int): void {
        if (minimumCapacity < LightWeightMapEntry.MIN_BUCKETS_COUNT || this.capacity >= minimumCapacity) {
            return;
        }
        this.entryHashs = this.increaseCapacity<int>(this.entryHashs, minimumCapacity);
        this.entryKeys = this.increaseCapacity<K>(this.entryKeys, minimumCapacity);
        this.entryValues = this.increaseCapacity<V>(this.entryValues, minimumCapacity);
        this.capacity = minimumCapacity;
    }

    ensureUnusedCapacity(): void {
        if (this.capacity < (this.actualLength + 1)) {
            this.increaseCapacityTo(2 * this.capacity);
        }
    }

    remove(index: int): void {
        this.entryHashs.splice(index, 1);
        this.entryKeys.splice(index, 1);
        this.entryValues.splice(index, 1);
        this.actualLength--;
    }

    clear(): void {
        this.actualLength = 0;
        this.capacity = LightWeightMapEntry.MIN_BUCKETS_COUNT;
        this.entryHashs = new Array<int>(this.capacity);
        this.entryKeys = new Array<K>(this.capacity);
        this.entryValues = new Array<V>(this.capacity);
    }

    private insert<T>(a: Array<T>, index: int, value: T): void {
        for (let i: int = this.actualLength; i > index; i--) {
            a[i] = a[i - 1];
        }
        a[index] = value;
    }

    private increaseCapacity<T>(array: Array<T>, capacity: int): Array<T> {
        const newArray = new Array<T>(capacity);
        for (let i = 0; i < this.actualLength; i++) {
            newArray[i] = array[i]
        }
        return newArray;
    }

    private remove<T>(array: Array<T>, index: int): Array<T> {
        const len = this.actualLength;
        for (let i = index; i < len; i++) {
            if ((i + 1) === len) {
                break;
            }
            array[i] = array[i + 1]
        }
        return array;
    }
}

class LightWeightMapIterator<K, V, R> implements IterableIterator<R> {
    private parent: LightWeightMap<K, V>
    private idx: int = 0;
    private mapper: (index: int) => R;

    constructor(parent: LightWeightMap<K, V>, mapper: (index: int) => R) {
        this.parent = parent;
        this.mapper = mapper;
    }

    override next(): IteratorResult<R> {
        if (this.idx >= this.parent.length) {
            return new IteratorResult<R>();
        }
        const iter = new IteratorResult<R>(this.mapper(this.idx));
        this.idx++;
        return iter;
    }

    override $_iterator(): IterableIterator<R> {
        return this;
    }
}

export default class LightWeightMap<K, V> implements ReadonlyLightWeightMap<K, V> {
    private buckets: LightWeightMapEntry<K, V>;

    private static readonly SHIFT_COUNT = 31;

    constructor() {
        this.buckets = new LightWeightMapEntry<K, V>();
    }

    /**
     * Returns number of Entries with unique keys in the LightWeightMap
     *
     * @returns number of Entries with unique keys in the LightWeightMap
     */
    get length(): int {
        return this.buckets.actualLength;
    }

    /**
     * Returns a value associated with key if present
     *
     * @param k the key to find in the LightWeightMap/class
     *
     * @returns value if associated with key presents.
     */
    get(key: K): V | undefined {
        const index = this.getIndexOfKey(key);
        if (index < 0) {
            return undefined;
        }
        return this.buckets.entryValues[index];
    }

    /**
     * Executes a provided function once per each key/value pair in the LightWeightMap, in insertion order
     *
     * @param callbackFn to apply
     */
    forEach(callbackFn: LightWeightMapCbFn<K, V>): void {
        const iter = this.entries();
        let res = iter.next();
        while (!res.done) {
            callbackFn(res.value![1], res.value![0], this);
            res = iter.next();
        }
    }

    /**
     * Returns elements from the LightWeightMap as an keys Iterator
     *
     * @returns ValueIterator with hash map keys
     */
    keys(): IterableIterator<K> {
        return new LightWeightMapIterator<K, V, K>(this, (index: int) => this.buckets.entryKeys[index]);
    }

    /**
     * Returns elements from the LightWeightMap as an values Iterator
     *
     * @returns ValueIterator with hash map values
     */
    values(): IterableIterator<V> {
        return new LightWeightMapIterator<K, V, V>(this, (index: int) => this.buckets.entryValues[index]);
    }

    /**
     * Returns elements from the LightWeightMap as an array of Entries
     *
     * @returns an array of Entries
     */
    entries(): IterableIterator<[K, V]> {
        return new LightWeightMapIterator<K, V, [K, V]>(this,
            (index: int) => [this.buckets.entryKeys[index], this.buckets.entryValues[index]]);
    }

    /**
     * Returns an iterator for the LightWeightMap
     *
     * @returns An iterator for the LightWeightMap
     */
    $_iterator(): IterableIterator<[K, V]> {
        return this.entries();
    }

    /**
     * Gets the index of the specified key in the LightWeightMap
     *
     * @param key the key to find the index of
     *
     * @returns the index of the key, or -1 if the key is not found
     */
    getIndexOfKey(key: K): int {
        const index = this.checkIndexOfKey(key);
        return index < 0 ? -1 : index;
    }

    /**
     * Gets the index of the specified value in the LightWeightMap
     *
     * @param value the value to find the index of
     *
     * @returns the index of the value, or -1 if the value is not found
     */
    getIndexOfValue(value: V): int {
        return this.buckets.entryValues.indexOf(value).toInt();
    }

    /**
     * Gets the key at the specified index in the LightWeightMap
     *
     * @param index the index to get the key from
     *
     * @returns the key at the specified index
     * 
     * @throws BusinessError if the index is out of range, the container is empty or the index exceeds Int.MAX_VALUE.
     */
    getKeyAt(index: int): K {
        this.checkIndex(index);
        this.checkEmptyContainer();
        this.checkRange(index, this.buckets.actualLength);

        return this.buckets.entryKeys[index];
    }

    /**
     * Gets the value at the specified index in the LightWeightMap
     *
     * @param index the index to get the value from
     *
     * @returns the value at the specified index
     * 
     * @throws BusinessError if the index is out of range, the container is empty or the index exceeds Int.MAX_VALUE.
     */
    getValueAt(index: int): V {
        this.checkEmptyContainer();
        this.checkRange(index, this.buckets.actualLength);

        return this.buckets.entryValues[index];
    }

    /**
     * Checks if all entries from another LightWeightMap are present in this LightWeightMap
     *
     * @param map the LightWeightMap to check entries from
     *
     * @returns true if all entries are present, false otherwise
     */
    hasAll(map: LightWeightMap<K, V>): boolean {
        if (map.length > this.buckets.actualLength) {
            return false;
        }
        for (let index: int = 0; index < map.length; index++) {
            const key = map.getKeyAt(index)!;
            if (!this.hasKey(key)) {
                return false;
            }
            const value = map.getValueAt(index)!;
            if (!this.hasValue(value)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Checks if the LightWeightMap contains the specified key
     *
     * @param key the key to check in the LightWeightMap
     *
     * @returns true if the key is present, false otherwise
     */
    hasKey(key: K): boolean {
        const index = this.getIndexOfKey(key);
        return index < 0 ? false : true;
    }

    /**
     * Checks if the LightWeightMap contains the specified value
     *
     * @param value the value to check in the LightWeightMap
     *
     * @returns true if the value is present, false otherwise
     */
    hasValue(value: V): boolean {
        const index = this.buckets.entryValues.indexOf(value);
        return index < 0 ? false : true;
    }

    /**
     * Increases the capacity of the LightWeightMap to the specified minimum capacity
     *
     * @param minimumCapacity the minimum capacity to increase to
     */
    increaseCapacityTo(minimumCapacity: int): void {
        if (minimumCapacity > Int.MAX_VALUE) {
            return;
        }
        this.buckets.increaseCapacityTo(minimumCapacity);
    }

    /**
     * Checks if the LightWeightMap is empty
     *
     * @returns true if the LightWeightMap is empty, false otherwise
     */
    isEmpty(): boolean {
        return this.buckets.actualLength === 0;
    }

    /**
     * Removes the entry with the specified key from the LightWeightMap
     *
     * @param key the key to remove from the LightWeightMap
     *
     * @returns the value associated with the key if it was removed, undefined otherwise
     */
    remove(key: K): V | undefined {
        const index = this.getIndexOfKey(key);
        if (index < 0) {
            return undefined;
        }
        const value = this.buckets.entryValues[index];
        this.buckets.remove(index);
        return value;
    }

    /**
     * Removes the entry at the specified index from the LightWeightMap
     *
     * @param index the index to remove the entry from
     *
     * @returns true if the entry was removed, false otherwise
     * 
     * @throws BusinessError if the index exceeds Int.MAX_VALUE.
     */
    removeAt(index: int): boolean {
        if (index < 0 || this.buckets.actualLength <= index) {
            return false;
        }
        this.buckets.remove(index);
        return true;
    }

    /**
     * Sets the value for the specified key in the LightWeightMap
     *
     * @param key the key to set in the LightWeightMap
     * @param value the value to set for the key
     *
     * @returns the LightWeightMap object
     */
    set(key: K, value: V): Object {
        let index = this.checkIndexOfKey(key);
        if (index >= 0) {
            this.buckets.entryValues[index] = value;
        } else {
            this.buckets.ensureUnusedCapacity();
            index = ~index;
            const hashCode = this.hash(key);
            this.buckets.set(index, hashCode, key, value);
        }
        return this;
    }

    /**
     * Sets all entries from another LightWeightMap into this LightWeightMap
     *
     * @param map the LightWeightMap to copy entries from
     */
    setAll(map: LightWeightMap<K, V>): void {
        map.forEach((value: V, key: K) => {
            this.set(key, value);
        });
    }

    /**
     * Sets the value at the specified index in the LightWeightMap
     *
     * @param index the index to set the value at
     * @param newValue the new value to set at the specified index
     *
     * @returns true if the value was set
     * 
     * @throws BusinessError if the index is out of range, the container is empty or the index exceeds Int.MAX_VALUE.
     */
    setValueAt(index: int, newValue: V): boolean {
        this.checkEmptyContainer();
        this.checkRange(index, this.buckets.actualLength.toInt());

        this.buckets.entryValues[index] = newValue;
        return true;
    }

    /**
     * Returns a string representation of the LightWeightMap
     *
     * @returns a string representation of the LightWeightMap
     */
    toString(): String {
        const strArray: Array<String> = new Array<String>();
        this.forEach((value: V, key: K) => {
            strArray.push(`${key}:${value}`);
        });
        return strArray.join(',');
    }

    /**
     * Clears all entries in the LightWeightMap
     */
    clear(): void {
        this.buckets.clear();
    }

    private checkIndexOfKey(key: K): int {
        const hashCode = this.hash(key);
        const index = this.binarySearchHashes(this.buckets.entryHashs, hashCode, this.buckets.actualLength);
        if (index >= 0) {
            let right = index;
            while ((right < this.buckets.actualLength) && (this.buckets.entryHashs[right] === hashCode)) {
                if (this.buckets.entryKeys[right] === key) {
                    return right;
                }
                right++;
            }
            let left = index - 1;
            while ((left > 0) && (this.buckets.entryHashs[right] === hashCode)) {
                if (this.buckets.entryKeys[left] === key) {
                    return left;
                }
                left--;
            }
            return -1;
        }
        return index;
    }

    private hash(key: K): int {
        if (key === null) {
            return 0;
        }
        if (key === undefined) {
            return 1;
        }

        const keyHash: int = Runtime.getHashCodeByValue(key! as object).toInt(); // #26217
        const positiveMask: int = 1 << LightWeightMap.SHIFT_COUNT;
        if (keyHash >= 0) {
            return (keyHash | positiveMask)
        } else {
            return (keyHash & (~positiveMask))
        }
    }

    private binarySearchHashes(hashArray: Array<int>, hashCode: int, size: int): int {
        let low: int = 0, high: int = size - 1;
        while (low <= high) {
            const mid = (low + high) >>> 1;
            const midHash = hashArray[mid];
            if (midHash < hashCode) {
                low = mid + 1;
            } else {
                if (midHash === hashCode) {
                    return mid;
                }
                high = mid - 1;
            }
        }
        return -(low + 1);
    }

    private checkIndex(index: int): void {
        if (index > Int.MAX_VALUE) {
            throw createBusinessError(TypeErrorCodeId,
                `The type of "index" must be small integer. Received value is: ${index}`)
        }
    }

    private checkRange(index: int, length: int): void {
        if (index < 0 || index >= this.buckets.actualLength) {
            throw createBusinessError(OutOfBoundsErrorCodeId, `The value of \"index\" is out of range. It must be >= 0 && <= ${this.buckets.actualLength - 1}. Received value is: ${index}`);
        }
    }

    private checkEmptyContainer(): void {
        if(this.isEmpty()) {
            throw createBusinessError(EmptyContainerId, `Container is empty`);
        }
    }
}
