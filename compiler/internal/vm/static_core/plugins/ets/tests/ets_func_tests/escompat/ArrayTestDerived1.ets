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

class ArrayDerived<T> extends Array<T> {
    public constructor(arr: Array<T>) {
        super()
        for (let i = 0; i < arr.length; i++) {
            this.push(arr[i])
        }
    }
}

class ArrayWithNegativeLength<T> extends Array<T> {
    protected negativeLength: int

    public constructor(arr: Array<T>) {
        super()
        this.negativeLength = -1;
        for (let i = 0; i < arr.length; i++) {
            this.push(arr[i])
        }
    }

    override get length(): int {
        return this.negativeLength
    }

    override set length(newLen: int) {
        this.negativeLength = newLen
    }
}

function testArrayDerivedUnshift() {
    let a = new ArrayDerived<number>([4, 5, 6])
    a.unshift(1, 2, 3)

    arktest.assertEQ(a.length, 6)
    arktest.assertEQ(a[0], 1)
    arktest.assertEQ(a[1], 2)
    arktest.assertEQ(a[2], 3)
    arktest.assertEQ(a[3], 4)
    arktest.assertEQ(a[4], 5)
    arktest.assertEQ(a[5], 6)
}

function testFromCharCodeWithArrayDerived() {
    // Using reflection to invoke the method on the overridden class instance
    const stringClass = Class.from<string>();
    const staticMethod = stringClass.getStaticMethod("fromCharCode", [Class.from<Array<number>>()])

    let emptyArray = new ArrayDerived<number>([])
    arktest.assertEQ(staticMethod!.invoke([emptyArray]), "")

    let negativeLengthArray = new ArrayWithNegativeLength<number>([1, 2, 4]);
    const errorcheck = (e: Error) => {
        return (e instanceof NegativeArraySizeError) && e.message === "The value must be non negative"
    }
    arktest.expectThrow((): void => { staticMethod!.invoke([negativeLengthArray]) }, errorcheck)
}

function main(): int {
    testArrayDerivedUnshift()
    testFromCharCodeWithArrayDerived()

    return 0;
}
