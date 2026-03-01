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

const OutOfBoundsErrorCodeId: int = 10200001;
const EmptyContainerId: int = 10200010;

function createBusinessError(code: int, message: string) {
    let err = new BusinessError();
    err.code = code;
    err.name = 'BusinessError';
    err.message = message;
    return err;
}

export type TreeMapForEachCb<K, V> = (value: V, key: K, map: TreeMap<K, V>) => void;
export type TreeMapComparator<K> = (firstValue: K, secondValue: K) => double;

export interface ReadonlyTreeMap<K, V> extends Iterable<[K, V]> {
    /**
     * Return the number of elements in the TreeMap
     *
     * @returns the number of elements in the TreeMap
     */
    get length(): int;

    /**
     * Returns a value associated with key if present
     *
     * @param k: the key to find in the TreeMap
     *
     * @returns value if associated with key presents
     */
    get(key: K): V | undefined;

    /**
     * Executes a provided function once per each key/value pair in the TreeMap
     *
     * @param callbackfn: the function to apply
     */
    forEach(callbackfn: (value: V, key: K, map: ReadonlyTreeMap<K, V>) => void): void;

    /**
     * Return the elements in the TreeMap as an Iterator of keys
     *
     * @returns an Iterator of keys
     */
    keys(): IterableIterator<K>;

    /**
     * Return the elements in the TreeMap as an Iterator of values
     *
     * @returns an Iterator of values
     */
    values(): IterableIterator<V>;

    /**
     * Return the elements in the TreeMap as an Iterator of [key, value]s
     *
     * @returns an Iterator of [key, value]s
     */
    entries(): IterableIterator<[K, V]>;
}

final class TreeMapEntry<K, V> {
    left: TreeMapEntry<K, V> | undefined = undefined;
    right: TreeMapEntry<K, V> | undefined = undefined;
    parent: TreeMapEntry<K, V> | undefined = undefined;
    isBlack: boolean = false;
    key: K;
    val: V;

    constructor(key: K, val: V) {
        this.key = key;
        this.val = val;
    }

    toString(): string {
        return `{${this.key},${this.val}}`;
    }
}

final class TreeMapIterator<K, V, R> implements IterableIterator<R> {
    private node: TreeMapEntry<K, V> | undefined;
    private mapper: (e: TreeMapEntry<K, V>) => R;

    constructor(node: TreeMapEntry<K, V>, mapper: (e: TreeMapEntry<K, V>) => R) {
        this.node = node;
        this.mapper = mapper;
    }

    override next(): IteratorResult<R> {
        if (this.node !== undefined) {
            const entry = this.node;
            if (this.node!.right !== undefined) {
                this.node = this.node!.right;
                while (this.node!.left !== undefined) {
                    this.node = this.node!.left;
                }
            } else {
                while ((this.node!.parent !== undefined) && (this.node === this.node!.parent!.right)) {
                    this.node = this.node!.parent;
                }
                this.node = this.node!.parent;
            }
            return new IteratorResult<R>(this.mapper(entry!));
        } else {
            return new IteratorResult<R>();
        }
    }

    override $_iterator(): IterableIterator<R> {
        return this;
    }
}

final class EmptyTreeMapIterator<R> implements IterableIterator<R> {
    override next(): IteratorResult<R> {
        return new IteratorResult<R>();
    }

    override $_iterator(): IterableIterator<R> {
        return this;
    }
}

type comparator<K> = (firstValue: K, secondValue: K) => double;

export default class TreeMap<K, V> implements ReadonlyTreeMap<K, V> {
    private rootEntry: TreeMapEntry<K, V> | undefined = undefined;
    private comparator: comparator<K>;
    private elementNum: int = 0;

    private mappedIterator<R>(fn: (e: TreeMapEntry<K, V>) => R): IterableIterator<R> {
        let entry = this.rootEntry;
        if (entry !== undefined) {
            while (entry!.left !== undefined) {
                entry = entry!.left;
            }
            return new TreeMapIterator<K, V, R>(entry!, fn);
        } else {
            return new EmptyTreeMapIterator<R>();
        }
    }

    private getEntry(key: K): TreeMapEntry<K, V> | undefined {
        let entry = this.rootEntry;
        while (entry !== undefined) {
            const res = this.comparator!(entry!.key, key);
            if (res === 0) {
                break;
            } else if (res > 0) {
                entry = entry!.right;
            } else {
                entry = entry!.left;
            }
        }

        return entry;
    }

