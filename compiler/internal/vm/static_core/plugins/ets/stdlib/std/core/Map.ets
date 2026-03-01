/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package std.core;

export interface ReadonlyMap<K, V> extends Iterable<[K, V]> {
    /**
     * Returns number of Entries with unique keys in the Map
     *
     * @returns number of Entries with unique keys in the Map
     */
    get size(): int;

    /**
     * Returns a value assosiated with key if present
     *
     * @param k the key to find in the Map/class
     *
     * @returns value if assosiated with key presents.
     */
    get(key: K): V | undefined;

    /**
     * Checks if a key is in the Map
     *
     * @param k the key to find in the Map
     *
     * @returns true if the value is in the Map
     */
    has(key: K): boolean;

    /**
     * Executes a provided function once per each key/value pair in the Map, in insertion order
     *
     * @param callbackfn to apply
     */
    forEach(callbackfn: (value: V, key: K, map: ReadonlyMap<K, V>) => void): void;

    /**
     * Returns elements from the Map as an keys Iterator
     *
     * @returns ValueIterator with map keys
     */
    keys(): IterableIterator<K>;

    /**
     * Returns elements from the Map as an values Iterator
     *
     * @returns ValueIterator with map values
     */
    values(): IterableIterator<V>;

    /**
     * Returns elements from the Map as an array of Entries
     *
     * @returns an array of Entries
     */
    entries(): IterableIterator<[K, V]>;
}


type DataArrType = FixedArray<Any>

type At = int  // (index + pair) if positive, special values otherwise.
type BucketsArrType = FixedArray<At>

final class Const {
    // DataArrType
    static readonly KEY_DATA_OFS: int = 0  // First in the pair
    static readonly VAL_DATA_OFS: int = 1  // Second in the pair
    static readonly BUCKETS_FROM_DATA_OFS: int = -2  // From the end
    static readonly NESTED_DATA_OFS: int = -1  // From the end

    // At
    static readonly EMPTY_ENTRY: At = 0  // Not filled yet - can be used as the margin for an empty tail (auto set on allocation).
    static readonly DELETED_ENTRY: At = -2  // Entry has been deleted.
    static readonly BUCKET_END: At = -1  // End of the chain of buckets with the same hash index.

    // BucketsArrType
    static readonly DATA_BUCK_OFS: int = 0  // First in the pair
    static readonly NEXT_BUCK_OFS: int = 1  // Second in the pair
    static readonly NDEL_BUCK_OFS: int = 1  // Second in the pair (after rehash or clear)
    static readonly CLEAR_FLAG_IDX: int = 0  // If the table has been cleared, then there are no numbers of deleted elements in it - check this flag.
    static readonly ITER_CNT_IDX: int = -1  // From the end
}


function pairs(ofs: int): int {  // There are pairs, so we need to move offset by 2
    return 2 * ofs
}

function getAtZero(): int {
    return pairs(1)
}

function isLegalAt(at: At): boolean {
    return at > 0
}

function isInsideAt(at: At, length: int): boolean {
    return at <= length
}

function getIdxAt(at: At): int {
    return at - pairs(1)
}
function getAtByIdx(idx: int): At {
    return idx + pairs(1)
}


function getDataIdxLength(this: DataArrType): int {
    return this.length - 2
}

// Replace with reciver (like getBucketsIdxLength) here and later after issue 31223
function getKeyByIdx<K>(data: DataArrType, idx: int): K {
    return data[idx + Const.KEY_DATA_OFS] as K
}
function getKeyAt<K>(data: DataArrType, at: At): K {
    return getKeyByIdx<K>(data, getIdxAt(at))
}

function getValueByIdx<V>(data: DataArrType, idx: int): V {
    return data[idx + Const.VAL_DATA_OFS] as V
}
function getValueAt<V>(data: DataArrType, at: At): V {
    return getValueByIdx<V>(data, getIdxAt(at))
}


function getBucketsIdxLength(this: BucketsArrType): int {
    return this.length - 1
}

function isDeletedByIdx(this: BucketsArrType, idx: int): boolean {
    return this[idx + Const.NEXT_BUCK_OFS] === Const.DELETED_ENTRY
}
function isDeletedAt(this: BucketsArrType, at: At): boolean {
    return this.isDeletedByIdx(getIdxAt(at))
}

function setDeletedByIdx(this: BucketsArrType, idx: int) {
    this[idx + Const.NEXT_BUCK_OFS] = Const.DELETED_ENTRY
}
function setDeletedAt(this: BucketsArrType, at: At) {
    this.setDeletedByIdx(getIdxAt(at))
}

