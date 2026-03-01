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


import { BusinessError } from "@ohos.base"

const OutOfBoundsErrorCodeId: int = 10200001;
const TypeErrorCodeId: int = 401;

function createBusinessError(code: int, message: string) {
    let err = new BusinessError();
    err.code = code;
    err.name = 'BusinessError';
    err.message = message;
    return err;
}

export namespace collections {
    final class BitVectorIterator implements IterableIterator<int> {
        private index: int = 0;
        private parent: BitVector;

        constructor(parent: BitVector) {
            this.parent = parent;
        }

        override next(): IteratorResult<int> {
            if (this.parent.length === 0 || this.index >= this.parent.length) {
                return new IteratorResult<int>();
            }

            return new IteratorResult<int>(this.parent[this.index++]);
        }

        override $_iterator(): IterableIterator<int> {
            return this;
        }
    }

    final class BitIndex {
        // |0|1|...|0|0|1|1| ... |0|1|0|0|1| ... |0|1|
        //          ↑
        //         index
        // |-  element  -|-  element  -|-  element  -|
        //       ↑
        //    elementId
        // |---------------buffer--------------------|
        elementId: int;
        index: int;

        constructor(elementId: int, index: int) {
            this.elementId = elementId;
            this.index = index
        }
    }
    /**
     * An ordered collections of bit values, which are either 0 or 1.
     * If multiple threads access a BitVector instance concurrently,
     * and at least one of the threads modifies the array structurally,
     * it must be synchronized externally.
     *
     */
    export class BitVector implements Iterable<int> {
        static readonly BIT_SET_LENGTH = 32;    // ArkTS Specification, Release 1.1.0 <Integer Types and Operations>

        private buffer: Array<int>;
        private _length: int = 0;

        /**
        * A constructor used to create a BitVector object.
        *
        * @param { int } length - The length of BitVector object.
        */
        public constructor(length: int) {
            if (length === 0) {
                this.buffer = new Array<int>();
            } else {
                this._length = length;
                this.buffer = Array.create<int>(this.getUsedCapacity(), 0);
            }
        }

        /**
         * Gets the element number of the BitVector. This is a number one higher than the highest index in the bit vector.
         * It can be changed by resize().
         */
        public get length(): int {
            return this._length;
        }

        /**
         * Appends the bit element to the end of this bit vector.
         *
         * @param { int } element - Element to be appended to this bit vector (0 means 0, else means 1).
         * @returns { boolean } The boolean type, returns true if the addition is successful, and returns false if it fails.
         */
        public push(element: int): boolean {
            let index = this.computeElementIdAndBitId(this._length);
            this.checkAndIncrement(index.elementId);

            this.setBitUnsafe(this._length++, element);

            return true;
        }

        /**
         * Retrieves and removes the bit element to the end of this bit vector.
         *
         * @returns { int | undefined } The boolean type, if the bit push successfully, return true, else return false.
         */
        public pop(): int | undefined {
            if (this._length === 0) {
                return undefined;
            }

            return this.getBitUnsafe(--this._length);
        }

        /**
         * Returns the item at that index.
         *
         * @param { int } index - The zero-based index of the desired code unit.
         * @returns { int } The element in the bitVector matching the given index.
         * @throws { BusinessError } 10200001 - If the index is out of range.
         */
        public $_get(index: int): int {
            this.checkIndex(index);

            return this.getBitUnsafe(index);
        }