    private rotateLeft(parent: TreeMapEntry<K, V>): void {
        const entry = parent.right!;
        const subEntry = entry.left;

        parent.right = subEntry;
        if (subEntry !== undefined) {
            subEntry.parent = parent;
        }
        entry.left = parent;
        entry.parent = parent.parent;
        if (parent.parent !== undefined) {
            if (parent === parent.parent!.left) {
                parent.parent!.left = entry;
            } else {
                parent.parent!.right = entry;
            }
        }
        parent.parent = entry;

        if (parent === this.rootEntry) {
            this.rootEntry = entry;
        }
    }

    private rotateRight(parent: TreeMapEntry<K, V>): void {
        const entry = parent.left!;
        const subEntry = entry.right;

        parent.left = subEntry;
        if (subEntry !== undefined) {
            subEntry.parent = parent;
        }
        entry.right = parent;
        entry.parent = parent.parent;
        if (parent.parent !== undefined) {
            if (parent === parent.parent!.left) {
                parent.parent!.left = entry;
            } else {
                parent.parent!.right = entry;
            }
        }
        parent.parent = entry;

        if (parent === this.rootEntry) {
            this.rootEntry = entry;
        }
    }

    private adjustUpward(current: TreeMapEntry<K, V>): void {
        while (current !== this.rootEntry) {
            const parent = current.parent!;
            const isLeftChild = current === parent.left;
            const brother = isLeftChild ? parent.right! : parent.left!;
            if (!brother.isBlack) {
                brother.isBlack = true;
                parent.isBlack = false;
                if (isLeftChild) {
                    this.rotateLeft(parent);
                } else {
                    this.rotateRight(parent);
                }
            } else {
                if ((brother.left === undefined || brother.left!.isBlack) &&
                    (brother.right === undefined || brother.right!.isBlack)) {
                    brother.isBlack = false;
                    current = parent;
                } else {
                    if (isLeftChild) {
                        if (brother.right === undefined || brother.right!.isBlack) {
                            brother.left!.isBlack = true;
                            brother.isBlack = false;
                            this.rotateRight(brother);
                        }
                        brother.isBlack = parent.isBlack;
                        parent.isBlack = true;
                        brother.right!.isBlack = true;
                        this.rotateLeft(parent);
                    } else {
                        if (brother.left === undefined || brother.left!.isBlack) {
                            brother.right!.isBlack = true;
                            brother.isBlack = false;
                            this.rotateLeft(brother);
                        }
                        brother.isBlack = parent.isBlack;
                        parent.isBlack = true;
                        brother.left!.isBlack = true;
                        this.rotateRight(parent);
                    }
                    break;
                }
            }
        }
    }

    private iteratorForEach<V>(x: Iterator<V>, fn: (x: V) => void): void {
        while (true) {
            const v = x.next();
            if (v.done) {
                return;
            }
            fn(v.value as V);
        }
    }

    private static defaultComparator<K>(firstValue: K, secondValue: K): int {
        if (firstValue instanceof Number && secondValue instanceof Number) {
            const firstValueN = (firstValue as Number).valueOf();
            const secondValueN = (secondValue as Number).valueOf();
            if (Number.isInteger(firstValueN) && Number.isInteger(secondValueN) &&
                firstValueN <= Int.MAX_VALUE / 128 && firstValueN >= Int.MIN_VALUE / 128 &&
                secondValueN <= Int.MAX_VALUE / 128 && secondValueN >= Int.MIN_VALUE / 128) {
                const firstValueI = firstValueN.toInt();
                const secondValueI = secondValueN.toInt();
                return secondValueI - firstValueI;
            }
        } else if (firstValue instanceof String && secondValue instanceof String) {
            return secondValue.compareTo(firstValue);
        }
        const firstValueS = new String(firstValue);
        const secondValueS = new String(secondValue);
        return secondValueS.compareTo(firstValueS);
    }