function isEmptyAt(this: BucketsArrType, at: int): boolean {
    return this[getIdxAt(at) + Const.NEXT_BUCK_OFS] === Const.EMPTY_ENTRY
}

function getClearFlag(this: BucketsArrType): boolean {
    return this[Const.CLEAR_FLAG_IDX] !== 0
}
function setClearFlag(this: BucketsArrType, state: boolean) {
    return this[Const.CLEAR_FLAG_IDX] = (state ? 1 : 0)
}

function getNumOfDeletedAt(this: BucketsArrType, at: At): int { // Call only when clear or rehash occured.
    if (this.getClearFlag()) {
        return at / pairs(1)
    }
    return isLegalAt(at) ? this[getIdxAt(at) + Const.NDEL_BUCK_OFS] : 0
}
function setNumOfDeletedByIdx(this: BucketsArrType, idx: int, num: int) {
    this[idx + Const.NDEL_BUCK_OFS] = num
}

function getItersCounter(this: BucketsArrType): int {
    return this[this.length + Const.ITER_CNT_IDX]
}
function setItersCounter(this: BucketsArrType, cnt: int) {
    this[this.length + Const.ITER_CNT_IDX] = cnt
}
function chgItersCounter(this: BucketsArrType, chg: int) {
    let itersCounter = this.getItersCounter()
    if (itersCounter != Int.MAX_VALUE) {
        this.setItersCounter(itersCounter + chg)
    }
}

function getBucketsFromData(this: DataArrType): BucketsArrType {
    return this[this.length + Const.BUCKETS_FROM_DATA_OFS] as BucketsArrType
}
function setBucketsToData(this: DataArrType, buckets: BucketsArrType) {
    this[this.length + Const.BUCKETS_FROM_DATA_OFS] = buckets
}

function getNestedData(this: DataArrType): DataArrType | undefined {
    return this[this.length + Const.NESTED_DATA_OFS] as DataArrType | undefined
}
function setNestedData(this: DataArrType, nestedData: DataArrType) {
    this[this.length + Const.NESTED_DATA_OFS] = nestedData
    let nestedBuckets = nestedData.getBucketsFromData()
    let buckets = this.getBucketsFromData()
    nestedBuckets.setItersCounter(buckets.getItersCounter())
}

export interface MapIterator<R> extends IterableIterator<R> {}

final class MapIteratorImpl<K, V, R> implements MapIterator<R> {
    private data: DataArrType
    private buckets: BucketsArrType
    private curAt: At

    private mapper: (k: K, v: V) => R

    constructor(data: FixedArray<Any>, mapper: (k: K, v: V) => R) {
        this.data = data
        this.buckets = data.getBucketsFromData()
        this.buckets!.chgItersCounter(+1)
        this.curAt = getAtZero()
        this.mapper = mapper
    }

    override next(): IteratorResult<R> {
        if (!isLegalAt(this.curAt)) {
            return new IteratorResult<R>()
        }

        let data = this.data
        let buckets = this.buckets
        while (true) {
            let nestedData = getNestedData(data)
            if (nestedData === undefined) {
                break
            }

            if (this.curAt > getAtZero()) {
                this.curAt -= pairs(buckets!.getNumOfDeletedAt(this.curAt - pairs(1)))
            }
            data = nestedData as DataArrType
            buckets = data.getBucketsFromData()
        }
        this.data = data
        this.buckets = buckets

        for (const length = (this.buckets!.getBucketsIdxLength()); isInsideAt(this.curAt, length); this.curAt += pairs(1)) {
            if (this.buckets!.isEmptyAt(this.curAt)) {
                break
            }
            if (!this.buckets!.isDeletedAt(this.curAt)) {
                const res = new IteratorResult<R>(this.mapper(getKeyAt<K>(this.data!, this.curAt), getValueAt<V>(this.data!, this.curAt)))
                this.curAt += pairs(1)
                return res
            }
        }

        this.curAt = Const.EMPTY_ENTRY
        this.buckets!.chgItersCounter(-1)
        return new IteratorResult<R>()
    }

    override $_iterator(): IterableIterator<R> {
        return this
    }
}

final class EmptyMapIteratorImpl<R> implements MapIterator<R> {
    override next(): IteratorResult<R> {
        return new IteratorResult<R>()
    }

    override $_iterator(): IterableIterator<R> {
        return this
    }
}

