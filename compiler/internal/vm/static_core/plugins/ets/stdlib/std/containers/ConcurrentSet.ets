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
    * ConcurrentSet implementation
    */
    export final class ConcurrentSet<T> implements ReadonlySet<T> {
        private readonly elements: ConcurrentHashMap<T, T>;

        /**
        * Construct a ConcurrentHashMap without parameters.
        */
        constructor(){
            this.elements = new ConcurrentHashMap<T, T>();
        }

        /**
        * Constructs a ConcurrentSet from ArrayLike
        * @param { ArrayLike<T> } values initial ArrayLike
        */
        constructor(values: ArrayLike<T> ) {
            this.elements = new ConcurrentHashMap<T, T>();
            for (let i = 0; i < values.length; ++i) {
                this.elements.set(values[i], values[i]);
            }
        }

        /**
        * Constructs a ConcurrentSet from iterator
        * @param { Iterable<T> } iterable initial iterable
        */
        constructor(iterable: Iterable<T>) {
            const it = (iterable as Iterable<T>).$_iterator();
            const entriesIter = new MappingIterator<T, [T, T]>(it, (element: T): [T, T] => [element, element]);
            this.elements = new ConcurrentHashMap<T, T>(entriesIter);
        }

        /**
        * Constructs a ConcurrentSet from a Set
        * @param { Set<T> } set another Set
        */
        constructor(set: Set<T>) {
            this.elements = new ConcurrentHashMap<T, T>();
            set.forEach((value: T) => {
                this.add(value);
            });
        }

        private toStringVals(): String {
            const strBuf = new StringBuilder();

            const valsIter = this.values();
            let valsIterRes = valsIter.next();
            while (!valsIterRes.done) {
                strBuf.append(`${valsIterRes.value}`);

                valsIterRes = valsIter.next();
                if (!valsIterRes.done) {
                    strBuf.append(",");
                }
            }

            return strBuf.toString();
        }

        /**
        * Creates a string representation of the CoucurrentSet
        *
        * @returns { String } string representation
        */
        public override toString(): String {
            return "ConcurrentSet[" + this.toStringVals() + "]";
        }

        /**
        * Puts a value into the ConcurrentSet
        *
        * @param { T } val the value to put into the ConcurrentSet
        * @returns { this } this
        */
        public add(val: T): this {
            this.elements.set(val, val);
            return this;
        }

        /**
        * Checks if a value is in the ConcurrentSet.
        *
        * @param { T } val the value to find in the ConcurrentSet.
        *
        * @returns { boolean } true if the value is in the ConcurrentSet.
        */
        public override has(val: T): boolean {
            return this.elements.has(val);
        }

        /**
        * Returns number of unique elements in the ConcurrentSet.
        *
        * @returns { number } number of unique elements in the ConcurrentSet.
        */
        public override get size(): int {
            return this.elements.size;
        }

        /**
        * Removes a value from the ConcurrentSet.
        *
        * @param { T } val the value to remove
        *
        * @returns { boolean } true if the delete operation is success, otherwise false.
        */
        public delete(val: T): boolean {
            return this.elements.delete(val);
        }

        /**
        * Deletes all elements from the ConcurrentSet.
        */
        public clear(): void {
            this.elements.clear();
        }

        /**
        * Despite name, returns elements from the ConcurrentSet
        *
        * @returns { IterableIterator<T> } an iterable of the values in the ConcurrentSet
        */
        public override keys(): IterableIterator<T> {
            return this.elements.keys();
        }

        /**
        * Returns elements from the ConcurrentSet
        *
        * @returns { IterableIterator<T> } an iterable of the values in the ConcurrentSet
        */
        public override values(): IterableIterator<T> {
            return this.elements.keys();
        }

        /**
        * Iterable interface implementation
        *
        * @returns { IterableIterator<T> } iterator over all elements
        */
        public override $_iterator(): IterableIterator<T> {
            return this.values();
        }

        /**
        * @returns { IterableIterator<[T, T]> } an iterable of [v,v] pairs for every value `v` in the ConcurrentSet.
        */
        public override entries(): IterableIterator<[T, T]> {
            return this.elements.entries();
        }

        /**
        * Executes a provided function once per each value in the ConcurrentSet object, in insertion order
        *
        * @param callbackfn to apply
        */
        public override forEach(callbackfn: (value: T, value2: T, set: ConcurrentSet<T>) => void): void {
            iteratorForEach<T>(this.keys(), (x: T): void => callbackfn(x, x, this));
        }
    }

    class MappingIterator<I, O> implements IterableIterator<O> {
        private readonly inputIter: Iterator<I>;
        private readonly mappingFn: (input: I) => O;

        constructor(inputIter: Iterator<I>, mappingFn: (input: I) => O) {
            this.inputIter = inputIter;
            this.mappingFn = mappingFn;
        }

        override next(): IteratorResult<O> {
            const inputIterRes = this.inputIter.next();

            if (inputIterRes.done) {
                return new IteratorResult<O>();
            } else {
                const output = this.mappingFn(inputIterRes.value as I);
                return new IteratorResult<O>(output);
            }
        }
    }
}