        /**
         * Sets the value of item at that index.
         *
         * @param { int } index - The zero-based index of the desired code unit.
         * @param { int } value - The value to set at the given index.
         * @throws { BusinessError } 10200001 - If the index is out of range.
         */
        public $_set(index: int, value: int): void {
            this.checkIndex(index);

            this.setBitUnsafe(index, value);
        }
        /**
         * Check if bit vector contains a particular bit element.
         *
         * @param { int } element - Element to be contained (0 means 0, else means 1).
         * @param { int } fromIndex - The starting position of the index, containing the value at that index position.
         * @param { int } toIndex - The end of the index, containing the value at that index.
         * @returns { boolean } The boolean type, if bit vector contains the specified element, return true,
                                 else return false.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of fromIndex or toIndex is out of range.
         */
        public has(element: int, fromIndex: int, toIndex: int): boolean {
            this.checkRange(fromIndex, toIndex);

            let fromElem = this.computeElementIdAndBitId(fromIndex);
            let toElem =  this.computeElementIdAndBitId(toIndex);

            if (fromElem.elementId === toElem.elementId) {
                return this.checkBit(this.buffer[fromElem.elementId], fromElem.index, toElem.index - 1, element);
            }

            if (this.checkBit(this.buffer[fromElem.elementId], fromElem.index, BitVector.BIT_SET_LENGTH - 1, element)) {
                return true;
            }

            fromElem.elementId++;
            while (fromElem.elementId < toElem.elementId) {
                if (this.checkBit(this.buffer[fromElem.elementId], 0, BitVector.BIT_SET_LENGTH - 1, element)) {
                    return true;
                }
                fromElem.elementId++;
            }

            if (toElem.index > 0) {
                return this.checkBit(this.buffer[toElem.elementId], 0, toElem.index - 1, element);
            }

            return false;
        }

        /**
         * Returns an iterator that iterates over bit vector.
         *
         * @returns { IterableIterator<int> } A new iterable iterator object.
         */
        public override $_iterator(): IterableIterator<int> {
            return new BitVectorIterator(this);
        }

        /**
         * Returns an iterable of values in the bit vector
         *
         * @returns { IterableIterator<int> }  A new iterable iterator object.
         * @throws { BusinessError } 10200011 - The values method cannot be bound.
         * @throws { BusinessError } 10200201 - Concurrent modification error.
         * @syscap SystemCapability.Utils.Lang
         * @crossplatform
         * @atomicservice
         * @since 20
         * @arkts 1.2
         */
        public values(): IterableIterator<int> {
            return this.$_iterator();
        }

        /**
         * Flips the bit value by index in a bit vector.(Flip 0 to 1, flip 1 to 0)
         *
         * @param { int } index - The index in the bit vector.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of index is out of range.
         */
        public flipBitByIndex(index: int): void {
            this.checkIndex(index);

            let bitIndex = this.computeElementIdAndBitId(index);
            this.buffer[bitIndex.elementId] = this.flipBits(this.buffer[bitIndex.elementId], bitIndex.index, bitIndex.index);
        }

        /**
         * Flips a range of bit values in a bit vector.(Flip 0 to 1, flip 1 to 0).
         *
         * @param { int } fromIndex - The starting position of the index, containing the value at that index position.
         * @param { int } toIndex - The end of the index, excluding the value at that index.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of fromIndex or toIndex is out of range.
         */
        public flipBitsByRange(fromIndex: int, toIndex: int): void {
            this.checkRange(fromIndex, toIndex);

            let fromElem = this.computeElementIdAndBitId(fromIndex);
            let toElem =  this.computeElementIdAndBitId(toIndex);

            if (fromElem.elementId === toElem.elementId) {
                this.buffer[fromElem.elementId] = this.flipBits(this.buffer[fromElem.elementId], fromElem.index, toElem.index - 1);
                return;
            }

            this.buffer[fromElem.elementId] = this.flipBits(this.buffer[fromElem.elementId], fromElem.index, BitVector.BIT_SET_LENGTH - 1);

            ++fromElem.elementId;
            while (fromElem.elementId < toElem.elementId) {
                this.buffer[fromElem.elementId] = this.flipBits(this.buffer[fromElem.elementId], 0, BitVector.BIT_SET_LENGTH - 1);
                ++fromElem.elementId;
            }

            if (toElem.index > 0) {
                this.buffer[toElem.elementId] = this.flipBits(this.buffer[toElem.elementId], 0, toElem.index - 1);
            }
        }

