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

package std.containers

export namespace containers {
    
    type UndefinableObject = Object | undefined

    export class UndefinableObjectArray {
        /* const */ static ArrayInitSize : int = 16

        private init(capacity: int, val: UndefinableObject): void {
            this.data = new FixedArray<UndefinableObject>(capacity)
            for (let i = 0; i < this.data.length; ++i) {
                this.data[i] = val
            }
            this.curSize = capacity
        }

        /**
        * Constructs new UndefinableObjectArray with respect to capacity and initial value
        *
        * @param capacity
        *
        * @param val
        */
        constructor(capacity: int, val: UndefinableObject) {
            this.init(capacity, val)
        }

        /**
        * Constructs new UndefinableObjectArray with required capacity
        *
        * @param capacity
        */
        constructor(capacity: int) {
            this.data = new FixedArray<UndefinableObject>(capacity)
            this.curSize = 0
        }

        /**
        * Constructs new empty UndefinableObjectArray
        */
        constructor() {
            this(UndefinableObjectArray.ArrayInitSize)
        }

        /**
        * Increases capacity if passed argument is greater than current capacity
        *
        * @param capacity
        */
        public reserve(capacity: int): void {
            if (this.data.length < capacity) {
                let newData : FixedArray<UndefinableObject> = new FixedArray<UndefinableObject>(capacity)
                for (let i = 0; i < this.curSize; ++i) {
                newData[i] = this.data[i]
                }
                this.data = newData
            }
        }

        /**
        * Gets the underlying array
        *
        * @returns the underlying array
        */
        public toArray(): Object[] {
            let newData = new (Object)[this.curSize]
            for (let i = 0; i < this.curSize; ++i) {
                newData[i] = this.data[i]!
            }
            return newData
        }

        private static getNewCapacity(currentCapacity: int): int {
            const fastGrowThreshold = 8192
            const multiplier = 2
            if (currentCapacity < fastGrowThreshold) {
                // Adding 4 to jump over 0
                return (currentCapacity + 4) * multiplier * 2
            } else {
                return currentCapacity * multiplier
            }
        }

        /**
        * Pushes a value to the end of the array
        *
        * @param e value to push
        */
        public pushBack(e: UndefinableObject): void {
            if (this.curSize == this.data.length) {
                let newData : FixedArray<UndefinableObject> = new FixedArray<UndefinableObject>(UndefinableObjectArray.getNewCapacity(this.data.length))
                for (let i = 0; i < this.curSize; ++i) {
                    newData[i] = this.data[i]
                }
                this.data = newData
            }
            this.data[this.curSize] = e
            ++this.curSize
        }

        /**
        * Pops a value from the end of the List
        *
        * @returns popped value
        */
        public popBack(): UndefinableObject {
            if (this.curSize === 0) {
                throw new AssertionError("No data to popBack from UndefinableObjectArray!")
            }
            --this.curSize
            return this.data[this.curSize]
        }

        /**
        * Returns number of elements in the List
        */
        public size(): int {
            return this.curSize
        }

        /**
        * Returns an element at the specified index
        *
        * @param index element position
        *
        * @returns an element
        */
        public at(index: int): UndefinableObject {
            return this.data[index]
        }

        /**
        * Sets an element at the specified index
        *
        * @param index element position
        *
        * @param e new value
        */
        public set(index: int, e: UndefinableObject): void {
            this.data[index] = e
        }

        private data: FixedArray<UndefinableObject> = []
        private curSize: int
    }
}
