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

package std.containers;

export namespace containers {

    export class ArrayAsListString implements ListString {

        private init(capacity: int, val: String): void {
            this.data = new FixedArray<String>(capacity);
            for (let i = 0; i < this.data.length; ++i) {
                this.data[i] = val;
            }
            this.curSize = capacity;
        }

        /**
        * Constructs new ArrayAsList with respect to capacity and initial value
        *
        * @param capacity
        *
        * @param val
        */
        constructor(capacity: int, val: String) {
            this.init(capacity, val);
        }

        /**
        * Constructs new empty ArrayAsList
        */
        constructor() {
            this.init(0, new String());
        }

        /**
        * Constructs new ArrayAsList with required capacity
        *
        * @param capacity
        */
        constructor(capacity: int) {
            this.init(capacity, new String());
        }

        /**
        * Increases capacity if passed argument is greater than current capacity
        *
        * @param capacity
        */
        public reserve(capacity: int): void {
            if (this.data.length < capacity) {
                let newData : FixedArray<String> = new FixedArray<String>(capacity);
                for (let i = 0; i < this.curSize; ++i) {
                newData[i] = this.data[i];
                }
                this.data = newData;
            }
        }

        /**
        * Gets a shallow-copy of the underlying array
        *
        * @returns a shallow-copy of the underlying array
        */
        public toArray(): String[] {
            let data = new String[this.curSize];
            for (let i = 0; i < this.curSize; ++i) {
                data[i] = this.data[i];
            }
            return data;
        }

        private static getNewCapacity(currentCapacity: int): int {
            // TODO(ivan-tyulyandin): select proper capacity increasing strategy
            const fastGrowThreshold = 8192;
            const multiplier = 2;
            if (currentCapacity < fastGrowThreshold) {
                // Adding 4 to jump over 0
                return (currentCapacity + 4) * multiplier * 2;
            } else {
                return currentCapacity * multiplier;
            }
        }

        /**
        * Pushes a value to the begin of the List
        *
        * @param e value to push
        */
        public override pushFront(e: String): void {
            let dst = this.data;
            if (this.curSize == this.data.length) {
                dst = new FixedArray<String>(ArrayAsListString.getNewCapacity(this.data.length));
            }
            for (let i = this.curSize; i != 0; --i) {
                dst[i] = this.data[i-1];
            }
            this.data = dst;
            this.data[0] = e;
            ++this.curSize;
        }

        /**
        * Pops a value from the begin of the List
        *
        * @returns popped value
        */
        public override popFront(): String {
            if (this.curSize === 0) {
                throw new AssertionError("No data to popFront from ArrayAsList!")
            }
            let res: String = this.data[0];
            for (let i = 1; i < this.curSize; ++i) {
                this.data[i-1] = this.data[i];
            }
            --this.curSize;
            return res;
        }

        /**
        * Pushes a value to the end of the List
        *
        * @param e value to push
        */
        public override pushBack(e: String): void {
            if (this.curSize == this.data.length) {
                let newData : FixedArray<String> = new FixedArray<String>(ArrayAsListString.getNewCapacity(this.data.length));
                for (let i = 0; i < this.curSize; ++i) {
                    newData[i] = this.data[i];
                }
                this.data = newData;
            }
            this.data[this.curSize] = e;
            ++this.curSize;
        }

        /**
        * Pops a value from the end of the List
        *
        * @returns popped value
        */
        public override popBack(): String {
            if (this.curSize === 0) {
                throw new AssertionError("No data to popBack from ArrayAsList!")
            }
            --this.curSize;
            return this.data[this.curSize];
        }

        /**
        * Returns number of elements in the List
        */
        public override size(): int {
            return this.curSize;
        }

        /**
        * Returns an element at the specified index
        *
        * @param index element position
        *
        * @returns an element
        */
        public override at(index: int): String {
            return this.data[index];
        }

        /**
        * Sets an element at the specified index
        *
        * @param index element position
        *
        * @param e new value
        */
        public override set(index: int, e: String): void {
            this.data[index] = e;
        }

        /**
        * Checks if an element is in the List
        *
        * @param e value to find
        *
        * @returns true if value present, false otherwise
        */
        public override has(e: String): boolean {
            return false;
        }

        private data: FixedArray<String> = [];
        private curSize: int;
    }
}