        /**
         * Returns the bit values in a range of indices in a bit vector.
         *
         * @param { int } fromIndex - The starting position of the index, containing the value at that index position.
         * @param { int } toIndex - The end of the index, excluding the value at that index.
         * @returns { BitVector } The BitVector type, returns the bit values in a range of indices in a bit vector.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of fromIndex or toIndex is out of range.
         */
        public getBitsByRange(fromIndex: int, toIndex: int): BitVector {
            this.checkRange(fromIndex, toIndex);

            let fromElem = this.computeElementIdAndBitId(fromIndex);
            let toElem =  this.computeElementIdAndBitId(toIndex);

            let newBitVector = new BitVector(toIndex - fromIndex);

            if (fromElem.elementId === toElem.elementId) {
                newBitVector.buffer[0] = this.getBits(this.buffer[fromElem.elementId], fromElem.index, toElem.index - 1);
                return newBitVector;
            }

            for (let bit of this) {
                newBitVector.push(bit);
            }

            return newBitVector;
        }

        /**
         * Sets a range of bits in a bit vector to a particular element.
         *
         * @param { int } element - Element to be set (0 means 0, else means 1).
         * @param { int } fromIndex - The starting position of the index, containing the value at that index position.
         * @param { int } toIndex - The end of the index, excluding the value at that index.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of fromIndex or toIndex is out of range.
         */
        public setBitsByRange(element: int, fromIndex: int, toIndex: int): void {
            this.checkRange(fromIndex, toIndex);

            let fromElem = this.computeElementIdAndBitId(fromIndex);
            let toElem =  this.computeElementIdAndBitId(toIndex);

            if (fromElem.elementId === toElem.elementId) {
                this.buffer[fromElem.elementId] = this.setBits(this.buffer[fromElem.elementId], fromElem.index, toElem.index - 1, element);
                return;
            }

            this.buffer[fromElem.elementId] = this.setBits(this.buffer[fromElem.elementId], fromElem.index, BitVector.BIT_SET_LENGTH - 1, element);

            ++fromElem.elementId;
            while (fromElem.elementId < toElem.elementId) {
                this.buffer[fromElem.elementId] = this.setBits(this.buffer[fromElem.elementId], 0, BitVector.BIT_SET_LENGTH - 1, element);
                ++fromElem.elementId;
            }

            if (toElem.index != 0) {
                this.buffer[fromElem.elementId] = this.setBits(this.buffer[toElem.elementId], 0, toElem.index - 1, element);
            }
        }

        /**
         * Sets all of bits in a bit vector to a particular element.
         *
         * @param { int } element - Element to be set (0 means 0, else means 1).
         */
        public setAllBits(element: int): void {
            if (element === 0) {
                for (let i = 0; i < this.getUsedCapacity(); i++) {
                    this.buffer[i] = 0;
                }
            } else {
                for (let i = 0; i < this.getUsedCapacity(); i++) {
                    this.buffer[i] = ~0x0;
                }
            }
        }

        /**
         * Resize the bitVector's length.
         *
         * @param { int } size - The new size for bitVector. If count is greater than the current size of bitVector,
         * the additional bit elements are set to 0.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                            1.Mandatory parameters are left unspecified.
         *                            2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of size is out of range.
         */
        public resize(size: int): void {
            if (size < 0) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of \"length\" is out of range. It must be >= 0. Received value is: ${size}`);
            }

            let sizeInt: int = size;
            if (this._length >= sizeInt) {
                this._length = sizeInt;
                return;
            }

            let after = this.computeElementIdAndBitId(sizeInt);
            let before = this.computeElementIdAndBitId(this.length);

            this.checkAndIncrement(before.elementId);
            this.buffer[before.elementId] = this.setBits(this.buffer[before.elementId], before.index, BitVector.BIT_SET_LENGTH - 1, 0);

            if (before.elementId != after.elementId) {
                ++before.elementId;
                while (before.elementId <= after.elementId) {
                    this.buffer.push(0);
                    ++before.elementId;
                }
            }
            this._length = sizeInt;
        }

