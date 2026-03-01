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

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

let longSize: int = 100;
let shortSize: int = 10;

let smallFromBigInt64Array: BigInt64Array = new BigInt64Array(shortSize);
let bigFromBigInt64Array: BigInt64Array = new BigInt64Array(longSize);
let smallFromBigUint64Array: BigUint64Array = new BigUint64Array(shortSize);
let bigFromBigUint64Array: BigUint64Array = new BigUint64Array(longSize);
let smallFromFloat32Array: Float32Array = new Float32Array(shortSize);
let bigFromFloat32Array: Float32Array = new Float32Array(longSize);
let smallFromFloat64Array: Float64Array = new Float64Array(shortSize);
let bigFromFloat64Array: Float64Array = new Float64Array(longSize);
let smallFromInt16Array: Int16Array = new Int16Array(shortSize);
let bigFromInt16Array: Int16Array = new Int16Array(longSize);
let smallFromInt32Array: Int32Array = new Int32Array(shortSize);
let bigFromInt32Array: Int32Array = new Int32Array(longSize);
let smallFromInt8Array: Int8Array = new Int8Array(shortSize);
let bigFromInt8Array: Int8Array = new Int8Array(longSize);
let smallFromSet: Set<number> = new Set<number>();
let bigFromSet: Set<number> = new Set<number>();

let smallFromUint16Array: Uint16Array = new Uint16Array(shortSize);
let bigFromUint16Array: Uint16Array = new Uint16Array(longSize);
let smallFromUint32Array: Uint32Array = new Uint32Array(shortSize);
let bigFromUint32Array: Uint32Array = new Uint32Array(longSize);
let smallFromUint8Array: Uint8Array = new Uint8Array(shortSize);
let bigFromUint8Array: Uint8Array = new Uint8Array(longSize);
let smallFromUint8ClampedArray: Uint8ClampedArray = new Uint8ClampedArray(shortSize);
let bigFromUint8ClampedArray: Uint8ClampedArray = new Uint8ClampedArray(longSize);

class ArrayLikeImplTest<T> implements ArrayLike<T> {
    private buffer: Any[]
    protected actualLength: int

    override $_get(index: int): T {
        if ((index  < 0) || (index >= this.actualLength)) {
            throw new RangeError("Out of bounds");
        }
        return this.buffer[index] as T;
    }

    get length(): int {
        return this.actualLength;
    }

    public push(val: T): number {
        this.ensureUnusedCapacity(1);
        this.buffer[this.actualLength] = val as Any;
        this.actualLength += 1;
        return this.actualLength;
    }

    private ensureUnusedCapacity(cap: int): void {
        if (this.actualLength + cap > this.buffer.length) {
            const copy = new Any[this.buffer.length * 2 + cap];
            for (let i = 0; i < this.actualLength; i++) {
                copy[i] = this.buffer[i];
            }
            this.buffer = copy;
        }
    }

    public constructor() {
        this.buffer = new Any[4]
        this.actualLength = 0
    }
}

class ArrayValueIteratorTest implements IterableIterator<Number> {
    private idx: int
    private val: int

    constructor() {
      this.idx = 0
      this.val = 1
    }

    override $_iterator(): IterableIterator<Number> {
      return this;
    }

    override next(): IteratorResult<Number> {
      if (this.idx < 0 || this.idx >= 1) {
        return new IteratorResult<Number>()
      }
      this.idx++
      return new IteratorResult<Number>(this.val);
    }
}

function check_array<T>(arr: Array<T>, start: int, end: int, expected: T) {
    for (let i = start; i < end; i++) {
        arktest.assertEQ(arr[i], expected);
    }
}