    /**
     * Construct a TreeMap
     *
     * @param comparator: the comparator of the TreeMap
     */
    constructor(comparator?: TreeMapComparator<K>) {
        if (comparator === undefined) {
            const defaultComparator: (firstValue: K, secondValue: K) => double =
                (firstValue: K, secondValue: K): double => {
                return TreeMap.defaultComparator(firstValue, secondValue);
            }
            this.comparator = defaultComparator;
        } else {
            this.comparator = comparator;
        }
    }

    /**
     * Return the number of elements in the TreeMap
     *
     * @returns the number of elements in the TreeMap
     */
    override get length(): int {
        return this.elementNum;
    }

    /**
     * Returns a value associated with key if present
     *
     * @param k: the key to find in the TreeMap
     *
     * @returns value if associated with key presents
     */
    get(key: K): V | undefined {
        return this.getEntry(key)?.val;
    }

    /**
     * Executes a provided function once per each key/value pair in the TreeMap
     *
     * @param callbackfn: the function to apply
     */
    override forEach(callbackfn: TreeMapForEachCb<K, V>): void {
        const entriesIter =
            this.mappedIterator<TreeMapEntry<K, V>>((e: TreeMapEntry<K, V>): TreeMapEntry<K, V> => e);
        this.iteratorForEach<TreeMapEntry<K, V>>(entriesIter, (e: TreeMapEntry<K, V>): void => {
            callbackfn(e.val, e.key, this);
        })
    }

    /**
     * Return the elements in the TreeMap as an Iterator of keys
     *
     * @returns an Iterator of keys
     */
    override keys(): IterableIterator<K> {
        return this.mappedIterator<K>((e: TreeMapEntry<K, V>): K => e.key);
    }

    /**
     * Return the elements in the TreeMap as an Iterator of values
     *
     * @returns an Iterator of values
     */
    override values(): IterableIterator<V> {
        return this.mappedIterator<V>((e: TreeMapEntry<K, V>): V => e.val);
    }

    /**
     * Return the elements in the TreeMap as an Iterator of [key, value]s
     *
     * @returns an Iterator of [key, value]s
     */
    override entries(): IterableIterator<[K, V]> {
        return this.mappedIterator<[K, V]>((e: TreeMapEntry<K, V>): [K, V] => [e.key, e.val]);
    }

    /**
     * Delete all elements of the TreeMap
     */
    clear(): void {
        this.elementNum = 0;
        this.rootEntry = undefined;
    }

    /**
     * Return the key of the first element in the TreeMap
     *
     * @returns the key of the first element
     */
    getFirstKey(): K {
        this.checkEmptyContainer();
        let entry = this.rootEntry!;
        while (entry.left !== undefined) {
            entry = entry.left!;
        }
        return entry.key;
    }

    /**
     * Return the higher key of the given key's element in the TreeMap
     *
     * @param key: the key of the element which will be get the higher key
     *
     * @returns the higher key of the given key's element
     */
    getHigherKey(key: K): K | undefined {
        this.checkEmptyContainer();
        let resultEntry: TreeMapEntry<K, V> | undefined = undefined;
        let entry = this.rootEntry;

        while(entry !== undefined) {
            if (this.comparator!(entry!.key, key) >= 0) {
                entry = entry!.right;
            } else {
                resultEntry = entry;
                entry = entry!.left;
            }
        }

        if (resultEntry !== undefined) {
            return resultEntry.key;
        } else {
            return undefined;
        }
    }

    /**
     * Return the key of the last element in the TreeMap
     *
     * @returns the key of the last element
     */
    getLastKey(): K {
        this.checkEmptyContainer();
        let entry = this.rootEntry!;
        while (entry.right !== undefined) {
            entry = entry.right!;
        }
        return entry.key;
    }

    /**
     * Return the lower key of the given key's element in the TreeMap
     *
     * @param key: the key of the element which will be get the lower key
     *
     * @returns the lower key of the given key's element if exists or undefined
     */
    getLowerKey(key: K): K | undefined {
        this.checkEmptyContainer();
        let resultEntry: TreeMapEntry<K, V> | undefined = undefined;
        let entry = this.rootEntry;

        while(entry !== undefined) {
            if (this.comparator!(entry!.key, key) > 0) {
                resultEntry = entry;
                entry = entry!.right;
            } else {
                entry = entry!.left;
            }
        }

        if (resultEntry !== undefined) {
            return resultEntry.key;
        } else {
            return undefined;
        }
    }

