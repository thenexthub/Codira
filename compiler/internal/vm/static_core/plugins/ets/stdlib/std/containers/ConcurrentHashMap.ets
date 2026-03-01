/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

package std.containers;

export namespace containers {

    final class MapNode<K, V> {
        prev: MapNode<K, V>;
        next: MapNode<K, V>;

        key: K;
        val: V;
        isDeleted: boolean = false;

        constructor(key: K, val: V) {
            this.prev = this
            this.next = this
            this.key = key;
            this.val = val;
        }

        toString(): string {
            return `{${this.key},${this.val}}`;
        }
    }

    final class ConcurrentHashMapIterator<K, V, R> implements IterableIterator<R> {
        private node: MapNode<K, V>;
        private tail: MapNode<K, V>;
        private mapper: (e: MapNode<K, V>) => R;
        private mutex: Object;

        constructor(mutex: Object, node: MapNode<K, V>, tail: MapNode<K, V>, mapper: (e: MapNode<K, V>) => R) {
            this.mutex = mutex;
            this.node = node;
            this.tail = tail;
            this.mapper = mapper;
        }

        override next(): IteratorResult<R> {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                const nextNode = this.node.next;
                if (nextNode === this.tail) {
                    return new IteratorResult<R>();
                }
                this.node = nextNode;
                return new IteratorResult<R>(this.mapper(this.node));
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }

        }