export class Map<K, V> implements ReadonlyMap<K, V> {
    private data: DataArrType  // Contains [key, value] pairs in a flat array. Last two cells are reference to buckets and reference to new .data if set.
    private buckets: BucketsArrType  // Contains [at1, at2] pairs in a flat array, where at1 points to data and at2 has differen meanings (see declaration of the type 'At' and special values from class 'Const').
                                     // After rehash 'at2' contains not a link, but the number of deleted elements in the 'data' up to current position.
                                     // At the end there is extra element - the number of existing iterators and active calls of 'forEach' function.
                                     // If it's not zero, we need to prepare old 'data' and 'buckets' arrays, which will be used to move the current position after rehash or clear.
                                     // If it is zero, no need to waste resources and link old 'data' array to new one.
    private cap: int  // Maximum number of entries
    private numEntries: int = 0  // Current number of entries
    private sizeVal: int = 0  // Current number of elements
    private initialCapacity: int = Map.MIN_CAPACITY  // The capacity used to create the Map.

    private static readonly MIN_CAPACITY = 4
    private static readonly MIN_SHRINK_CAPACITY = 16
    private static readonly ENTRY_KEY = 0
    private static readonly ENTRY_VAL = 1

    private static adjustCapacity(numEntries: int): int {
        if (numEntries <= Map.MIN_CAPACITY)
            return Map.MIN_CAPACITY
        return Math.ceilPow2(numEntries)
    }

    private static computeCapacity(numEntries: int): int {
        return Map.adjustCapacity(numEntries + numEntries / 2)
    }

    private static computeCapacityForGrow(capacity: int, size: int, numEntries: int): int {
        const deleted = numEntries - size
        if (deleted >= capacity / 2) {
            // just clear out deleted elements
            return capacity
        }

        // grow
        return capacity * 2
    }

    private static getNextByIdx(buckets: BucketsArrType, idx: int): At {
        return buckets[idx + Const.NEXT_BUCK_OFS]
    }
    private getNextByIdx(idx: int): At {
        return Map.getNextByIdx(this.buckets, idx)
    }
    private getNextAt(at: At): At {
        return this.getNextByIdx(getIdxAt(at))
    }
    private static setNextByIdx(buckets: BucketsArrType, idx: int, next: At) {
        buckets[idx + Const.NEXT_BUCK_OFS] = next
    }
    private setNextByIdx(idx: int, next: At) {
        Map.setNextByIdx(this.buckets, idx, next)
    }
    private setNextAt(at: At, next: At) {
        this.setNextByIdx(getIdxAt(at), next)
    }

    private static getBucketByIdx(buckets: BucketsArrType, idx: int): At {
        return buckets[idx + Const.DATA_BUCK_OFS]
    }
    private getBucketByIdx(idx: int): At {
        return Map.getBucketByIdx(this.buckets, idx)
    }
    private static setBucketByIdx(buckets: BucketsArrType, idx: int, bucket: At) {
        buckets[idx + Const.DATA_BUCK_OFS] = bucket
    }
    private setBucketByIdx(idx: int, bucket: At) {
        Map.setBucketByIdx(this.buckets, idx, bucket)
    }

    private static setKey<K>(data: DataArrType, idx: int, key: K) {
        data[idx + Const.KEY_DATA_OFS] = key
    }
    private setKey(idx: int, key: K) {
        Map.setKey(this.data, idx, key)
    }

    private static setValue<V>(data: DataArrType, idx: int, val: V) {
        data[idx + Const.VAL_DATA_OFS] = val
    }
    private setValue(idx: int, val: V) {
        Map.setValue(this.data, idx, val)
    }

    private deleteAt(at: At) {
        // NOTE(nkholiavin): maybe need to set deleted keys, values to undefined
        this.buckets.setDeletedAt(at)
    }

    private static getBucketIndex(capacity: int, k: Any): int {
        if (k === null) {
            return pairs(0)
        }
        if (k === undefined) {
            return pairs(1)
        }

        const keyHash: int = Runtime.getHashCodeByValue(k! as object).toInt() // #26217
        const t = keyHash >> 31

        return pairs(((keyHash ^ t) - t) & (capacity - 1))
    }
    private getBucketIndex(k: Any): int {
        return Map.getBucketIndex(this.cap, k)
    }

    private allocDataAndBuckets(capacity: int): [DataArrType, BucketsArrType] {
        const data: DataArrType = new FixedArray<Any>(pairs(capacity) + 2)  // extra two elements at the end are: links to buckets and nested data.
        const buckets: BucketsArrType = new FixedArray<int>(pairs(capacity) + 1)  // extra element at the end is: iterators counter.
        data.setBucketsToData(buckets)
        return [data, buckets]
    }