    /**
     * Check whether a key is in the TreeMap
     *
     * @param key: the key to find in the TreeMap
     *
     * @returns true if the key is in the TreeMap
     */
    hasKey(key: K): boolean {
        return this.getEntry(key) !== undefined;
    }

    /**
     * Check whether a value is in the TreeMap
     *
     * @param key: the value to find in the TreeMap
     *
     * @returns true if the value is in the TreeMap
     */
    hasValue(value: V): boolean {
        if (this.rootEntry !== undefined) {
            let entry = this.rootEntry;
            while (entry!.left !== undefined) {
                entry = entry!.left;
            }

            while (entry !== undefined) {
                if (entry!.val === value) {
                    return true;
                } else {
                    if (entry!.right !== undefined) {
                        entry = entry!.right;
                        while (entry!.left !== undefined) {
                            entry = entry!.left;
                        }
                    } else {
                        while ((entry!.parent !== undefined) && (entry === entry!.parent!.right)) {
                            entry = entry!.parent;
                        }
                        entry = entry!.parent;
                    }
                }
            }
        }

        return false;
    }

    /**
     * Check if the TreeMap is empty
     *
     * @returns true if the TreeMap is empty
     */
    isEmpty(): boolean {
        return (this.rootEntry === undefined && this.elementNum === 0);
    }

    /**
     * Remove an element from the TreeMap
     *
     * @param key: the key of the element which will be remove from the TreeMap
     *
     * @returns the value of the removed element if the element is successfully removed from the TreeSet
     */
    remove(key: K): V | undefined {
        let entry = this.getEntry(key);

        if (entry === undefined) {
            return undefined;
        }

        if (this.elementNum === 1) {
            this.clear();
            return entry.val;
        }

        if ((entry.left !== undefined) && (entry.right !== undefined)) {
            let deleteEntry = entry.right!;
            while (deleteEntry.left !== undefined) {
                deleteEntry = deleteEntry.left!;
            }
            const entryKey: K = entry.key;
            const entryValue: V = entry.val;
            entry.key = deleteEntry.key;
            entry.val = deleteEntry.val;
            entry = deleteEntry;
            deleteEntry.key = entryKey;
            deleteEntry.val = entryValue;
        }

        if ((entry.left !== undefined) || (entry.right !== undefined) || !entry.isBlack) {
            const child = entry.left !== undefined ? entry.left : entry.right;
            if (child !== undefined) {
                child.parent = entry.parent;
            }
            if (entry === this.rootEntry) {
                this.rootEntry = child;
            } else {
                if (entry === entry.parent!.left) {
                    entry.parent!.left = child;
                } else {
                    entry.parent!.right = child;
                }
            }
            if (entry.isBlack) {
                child!.isBlack = true;
            }
        } else {
            let current = entry;
            while (1) {
                if (!current.parent!.isBlack) {
                    current.parent!.isBlack = true;
                    if (current === current.parent!.left) {
                        current.parent!.left = undefined;
                        current.parent!.right!.isBlack = false;
                    } else {
                        current.parent!.right = undefined;
                        current.parent!.left!.isBlack = false;
                    }
                } else {
                    let parent = current.parent!;
                    if (current === parent.left) {
                        let brother = parent.right!;
                        parent.left = undefined;
                        if (!brother.isBlack) {
                            brother.isBlack = true;
                            brother.left!.isBlack = false;
                            this.rotateLeft(parent);
                        } else {
                            if ((brother.left === undefined) && (brother.right === undefined)) {
                                brother.isBlack = false;
                                if (parent !== this.rootEntry) {
                                    this.adjustUpward(parent);
                                }
                            } else if (brother.right !== undefined) {
                                brother.right!.isBlack = true;
                                this.rotateLeft(parent);
                            } else {
                                brother.left!.isBlack = true;
                                this.rotateRight(brother);
                                this.rotateLeft(parent);
                            }
                        }
                    } else {
                        let brother = parent.left!;
                        parent.right = undefined;
                        if (!brother.isBlack) {
                            brother.isBlack = true;
                            brother.right!.isBlack = false;
                            this.rotateRight(parent);
                        } else {
                            if ((brother.left === undefined) && (brother.right === undefined)) {
                                brother.isBlack = false;
                                if (parent !== this.rootEntry) {
                                    this.adjustUpward(parent);
                                }
                            } else if (brother.left !== undefined) {
                                brother.left!.isBlack = true;
                                this.rotateRight(parent);
                            } else {
                                brother.right!.isBlack = true;
                                this.rotateLeft(brother);
                                this.rotateRight(parent);
                            }
                        }
                    }
                }
                break;
            }
        }
        this.elementNum = this.elementNum - 1;
        return entry?.val;
    }