        /**
         * Counts the number of times a certain bit element occurs within a range of bits in a bit vector.
         *
         * @param { int } element - Element to be counted (0 means 0, else means 1).
         * @param { int } fromIndex - The starting position of the index, containing the value at that index position.
         * @param { int } toIndex - The end of the index, excluding the value at that index.
         * @returns { int } The number type, return the number of times a certain bit element
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of fromIndex or toIndex is out of range.
         */
        public getBitCountByRange(element: int, fromIndex: int, toIndex: int): int {
            this.checkRange(fromIndex, toIndex);

            let fromElem = this.computeElementIdAndBitId(fromIndex);
            let toElem =  this.computeElementIdAndBitId(toIndex);

            if (fromElem.elementId === toElem.elementId) {
                let count = this.countBit1InRange(this.buffer[fromElem.elementId], fromElem.index, toElem.index - 1);
                return element === 0 ? (toIndex - fromIndex - count) : count;
            }

            let count = this.countBit1InRange(this.buffer[fromElem.elementId], fromElem.index, BitVector.BIT_SET_LENGTH - 1);

            ++fromElem.elementId;
            while (fromElem.elementId < toElem.elementId) {
                count += this.countBit1InRange(this.buffer[fromElem.elementId], 0, BitVector.BIT_SET_LENGTH - 1);
                ++fromElem.elementId;
            }

            if (toElem.index != 0) {
                count += this.countBit1InRange(this.buffer[toElem.elementId], 0, toElem.index - 1);
            }

            return element === 0 ? (toIndex - fromIndex - count) : count;
        }

        /**
         * Locates the first occurrence of a certain bit value within a range of bits in a bit vector.
         *
         * @param { int } element - Element to be Located (0 means 0, else means 1).
         * @param { int } fromIndex - The starting position of the index, containing the value at that index position.
         * @param { int } toIndex - The end of the index, excluding the value at that index.
         * @returns { int } The number type, return the first index of specified bit within a range,
         * or -1 if this range of the bitVector does not contain the element.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of fromIndex or toIndex is out of range.
         */
        public getIndexOf(element: int, fromIndex: int, toIndex: int): int {
            this.checkRange(fromIndex, toIndex);

            let fromElem = this.computeElementIdAndBitId(fromIndex);
            let toElem =  this.computeElementIdAndBitId(toIndex);

            if (fromElem.elementId === toElem.elementId) {
                let bitIndex = this.findSignificantBit(this.buffer[fromElem.elementId], fromElem.index, toElem.index - 1, element);
                return bitIndex != -1 ? this.computeIndex(fromElem.elementId, bitIndex) : -1;
            }

            let bitIndex = this.findSignificantBit(this.buffer[fromElem.elementId], fromElem.index, BitVector.BIT_SET_LENGTH - 1, element);
            if (bitIndex != -1) {
                return this.computeIndex(fromElem.elementId, bitIndex)
            }

            ++fromElem.elementId;
            while (fromElem.elementId < toElem.elementId) {
                bitIndex = this.findSignificantBit(this.buffer[fromElem.elementId], 0, BitVector.BIT_SET_LENGTH - 1, element);
                if (bitIndex != -1) {
                    return this.computeIndex(fromElem.elementId, bitIndex);
                }

                ++fromElem.elementId;
            }

            if (toElem.index > 0) {
                bitIndex = this.findSignificantBit(this.buffer[toElem.elementId], 0, toElem.index - 1, element);
                if (bitIndex != -1) {
                    return this.computeIndex(toElem.elementId, bitIndex);
                }
            }

            return -1;
        }