    // NOTE: This API may be removed someday due #28030 internal issue
    constructor(initialCapacity: int) {
        const capacity = Map.adjustCapacity(initialCapacity)
        const alloc = this.allocDataAndBuckets(capacity)
        this.data = alloc[0]
        this.buckets = alloc[1]
        this.cap = capacity
        this.initialCapacity = capacity
    }

    // NOTE(templin.konstantin): Temporary workaround due #20944 internal issue
    //  if we remove this constructor we got error on "readonly (readonly [K,V])[]"
    //  branch at constructor from union type
    constructor(values: FixedArray<[K, V]>) {
        const capacity = Map.computeCapacity(values.length)
        const alloc = this.allocDataAndBuckets(capacity)
        this.data = alloc[0]
        this.buckets = alloc[1]
        this.cap = capacity
        this.initialCapacity = capacity

        for (let i = 0; i < values.length; ++i) {
            let pair = values[i]
            if (pair == null) {
                continue
            }
            this.set(pair[Map.ENTRY_KEY] as K, pair[Map.ENTRY_VAL] as V)
        }
    }

    /**
     * Constructs a Map from Array of entries
     * @param entries Array of entries
     */
    constructor(entries: Array<[K, V]>) {
        const capacity = Map.computeCapacity(entries.length)
        const alloc = this.allocDataAndBuckets(capacity)
        this.data = alloc[0]
        this.buckets = alloc[1]
        this.cap = capacity
        this.initialCapacity = capacity

        entries.forEach((entry: [K, V]) => {
            this.set(entry[Map.ENTRY_KEY] as K, entry[Map.ENTRY_VAL] as V)
        })
    }

    /**
     * Constructs a Map from another Map
     * @param map another map
     */
    constructor(map: Map<K, V>) {
        const capacity = Map.computeCapacity(map.sizeVal)
        const alloc = this.allocDataAndBuckets(capacity)
        this.data = alloc[0]
        this.buckets = alloc[1]
        this.cap = capacity
        this.initialCapacity = capacity

        map.forEach((v: V, k: K) => {
            this.set(k, v)
        })
    }

    /**
     * Constructs a Map from collection
     * @param entries initial collection
     */
    constructor(entries?: Iterable<[K, V]> | readonly ((readonly [K, V]) | null | undefined)[] | null) {
        if (entries == null) {
            const alloc = this.allocDataAndBuckets(Map.MIN_CAPACITY)
            this.data = alloc[0]
            this.buckets = alloc[1]
            this.cap = Map.MIN_CAPACITY
        } else if (entries instanceof Array) {
            const array = entries as Array<(readonly [K, V]) | null | undefined>
            const capacity = Map.computeCapacity(array.length.toInt())
            const alloc = this.allocDataAndBuckets(capacity)
            this.data = alloc[0]
            this.buckets = alloc[1]
            this.cap = capacity

            for (let i = 0; i < array.length; ++i) {
                let pair = array[i]
                if (pair == undefined || pair == null) {
                    continue
                }
                this.set(pair[Map.ENTRY_KEY] as K, pair[Map.ENTRY_VAL] as V)
            }
        } else {
            const entriesIter = (entries as Iterable<[K, V]>).$_iterator()
            const entriesBuf = new Array<[K, V]>()
            for (let iterRes = entriesIter.next(); !iterRes.done; iterRes = entriesIter.next()) {
                const entry = iterRes.value
                if (entry != null) {
                    entriesBuf.push(entry)
                }
            }
            const capacity = Map.computeCapacity(entriesBuf.length)
            const alloc = this.allocDataAndBuckets(capacity)
            this.data = alloc[0]
            this.buckets = alloc[1]
            this.cap = capacity

            entriesBuf.forEach((e: [K, V]) => {
                this.set(e[Map.ENTRY_KEY] as K, e[Map.ENTRY_VAL] as V)
            })
        }
        this.initialCapacity = this.cap
    }

    override toString(): String {
        if(this.sizeVal == 0) {
            return "[object Map]"
        }
        const strBuf = new StringBuilder()

        const entriesIter = this.entries()
        let entriesIterRes = entriesIter.next()
        while (!entriesIterRes.done) {
            const entry = entriesIterRes.value
            if (entry != null) {
                strBuf.append(entry!)
            }

            entriesIterRes = entriesIter.next()
            if (!entriesIterRes.done) {
                strBuf.append(",")
            }
        }

        return strBuf.toString()
    }