        override $_iterator(): IterableIterator<R> {
            return this;
        }
    }

    final class EmptyConcurrentHashMapIterator<R> implements IterableIterator<R> {
        override next(): IteratorResult<R> {
            return new IteratorResult<R>();
        }

        override $_iterator(): IterableIterator<R> {
            return this;
        }
    }

    function iteratorForEach<V>(x: Iterator<V>, fn: (x: V) => void): void {
        while (true) {
            const v = x.next()
            if (v.done) {
                return
            }
            fn(v.value as V)
        }
    }

    final class AtomicsCounter {
        private count: int = 0;
        private mutex: Object = ConcurrencyHelpers.mutexCreate();

        constructor(val: int) {
            this.count = val;
        }

        public increase(): void {
            ConcurrencyHelpers.mutexLock(this.mutex);
            this.count++;
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }

        public decrease(): void {
            ConcurrencyHelpers.mutexLock(this.mutex);
            this.count--;
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }

        public get(): int {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                return this.count;
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        public set(val: int): void {
            ConcurrencyHelpers.mutexLock(this.mutex);
            this.count = val;
            ConcurrencyHelpers.mutexUnlock(this.mutex);
        }
    }

    type ConcurrentHashMapBucket<K, V> = Array<MapNode<K, V>> | undefined;

    export class ConcurrentHashMap<K, V> implements ReadonlyMap<K, V> {
        private headNode: MapNode<K, V> | undefined = undefined;

        private buckets: ConcurrentHashMapBucket<K, V>[] = [];
        private bucketMutex: (Object | undefined)[] = [];
        private mutex: Object = ConcurrencyHelpers.mutexCreate();
        private recreateMutex: Object = ConcurrencyHelpers.mutexCreate();
        private bucketsMutex: Object = ConcurrencyHelpers.mutexCreate();
        private delMutex: Object = ConcurrencyHelpers.mutexCreate();
        private sizeVal: AtomicsCounter = new AtomicsCounter(0);
        private setVal: AtomicsCounter = new AtomicsCounter(0);

        private static readonly ENTRY_KEY = 0;
        private static readonly ENTRY_VAL = 1;

        private static readonly MIN_BUCKETS_COUNT = 8;

        private getBucketIndex(k: Any): int {
            if (k === null) {
                return 0;
            }
            if (k === undefined) {
                return 1;
            }

            const keyHash: int = Runtime.getHashCodeByValue(k! as object).toInt(); // #26217
            const t = keyHash >> 31;

            return ((keyHash ^ t) - t) & (this.buckets.length - 1);
        }

        private getInitialBucketsCount(entriesCount: int): int {
            const bucketsCount = Math.ceilPow2(entriesCount * 4 / 3 + 1);

            return (bucketsCount >= ConcurrentHashMap.MIN_BUCKETS_COUNT) ?
                bucketsCount : ConcurrentHashMap.MIN_BUCKETS_COUNT;
        }

        private getBucket(k: K): ConcurrentHashMapBucket<K, V> {
            ConcurrencyHelpers.mutexLock(this.bucketsMutex);
            try {
                return this.buckets[this.getBucketIndex(k)];
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.bucketsMutex);
            }
        }

        private getOrCreateBucket(k: K): Array<MapNode<K, V>> {
            const idx = this.getBucketIndex(k);
            const bucket = this.buckets[idx];
            if (bucket === undefined) {
                const newBucket = new Array<MapNode<K, V>>();
                this.buckets[idx] = newBucket;
                return newBucket;
            } else {
                return bucket!;
            }
        }

        private getOrCreateBucketMutex(idx: int): Object | undefined {
            ConcurrencyHelpers.mutexLock(this.recreateMutex);
            try {
                if (idx >= this.bucketMutex.length ) {
                    return undefined;
                }
                const mutex = this.bucketMutex[idx];
                if (mutex === undefined || mutex === null) {
                    const newMutex = ConcurrencyHelpers.mutexCreate();
                    this.bucketMutex[idx] = newMutex;
                    return newMutex;
                } else {
                    return mutex!;
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.recreateMutex);
            }
        }

        private rehash(): void {
            ConcurrencyHelpers.mutexLock(this.bucketsMutex);
            // constant load factor (entries / buckets < 0.75)
            if (this.sizeVal.get() * 4 < this.buckets.length * 3) {
                ConcurrencyHelpers.mutexUnlock(this.bucketsMutex);
                return;
            }
            const newSize = this.buckets.length * 2;
            while (this.setVal.get() != 0) {
                Coroutine.Schedule();
            }
            this.buckets = new ConcurrentHashMapBucket<K, V>[newSize];
            const entriesIter = this.mappedIterator<MapNode<K, V>>((e: MapNode<K, V>): MapNode<K, V> => e)
            iteratorForEach<MapNode<K, V>>(entriesIter, (e: MapNode<K, V>): void => {
                this.getOrCreateBucket(e.key).push(e)
            })
            ConcurrencyHelpers.mutexLock(this.recreateMutex);
            const oldBucketMutex = this.bucketMutex;
            this.bucketMutex = new (Object | undefined)[newSize];
            for (let i = 0; i < oldBucketMutex.length; ++i) {
                this.bucketMutex[i] = oldBucketMutex[i];
            }
            ConcurrencyHelpers.mutexUnlock(this.recreateMutex);
            ConcurrencyHelpers.mutexUnlock(this.bucketsMutex);
        }

        private atomicBucketAt(idx: int, bucket: Array<MapNode<K, V>>, i: int): MapNode<K, V> | null {
            const mutex = this.getOrCreateBucketMutex(idx);
            if (mutex === undefined) {
                return null;
            }
            ConcurrencyHelpers.mutexLock(mutex);
            try {
                if (i >= bucket.length || i < 0) {
                    return null;
                }
                return bucket[i];
            } finally {
                ConcurrencyHelpers.mutexUnlock(mutex);
            }
        }

        private atomicSetBucketAt(idx: int, bucket: Array<MapNode<K, V>>, i: int, v: MapNode<K, V>, isPush: boolean): void {
            const mutex = this.getOrCreateBucketMutex(idx);
            if (mutex === undefined) {
                return;
            }
            ConcurrencyHelpers.mutexLock(mutex);
            try {
                if (isPush) {
                    bucket.push(v);
                } else {
                    bucket[i] = v;
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(mutex);
            }
        }

        /**
        * Construct a ConcurrentHashMap without parameters.
        */
        constructor() {
            this.buckets = new ConcurrentHashMapBucket < K, V > [ConcurrentHashMap.MIN_BUCKETS_COUNT];
            this.bucketMutex = new (Object | undefined)[ConcurrentHashMap.MIN_BUCKETS_COUNT];
        }

        /**
        * Constructs a ConcurrentHashMap from ArrayLike
        * @param { ArrayLike<[K, V]> } entries initial ArrayLike
        */
        constructor(entries: ArrayLike<[K, V]>) {
            const bucketsCount = this.getInitialBucketsCount(Double.toInt(entries.length));
            this.buckets = new ConcurrentHashMapBucket < K, V > [bucketsCount];
            this.bucketMutex = new (Object | undefined)[bucketsCount];
            for (let i = 0; i < entries.length; ++i) {
                this.set(entries[i][ConcurrentHashMap.ENTRY_KEY], entries[i][ConcurrentHashMap.ENTRY_VAL]);
            }
        }

        /**
        * Constructs a ConcurrentHashMap from any iterable
        * @param { Iterable<[K, V]> } iterable The iterable object
        */
        constructor(iterable: Iterable<[K, V]>) {
            const entriesIter = (iterable as Iterable<[K, V]>).$_iterator();
            const entriesBuf = new Array<[K, V]>();
            for (let iterRes = entriesIter.next();!iterRes.done;iterRes = entriesIter.next()) {
                const entry = iterRes.value;
                if (entry != null) {
                    entriesBuf.push(entry);
                }
            }
            const bucketsCount = this.getInitialBucketsCount(entriesBuf.length);
            this.buckets = new ConcurrentHashMapBucket<K, V>[bucketsCount];
            this.bucketMutex = new (Object | undefined)[bucketsCount];

            entriesBuf.forEach((e: [K, V]) => {
                this.set(e[ConcurrentHashMap.ENTRY_KEY], e[ConcurrentHashMap.ENTRY_VAL]);
            });
        }

        /**
        * Constructs a ConcurrentHashMap from another Map
        * @param { Map<K, V> } map another map
        */
        constructor(map: Map<K, V>) {
            const bucketsCount = this.getInitialBucketsCount(Double.toInt(map.size));
            this.buckets = new ConcurrentHashMapBucket<K, V>[bucketsCount];
            this.bucketMutex = new (Object | undefined)[bucketsCount];

            map.forEach((v: V, k: K) => {
                this.set(k, v);
            });
        }

        /**
        * Transform container to a string
        *
        * @returns { String } string created from the container
        */
        public override toString(): String {
            const strBuf = new StringBuilder();

            const entriesIter = this.entries();
            let entriesIterRes = entriesIter.next();
            while (!entriesIterRes.done) {
                const entry = entriesIterRes.value;
                if (!(entry === undefined) && !(entry === null)) {
                    strBuf.append(entry!);
                }

                entriesIterRes = entriesIter.next();
                if (!entriesIterRes.done) {
                    strBuf.append(",");
                }
            }

            return strBuf.toString();
        }

        private static native sameValueZero(o1: Any, o2: Any): boolean

        /**
        * Puts a pair of key and value into the Map
        *
        * @param { K } key the key to put into the Map
        * @param { V } val the value to put into the Map
        *
        * @returns { this } return this.
        */
        public set(key: K, val: V): this {
            // insert into buckets
            ConcurrencyHelpers.mutexLock(this.bucketsMutex);
            this.setVal.increase();
            const buck = this.getOrCreateBucket(key);
            const buckIndex = this.getBucketIndex(key);
            ConcurrencyHelpers.mutexUnlock(this.bucketsMutex);
            const buckSize = buck.length;
            ConcurrencyHelpers.mutexLock(this.mutex);
            for (let i = 0; i < buckSize; i++) {
                const node = this.atomicBucketAt(buckIndex, buck, i);
                if (node === null) {
                    continue;
                }
                if (ConcurrentHashMap.sameValueZero(node.key, key)) {
                    node.val = val;
                    this.setVal.decrease();
                    ConcurrencyHelpers.mutexUnlock(this.mutex);
                    return this;
                }
            }

            // not found, add to insertion-ordered list
            const newNode = new MapNode<K, V>(key, val);
            if (this.headNode === undefined) {
                this.headNode = new MapNode<K, V>(key, val);
            }

            const headNode = this.headNode!;
            newNode.next = headNode;
            newNode.prev = headNode.prev;
            headNode.prev.next = newNode;
            headNode.prev = newNode;
            this.sizeVal.increase();
            ConcurrencyHelpers.mutexUnlock(this.mutex);

            this.atomicSetBucketAt(buckIndex, buck, 0, newNode, true);
            this.setVal.decrease();

            this.rehash();

            return this;
        }

        /**
        * Puts a pair of key and value into the Map
        *
        * @param { K } key the key to find in the Map
        * @param { V } newValue the value to replace into the Map
        *
        * @returns { boolean } true if the delete Operation is success, otherwise false.
        */
        public replace(key: K, newValue: V): boolean {
            const buck = this.getBucket(key);
            if (buck !== undefined) {
                const buckSize = buck.length;
                ConcurrencyHelpers.mutexLock(this.bucketsMutex);
                const buckIndex = this.getBucketIndex(key);
                ConcurrencyHelpers.mutexUnlock(this.bucketsMutex);
                ConcurrencyHelpers.mutexLock(this.mutex);
                for (let i = 0; i < buckSize; i++) {
                    const node = this.atomicBucketAt(buckIndex, buck, i);
                    if (node === null) {
                        continue;
                    }
                    if (ConcurrentHashMap.sameValueZero(node.key, key)) {
                        node.val = newValue;
                        ConcurrencyHelpers.mutexUnlock(this.mutex);
                        return true;
                    }
                }
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }

            return false;
        }

        /**
        * Checks if a key is in the ConcurrentHashMap
        *
        * @param { K } key the key to find in the ConcurrentHashMap
        *
        * @returns { boolean } true if the value is in the ConcurrentHashMap
        */
        public override has(key: K): boolean {
            const buck = this.getBucket(key);
            if (buck !== undefined) {
                const buckSize = buck.length;
                ConcurrencyHelpers.mutexLock(this.bucketsMutex);
                const buckIndex = this.getBucketIndex(key);
                ConcurrencyHelpers.mutexUnlock(this.bucketsMutex);
                for (let i = 0; i < buckSize; i++) {
                    const node = this.atomicBucketAt(buckIndex, buck, i);
                    if (node === null) {
                        continue;
                    }
                    if (ConcurrentHashMap.sameValueZero(node.key, key)) {
                        return true;
                    }
                }
            }

            return false;
        }

        /**
        * Returns number of Entries with unique keys in the ConcurrentHashMap
        *
        * @returns { number } number of Entries with unique keys in the ConcurrentHashMap
        */
        public override get size(): int {
            return this.sizeVal.get();
        }

        /**
        * Removes an MapNode with specified key from the ConcurrentHashMap, and return the deleted value.
        *
        * @param { K } key the key to remove
        * @returns { V | undefined } the deleted value or undefined.
        */
        public remove(key: K): V | undefined {
            return this.deleteImpl(key);
        }

        /**
        * Removes an Entry with specified key from the ConcurrentHashMap
        *
        * @param { K } key the key to remove
        * @returns { boolean } true if the delete Operation is success, otherwise false.
        */
        public delete(key: K): boolean {
            if (this.deleteImpl(key) === undefined) {
                return false;
            }

            if (this.sizeVal.get() == 0) {
                this.headNode = undefined;
            }
            return true;
        }

        private deleteImpl(key: K): V | undefined {
            const buck = this.getBucket(key);
            if (buck === undefined) {
                return undefined;
            }

            ConcurrencyHelpers.mutexLock(this.bucketsMutex);
            const buckIndex = this.getBucketIndex(key);
            ConcurrencyHelpers.mutexUnlock(this.bucketsMutex);
            ConcurrencyHelpers.mutexLock(this.delMutex);
            const buckSize = buck.length;
            for (let i = 0; i < buckSize; i++) {
                const node = this.atomicBucketAt(buckIndex, buck, i);
                if (node === null) {
                    continue;
                }
                if (!ConcurrentHashMap.sameValueZero(node.key, key)) {
                    continue;
                }

                ConcurrencyHelpers.mutexLock(this.mutex);
                if (node.isDeleted) {
                    ConcurrencyHelpers.mutexUnlock(this.mutex);
                    continue;
                }
                node.prev.next = node.next;
                node.next.prev = node.prev;
                node.isDeleted = true;
                this.sizeVal.decrease();
                ConcurrencyHelpers.mutexUnlock(this.mutex);

                const buckMutex = this.getOrCreateBucketMutex(buckIndex);
                if (buckMutex !== undefined) {
                    ConcurrencyHelpers.mutexLock(buckMutex);
                    const popped = buck.pop()!;
                    if (i != buckSize - 1 || buck.length > buckSize) {
                        buck[i] = popped;
                    }
                    ConcurrencyHelpers.mutexUnlock(buckMutex);
                }
                ConcurrencyHelpers.mutexUnlock(this.delMutex);
                return node.val;
            }
            ConcurrencyHelpers.mutexUnlock(this.delMutex);
            return undefined;
        }

        /**
        * Deletes all elements from the ConcurrentHashMap
        */
        public clear(): void {
            if (this.headNode !== undefined) {
                let iterator = this.keys();
                while (true) {
                    const v = iterator.next();
                    if (v.done) {
                        break;
                    }
                    let key = v.value;
                    if (key != undefined) {
                        this.delete(key as K);
                    }
                }
                ConcurrencyHelpers.mutexLock(this.bucketsMutex);
                while (this.setVal.get() != 0) {
                    Coroutine.Schedule();
                }
                this.buckets = new ConcurrentHashMapBucket<K, V>[ConcurrentHashMap.MIN_BUCKETS_COUNT];
                ConcurrencyHelpers.mutexLock(this.recreateMutex);
                this.bucketMutex = new (Object | undefined)[ConcurrentHashMap.MIN_BUCKETS_COUNT];
                ConcurrencyHelpers.mutexUnlock(this.recreateMutex);
                ConcurrencyHelpers.mutexUnlock(this.bucketsMutex);
                ConcurrencyHelpers.mutexLock(this.mutex);
                this.headNode = undefined;
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
            this.sizeVal.set(0);
        }

        /**
        * Returns a value assosiated with key if present
        *
        * @param { K } key the key to find in the ConcurrentHashMap
        *
        * @returns { V | undefined } value if assosiated with key presents.
        */
        public override get(key: K): V | undefined {
            const buck = this.getBucket(key);
            if (buck !== undefined) {
                const buckSize = buck.length;
                ConcurrencyHelpers.mutexLock(this.bucketsMutex);
                const buckIndex = this.getBucketIndex(key);
                ConcurrencyHelpers.mutexUnlock(this.bucketsMutex);
                for (let i = 0; i < buckSize; i++) {
                    const node = this.atomicBucketAt(buckIndex, buck, i);
                    if (node === null) {
                        continue;
                    }
                    if (ConcurrentHashMap.sameValueZero(node.key, key)) {
                        return node.val;
                    }
                }
            }

            return undefined;
        }

        /**
        * Checks if ConcurrentHashMap is empty
        *
        * @returns { boolean } true if ConcurrentHashMap is empty, false otherwise.
        */
        public isEmpty(): boolean {
            if (this.sizeVal.get() == 0) {
                return true;
            }
            return false;
        }

        protected mappedIterator<R>(fn: (e: MapNode<K, V>) => R): IterableIterator<R> {
            ConcurrencyHelpers.mutexLock(this.mutex);
            try {
                const headNode = this.headNode;
                if (headNode !== undefined) {
                    return new ConcurrentHashMapIterator<K, V, R>(this.mutex, headNode, headNode, fn);
                } else {
                    return new EmptyConcurrentHashMapIterator<R>();
                }
            } finally {
                ConcurrencyHelpers.mutexUnlock(this.mutex);
            }
        }

        /**
        * Returns elements from the ConcurrentHashMap as an keys Iterator
        *
        * @returns { IterableIterator<K> } iterator with ConcurrentHashMap keys
        */
        public override keys(): IterableIterator<K> {
            return this.mappedIterator<K>((e: MapNode<K, V>): K => e.key);
        }

        /**
        * Returns elements from the ConcurrentHashMap as an values Iterator
        *
        * @returns { IterableIterator<K> } iterator with ConcurrentHashMap values
        */
        public override values(): IterableIterator<V> {
            return this.mappedIterator<V>((e: MapNode<K, V>): V => e.val );
        }

        /**
        * Iterable interface implementation
        *
        * @returns { IterableIterator<[K, V]> } iterator over all elements
        */
        public override $_iterator(): IterableIterator<[K, V]> {
            return this.entries();
        }

        /**
        * Returns elements from the ConcurrentHashMap as an array of Entries
        *
        * @returns { IterableIterator<[K, V]> } an array of Entries
        */
        public override entries(): IterableIterator<[K, V]> {
            return this.mappedIterator<[K, V]>((e: MapNode<K, V>): [K, V] => [e.key, e.val]);
        }

        /**
        * Executes a provided function once per each key/value pair in the ConcurrentHashMap, in insertion order
        *
        * @param callbackfn to apply
        */
        public override forEach(callbackfn: (value: V, key: K, map: ConcurrentHashMap<K, V>) => void): void {
            const entriesIter = this.mappedIterator<MapNode<K, V>>((e: MapNode<K, V>): MapNode<K, V> => e);
            iteratorForEach<MapNode<K, V>>(entriesIter, (e: MapNode<K, V>): void => {
                callbackfn(e.val, e.key, this);
            });
        }
    }
}