        /**
         * Locates the last occurrence of a certain bit value within a range of bits in a bit vector.
         *
         * @param { int } element - Element to be Located (0 means 0, else means 1).
         * @param { int } fromIndex - The starting position of the index, containing the value at that index position.
         * @param { int } toIndex - The end of the index, excluding the value at that index.
         * @returns { int } The number type, return the last index of specified bit within a range,
         * or -1 if this range of the bitVector does not contain the element.
         * @throws { BusinessError } 401 - Parameter error. Possible causes:
         *                                    1.Mandatory parameters are left unspecified.
         *                                    2.Incorrect parameter types.
         * @throws { BusinessError } 10200001 - The value of fromIndex or toIndex is out of range.
         */
        public getLastIndexOf(element: int, fromIndex: int, toIndex: int): int {
            this.checkRange(fromIndex, toIndex);

            let fromElem = this.computeElementIdAndBitId(fromIndex);
            let toElem =  this.computeElementIdAndBitId(toIndex);

            if (fromElem.elementId === toElem.elementId) {
                let bitIndex = this.findSignificantBit(this.buffer[fromElem.elementId], fromElem.index, toElem.index - 1, element, false);
                return bitIndex != -1 ? this.computeIndex(fromElem.elementId, bitIndex) : -1;
            }

            if (toElem.index > 0) {
                let bitIndex = this.findSignificantBit(this.buffer[toElem.elementId], 0, toElem.index - 1, element, false);
                if (bitIndex != -1) {
                    return this.computeIndex(toElem.elementId, bitIndex);
                }
            }

            --toElem.elementId;
            while (fromElem.elementId < toElem.elementId) {
                let bitIndex = this.findSignificantBit(this.buffer[toElem.elementId], 0, BitVector.BIT_SET_LENGTH - 1, element, false);
                if (bitIndex != -1) {
                    return this.computeIndex(toElem.elementId, bitIndex);
                }

                --toElem.elementId;
            }

            let bitIndex = this.findSignificantBit(this.buffer[fromElem.elementId], fromElem.index, BitVector.BIT_SET_LENGTH - 1, element, false);
            if (bitIndex != -1) {
                return this.computeIndex(fromElem.elementId, bitIndex);
            }

            return -1;
        }

        private checkAndIncrement(index: int): void {
            if (this.buffer.length === index) {
                this.buffer.push(0);
            }
        }

        private checkRange(fromIndex: int, toIndex: int): void {
            if (toIndex < 0 || toIndex > this._length) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of \"toIndex\" is out of range. It must be >= 0 && <= ${this._length}. Received value is: ${toIndex}`);
            }