    /**
     * Replace the value of the given key's element in the TreeMap
     *
     * @param key: the key of the element which will be replaced the value
     * @param value: the value which will be replaced to
     *
     * @returns true if the value is replaced successfully
     */
    replace(key: K, newValue: V): boolean {
        const entry = this.getEntry(key);

        if (entry !== undefined) {
            entry.val = newValue;
            return true;
        }

        return false;
    }

    /**
     * Set a new element into the TreeMap
     *
     * @param key: the key of the new element to be added into the TreeMap
     * @param value: the value of the new element to be added into the TreeMap
     *
     * @returns the TreeMap
     */
    set(key: K, value: V): Object {
        const entry = this.getEntry(key);
        if (entry !== undefined) {
            entry.val = value;
            return this;
        }

        if (this.rootEntry === undefined) {
            this.rootEntry = new TreeMapEntry<K, V>(key, value);
            this.rootEntry!.isBlack = true;
            this.elementNum = this.elementNum + 1;
            return this;
        }

        let current = this.rootEntry;
        let parent = current;
        while (current !== undefined) {
            parent = current;
            if (this.comparator!(current!.key, key) > 0) {
                current = current!.right;
            } else {
                current = current!.left;
            }
        }

        current = new TreeMapEntry<K, V>(key, value);
        if (this.comparator!(parent!.key, current!.key) > 0) {
            parent!.right = current;
        } else {
            parent!.left = current;
        }
        current.parent = parent;

        while ((parent !== undefined) && !parent!.isBlack) {
            const grand = parent!.parent!;
            parent!.isBlack = true;
            grand.isBlack = false;

            if (parent === grand.left) {
                const uncle = grand.right;
                if ((uncle !== undefined) && !uncle.isBlack) {
                    uncle.isBlack = true;
                    current = grand;
                    parent = current.parent;
                } else {
                    if (current === parent!.right) {
                        this.rotateLeft(parent!);
                        parent!.isBlack = false;
                        current!.isBlack = true;
                    }
                    this.rotateRight(grand);
                    break;
                }
            } else {
                const uncle = grand.left;
                if ((uncle !== undefined) && !uncle.isBlack) {
                    uncle.isBlack = true;
                    current = grand;
                    parent = current.parent;
                } else {
                    if (current === parent!.left) {
                        this.rotateRight(parent!);
                        parent!.isBlack = false;
                        current!.isBlack = true;
                    }
                    this.rotateLeft(grand);
                    break;
                }
            }
        }

        this.rootEntry!.isBlack = true;
        this.elementNum = this.elementNum + 1;
        return this;
    }

    /**
     * Set all elements of the given TreeMap into the TreeMap
     *
     * @param map: the given TreeMap which elements will be set into the TreeMap
     */
    setAll(map: TreeMap<K, V>): void {
        let entry = map.rootEntry;

        if (entry === undefined) {
            return;
        }

        while (entry!.left !== undefined) {
            entry = entry!.left;
        }

        while (entry !== undefined) {
            const resEntry = this.getEntry(entry!.key);
            if (resEntry !== undefined) {
                resEntry.val = entry!.val;
            } else {
                this.set(entry!.key, entry!.val);
            }

            if (entry!.right !== undefined) {
                entry = entry!.right;
                while (entry!.left !== undefined) {
                    entry = entry!.left;
                }
            } else {
                while ((entry!.parent !== undefined) && (entry === entry!.parent!.right)) {
                    entry = entry!.parent;
                }
                entry = entry!.parent;
            }
        }
    }

    /**
     * Returns an iterator for the TreeMap
     *
     * @returns An iterator for the TreeMap
     */
    override $_iterator(): IterableIterator<[K, V]> {
        return this.entries();
    }

    private checkEmptyContainer(): void {
        if (this.isEmpty()) {
            throw createBusinessError(EmptyContainerId, `Container is empty`);
        }
    }
}