    private rehash(newCapacity: int) {
        let curDeletedElements: int = 0

        let alloc = this.allocDataAndBuckets(newCapacity)
        const newData = alloc[0]
        const newBuckets = alloc[1]

        const oldNumEntries = this.numEntries
        const oldData = this.data
        const oldBuckets = this.buckets

        this.sizeVal = 0
        let sizeVal: int = 0

        const hasIters: boolean = (oldBuckets.getItersCounter() != 0)
        for (let i: int = 0; i < pairs(oldNumEntries); i += pairs(1)) {
            const isDeleted: boolean = oldBuckets.isDeletedByIdx(i)
            if (hasIters) {
                if (isDeleted) {
                    oldBuckets.setNumOfDeletedByIdx(i, ++curDeletedElements)  // it means number of deleted so far now.
                    continue
                }
                oldBuckets.setNumOfDeletedByIdx(i, curDeletedElements)  // it means number of deleted so far now.
            } else {
                if (isDeleted) {
                    continue
                }
            }

            const key = getKeyByIdx<K>(oldData, i)
            const val = getValueByIdx<V>(oldData, i)
            let bucketIdx = Map.getBucketIndex(newCapacity, key)
            const bucket = Map.getBucketByIdx(newBuckets, bucketIdx)
            const newIdx = pairs(sizeVal++)
            Map.setKey(newData, newIdx, key)
            Map.setValue(newData, newIdx, val)
            Map.setNextByIdx(newBuckets, newIdx, isLegalAt(bucket) ? bucket : Const.BUCKET_END)
            Map.setBucketByIdx(newBuckets, bucketIdx, getAtByIdx(newIdx))
        }

        this.numEntries = this.sizeVal = sizeVal
        if (hasIters) {
            setNestedData(this.data, newData)
            oldBuckets.setClearFlag(false)
        }
        this.data = newData
        this.buckets = newBuckets
        this.cap = newCapacity
    }

    private static native sameValueZero(o1: Any, o2: Any): boolean

    /**
     * Puts a pair of key and value into the Map
     *
     * @param key the key to put into the Map
     *
     * @param val the value to put into the Map
     */
    set(key: K, val: V): this {
        // rehash
        if (this.numEntries * 4 >= this.cap * 3) {
            this.rehash(Map.computeCapacityForGrow(this.cap, this.sizeVal, this.numEntries))
        }

        // check bucket
        let bucketIdx = this.getBucketIndex(key)
        let bucket = this.getBucketByIdx(bucketIdx)
        if (isLegalAt(bucket)) {
            let at = bucket
            do {
                if (Map.sameValueZero(getKeyAt<K>(this.data, at), key)) {
                    // found
                    this.setValue(getIdxAt(at), val)
                    return this
                }
                at = this.getNextAt(at)
            } while (isLegalAt(at))
        } else {
            bucket = Const.BUCKET_END
        }

        // insert
        const newIdx = pairs(this.numEntries++)
        this.setKey(newIdx, key)
        this.setValue(newIdx, val)
        this.setNextByIdx(newIdx, bucket)
        this.setBucketByIdx(bucketIdx, getAtByIdx(newIdx))
        this.sizeVal++
        return this
    }

    /**
     * Checks if a key is in the Map
     *
     * @param key the key to find in the Map
     *
     * @returns true if the value is in the Map
     */
    override has(key: K): boolean {
        const bucketIdx = this.getBucketIndex(key)
        let bucket = this.getBucketByIdx(bucketIdx)

        while (isLegalAt(bucket)) {
            if (Map.sameValueZero(getKeyAt<K>(this.data, bucket), key)) {
                return true
            }
            bucket = this.getNextAt(bucket)
        }

        return false
    }

    /**
     * Returns number of Entries with unique keys in the Map
     *
     * @returns number of Entries with unique keys in the Map
     */
    override get size(): int {
        return this.sizeVal
    }

    private shrink() {
        const cap = this.cap
        if (cap <= Map.MIN_SHRINK_CAPACITY) {  // too small
            return
        }
        if (this.sizeVal >= cap / 4) {  // filled enough
            return
        }

        // quarter filled
        const newCap = cap / 2
        if (newCap >= this.initialCapacity) {
            this.rehash(newCap)
        }
    }