            if (fromIndex < 0 || fromIndex >= toIndex) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of \"fromIndex\" is out of range. It must be >= 0 && <= ${toIndex - 1}. Received value is: ${fromIndex}`);
            }
        }

        private checkIndex(index: int): void {
            if (index < 0 || index >= this._length) {
                throw createBusinessError(OutOfBoundsErrorCodeId, `The value of \"index\" is out of range. It must be >= 0 && <= ${this._length - 1}. Received value is: ${index}`);
            }
        }

        private getUsedCapacity(): int {
            return Math.ceil(this._length.toDouble() / BitVector.BIT_SET_LENGTH).toInt();
        }

        private computeElementIdAndBitId(index: int): BitIndex {
            let elementId = Math.floor(index / BitVector.BIT_SET_LENGTH).toInt();
            let bitId = index % BitVector.BIT_SET_LENGTH;
            return new BitIndex(elementId, bitId);
        }

        private computeIndex(elementId: int, bitId: int): int {
            return elementId * BitVector.BIT_SET_LENGTH + bitId;
        }

        private getBitUnsafe(index: int): int {
            let bitIndex = this.computeElementIdAndBitId(index);
            let value = (this.buffer[bitIndex.elementId] >> bitIndex.index) & 1;

            return value;
        }

        private setBitUnsafe(index: int, value: int): void {
            let bitIndex = this.computeElementIdAndBitId(index);
            if (value === 0) {
                let mask = ~(1 << bitIndex.index);
                this.buffer[bitIndex.elementId] = (this.buffer[bitIndex.elementId] & mask);
            } else {
                this.buffer[bitIndex.elementId] = (this.buffer[bitIndex.elementId] | (1 << bitIndex.index));
            }
        }

        private checkBit(element: int, fromIndex: int, toIndex: int, target_bit: int): boolean {
            let length = toIndex - fromIndex + 1;
            let mask: int = this.getRangeMask(fromIndex, length);

            if (target_bit !== 0) {
                return (element & mask) != 0;
            } else {
                return (element & mask) != mask;
            }
        }

        private flipBits(num: int, fromIndex: int, toIndex: int): int {
            let length = toIndex - fromIndex + 1;
            let mask = this.getRangeMask(fromIndex, length);

            return num ^ mask;
        }

        private getRangeMask(fromIndex: int, length: int): int {
            let mask: int = 0x0;

            if (length == BitVector.BIT_SET_LENGTH) {
                mask = ~mask;
            } else {
                mask = ((1 << length) - 1) << fromIndex;
            }

            return mask;
        }

        private getBits(num: int, fromIndex: int, toIndex: int): int {
            let length = toIndex - fromIndex + 1;
            let mask: int = this.getRangeMask(fromIndex, length);
            return (num & mask) >> fromIndex;
        }

        private setBits(num: int, fromIndex: int, toIndex: int, element: int): int {
            let length = toIndex - fromIndex + 1;
            let mask: int = this.getRangeMask(fromIndex, length);
            if (element != 0) {
                return num | mask;
            } else {
                return num & ~mask;
            }
        }

        private countBit1InRange(num: int, fromIndex: int, toIndex: int): int {
            let length = toIndex - fromIndex + 1;
            let mask: int = this.getRangeMask(fromIndex, length);

            let masked_value = num & mask;

            let count = 0;
            while (masked_value != 0) {
                masked_value = masked_value & (masked_value - 1);
                count++;
            }

            return count;
        }

        private findLSB(num: int): int {
            if (num === 0) {
                return -1;
            }

            // Get the lowest 1 bit
            let isolatedBit: int = num & -num;

            let pos = 0;
            // Check upper 16 bits; if set, add 16 and shift right
            if (isolatedBit & 0xFFFF0000) { pos += 16; isolatedBit >>= 16; }
            // Check next 8 bits; if set, add 8 and shift right
            if (isolatedBit & 0x0000FF00) { pos += 8; isolatedBit >>= 8; }
            // Check next 4 bits; if set, add 4 and shift right
            if (isolatedBit & 0x000000F0) { pos += 4; isolatedBit >>= 4; }
            // Check next 2 bits; if set, add 2 and shift right
            if (isolatedBit & 0x0000000C) { pos += 2; isolatedBit >>= 2; }
            // Check bit 1; if set, add 1
            if (isolatedBit & 0x00000002) { pos += 1; }

            return pos;
        }

        private findMSB(num: int): int {
            if (num === 0) {
                return -1;
            }

            let pos = 0;
            // Check upper 16 bits; if set, add 16 and shift right
            if (num & 0xFFFF0000) { pos += 16; num >>= 16; }
            // Check next 8 bits; if set, add 8 and shift right
            if (num & 0xFF00) { pos += 8; num >>= 8; }
            // Check next 4 bits; if set, add 4 and shift right
            if (num & 0xF0) { pos += 4; num >>= 4; }
            // Check next 2 bits; if set, add 2 and shift right
            if (num & 0xC) { pos += 2; num >>= 2; }
            // Check bit 1; if set, add 1
            if (num & 0x2) { pos += 1; }

            return pos;
        }

        private findSignificantBit(element: int, fromIndex: int, toIndex: int, target: int, isLSB: boolean = true): int {
            let length = toIndex - fromIndex + 1;
            let mask: int = this.getRangeMask(fromIndex, length);

            let masked_value: int = 0;
            if (target != 0) {
                masked_value = element & mask;
            } else {
                masked_value = (~element) & mask;
            }

            return isLSB ? this.findLSB(masked_value) : fromIndex + this.findMSB(masked_value >> fromIndex);
        }
    }
}