//! CHECKER       std.core Array from primitive array
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, entry: "escompat_array_from.ETSGLOBAL::test_array_from"
//! METHOD        "escompat_array_from.ETSGLOBAL::test_array_from"
//! PASS_AFTER    "Codegen"
//! INST          /CallStatic.*std.core.Array::from/
function test_array_from() {
    {
        let array1 = Array.from(smallFromBigInt64Array);
        check_array<BigInt>(array1, 0, shortSize, new BigInt(0));
    }
    {
        let array1 = Array.from(bigFromBigInt64Array);
        check_array<BigInt>(array1, 0, longSize, new BigInt(0));
    }
    {
        let array1 = Array.from(smallFromBigUint64Array);
        check_array<BigInt>(array1, 0, shortSize, new BigInt(0));
    }
    {
        let array1 = Array.from(bigFromBigUint64Array);
        check_array<BigInt>(array1, 0, longSize, new BigInt(0));
    }
    {
        let array1 = Array.from(smallFromFloat32Array);
        check_array<Double>(array1, 0, shortSize, 0);
    }
    {
        let array1 = Array.from(bigFromFloat32Array);
        check_array<Double>(array1, 0, longSize, 0);
    }
    {
        let array1 = Array.from(smallFromFloat64Array);
        check_array<Double>(array1, 0, shortSize, 0);
    }
    {
        let array1 = Array.from(bigFromFloat64Array);
        check_array<Double>(array1, 0, longSize, 0);
    }
    {
        let array1 = Array.from(smallFromInt16Array);
        check_array<Double>(array1, 0, shortSize, 0);
    }
    {
        let array1 = Array.from(bigFromInt16Array);
        check_array<Double>(array1, 0, longSize, 0);
    }
    {
        let array1 = Array.from(smallFromInt32Array);
        check_array<Double>(array1, 0, shortSize, 0);
    }
    {
        let array1 = Array.from(bigFromInt32Array);
        check_array<Double>(array1, 0, longSize, 0);
    }
    {
        let array1 = Array.from(smallFromInt8Array);
        check_array<Double>(array1, 0, shortSize, 0);
    }
    {
        let array1 = Array.from(bigFromInt8Array);
        check_array<Double>(array1, 0, longSize, 0);
    }
    {
        let array1 = Array.from(bigFromUint32Array);
        check_array<Double>(array1, 0, longSize, 0);
    }

    {
        let array1 = Array.from(smallFromUint16Array);
        check_array<Double>(array1, 0, shortSize, 0);
    }
    {
        let array1 = Array.from(bigFromUint16Array);
        check_array<Double>(array1, 0, longSize, 0);
    }
    {
        let array1 = Array.from(smallFromUint32Array);
        check_array<Double>(array1, 0, shortSize, 0);
    }
    {
        let array1 = Array.from(bigFromUint32Array);
        check_array<Double>(array1, 0, longSize, 0);
    }
    {
        let array1 = Array.from(smallFromUint8Array);
        check_array<Double>(array1, 0, shortSize, 0);
    }
    {
        let array1 = Array.from(bigFromUint8Array);
        check_array<Double>(array1, 0, longSize, 0);
    }
    {
        let array1 = Array.from(smallFromUint8ClampedArray);
        check_array<Double>(array1, 0, shortSize, 0);
    }
    {
        let array1 = Array.from(bigFromUint8ClampedArray);
        check_array<Double>(array1, 0, longSize, 0);
    }
    for (let i: int = 0; i < shortSize; i++) {
        smallFromSet.add(i);
    }
    for (let i: int = 0; i < longSize; i++) {
        bigFromSet.add(i);
    }
    let array1 = Array.from(smallFromSet);
    let array2 = Array.from(bigFromSet);
    for (let i: int = 0; i < shortSize; i++) {
        arktest.assertEQ(array1[i], i);
    }
    for (let i: int = 0; i < longSize; i++) {
        arktest.assertEQ(array2[i], i);
    }

    let arrayUnion: ArrayLike<number> | Iterable<number> = new Array<number>(1, 2, 3);
    let fromUnion = Array.from(arrayUnion);
    for (let i: int = 0; i < 3; i++) {
        arktest.assertEQ(fromUnion[i], i + 1);
    }
    let arrayIter: Iterable<number> = new Array<number>(1, 2, 3)
    let fromIter = Array.from(arrayIter);
    for (let i: int = 0; i < 3; i++) {
        arktest.assertEQ(fromIter[i], i + 1);
    }
    let arrCustom: Array<ArrayLike<number> | Iterable<number>> = new Array<ArrayLike<number> | Iterable<number>>(2);
    let tmpArrLike: ArrayLikeImplTest<number> = new ArrayLikeImplTest<number>();
    tmpArrLike.push(1);
    arrCustom[0] = tmpArrLike;
    arrCustom[1] = new ArrayValueIteratorTest();
    let fromArr1 = Array.from(arrCustom[0]);
    let fromArr2 = Array.from(arrCustom[1]);
    for (let i: int = 0; i < 1; i++) {
        arktest.assertEQ(fromArr1[i], 1);
        arktest.assertEQ(fromArr2[i], 1);
    }
}