    /**
     * Removes an Entry with specified key from the Map
     *
     * @param key the key to remove
     */
    delete(key: K): boolean {
        const bucketIdx = this.getBucketIndex(key)
        let bucket = this.getBucketByIdx(bucketIdx)
        let prev: At = 0

        while (isLegalAt(bucket)) {
            if (Map.sameValueZero(getKeyAt<K>(this.data, bucket), key)) {
                if (isLegalAt(prev)) {
                    this.setNextAt(prev, this.getNextAt(bucket))
                } else {
                    this.setBucketByIdx(bucketIdx, this.getNextAt(bucket))
                }
                this.deleteAt(bucket)
                this.sizeVal--
                this.shrink()
                return true
            }
            prev = bucket
            bucket = this.getNextAt(bucket)
        }

        return false
    }

    /**
     * Deletes all elements from the Map
     */
    clear(): void {
        const capacity = this.initialCapacity
        if (this.sizeVal === 0 && this.cap === capacity) {
            return
        }

        let alloc = this.allocDataAndBuckets(capacity)
        const oldData = this.data
        this.data = alloc[0]
        if (this.buckets.getItersCounter() != 0) {
            setNestedData(oldData, this.data)
            this.buckets.setClearFlag(true)
        }
        this.buckets = alloc[1]
        this.cap = capacity
        this.sizeVal = 0
        this.numEntries = 0
    }

    /**
     * Returns a value assosiated with key if present
     *
     * @param key the key to find in the Map
     *
     * @returns value if assosiated with key presents.
     */
    override get(key: K): V | undefined {
        const bucketIdx = this.getBucketIndex(key)
        let bucket = this.getBucketByIdx(bucketIdx)

        while (isLegalAt(bucket)) {
            if (Map.sameValueZero(getKeyAt<K>(this.data, bucket), key)) {
                return getValueAt<V>(this.data, bucket)
            }
            bucket = this.getNextAt(bucket)
        }

        return undefined
    }

    /**
     * Returns a value assosiated with key if present, or a default value otherwise
     *
     * @param key the key to find in the Map
     *
     * @param def a value to return if key is not in the Map
     *
     * @returns value if key presents, def otherwise
     */
    get(key: K, def: V): V {
        return this.get(key) ?? def
    }

    private mappedIterator<R>(fn: (k: K, v: V) => R): IterableIterator<R> {
        if (this.sizeVal > 0) {
            return new MapIteratorImpl<K, V, R>(this.data, fn)
        } else {
            return new EmptyMapIteratorImpl<R>()
        }
    }

    /**
     * Returns elements from the Map as an keys Iterator
     *
     * @returns iterator with map keys
     */
    override keys(): IterableIterator<K> {
        return this.mappedIterator<K>((k: K, v: V): K => k)
    }

    /**
     * Returns elements from the Map as an values Iterator
     *
     * @returns iterator with map values
     */
    override values(): IterableIterator<V> {
        return this.mappedIterator<V>((k: K, v: V): V => v)
    }

    override $_iterator(): IterableIterator<[K, V]> {
        return this.entries()
    }

    /**
     * Returns elements from the Map as an array of Entries
     *
     * @returns an array of Entries
     */
    override entries(): IterableIterator<[K, V]> {
        return this.mappedIterator<[K, V]>((k: K, v: V): [K, V] => [k, v])
    }

    /**
     * Executes a provided function once per each key/value pair in the Map, in insertion order
     *
     * @param callbackfn to apply
     */
    override forEach(callbackfn: (v: V, k: K, map: Map<K, V>) => void): void {
        let curData = this.data
        this.buckets.chgItersCounter(+1)

        // NOTE(nkholiavin): need to use try/finally for decrement the counter back, but it costs too much (issue 31274)
        for (let curAt = getAtZero(); isInsideAt(curAt, pairs(this.numEntries)); curAt += pairs(1)) {
            if (this.buckets.isDeletedAt(curAt))
                continue

            callbackfn(getValueAt<V>(this.data, curAt), getKeyAt<K>(this.data, curAt), this)

            while (!Runtime.isSameReference(curData, this.data)) {
                let curBuckets = curData.getBucketsFromData()
                curAt -= pairs(curBuckets!.getNumOfDeletedAt(curAt))
                curData = getNestedData(curData) as DataArrType
            }
        }
        this.buckets.chgItersCounter(-1)
    }
}

export class Record<K extends Numeric | string | BaseEnum<Int> | BaseEnum<Long> | BaseEnum<string>, V> extends Map<K, V> {
    $_get(k : K) : V | undefined {
        return super.get(k)
    }

    $_set(k: K, v: V) : void {
        super.set(k, v)
    }
}
