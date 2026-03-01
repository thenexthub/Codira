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

function ArrayHasOwnProperty(): void {
    let arr: Array<number> = [1, 2, 3]
    arktest.assertTrue(arr.hasOwnProperty(1))
    arktest.assertFalse(arr.hasOwnProperty(4))
}

function TypedArrayHasOwnProperty(): void {
    let arrI8 = new Int8Array(10)
    arktest.assertTrue(arrI8.hasOwnProperty(9))
    arktest.assertFalse(arrI8.hasOwnProperty(10))
    let arrI16 = new Int16Array(10)
    arktest.assertTrue(arrI16.hasOwnProperty(9))
    arktest.assertFalse(arrI16.hasOwnProperty(10))
    let arrI32 = new Int32Array(10)
    arktest.assertTrue(arrI32.hasOwnProperty(9))
    arktest.assertFalse(arrI32.hasOwnProperty(10))
    let arrBig64 = new BigInt64Array(10)
    arktest.assertTrue(arrBig64.hasOwnProperty(9))
    arktest.assertFalse(arrBig64.hasOwnProperty(10))
    let arrF32 = new Float32Array(10)
    arktest.assertTrue(arrF32.hasOwnProperty(9))
    arktest.assertFalse(arrF32.hasOwnProperty(10))
    let arrF64 = new Float64Array(10)
    arktest.assertTrue(arrF64.hasOwnProperty(9))
    arktest.assertFalse(arrF64.hasOwnProperty(10))
}

function TypedUArrayHasOwnProperty(): void {
    let arrUC8 = new Uint8ClampedArray(10)
    arktest.assertTrue(arrUC8.hasOwnProperty(9))
    arktest.assertFalse(arrUC8.hasOwnProperty(10))
    let arrU8 = new Uint8Array(10)
    arktest.assertTrue(arrU8.hasOwnProperty(9))
    arktest.assertFalse(arrU8.hasOwnProperty(10))
    let arrU16 = new Uint16Array(10)
    arktest.assertTrue(arrU16.hasOwnProperty(9))
    arktest.assertFalse(arrU16.hasOwnProperty(10))
    let arrU32 = new Uint32Array(10)
    arktest.assertTrue(arrU32.hasOwnProperty(9))
    arktest.assertFalse(arrU32.hasOwnProperty(10))
    let arrUBig64 = new BigUint64Array(10)
    arktest.assertTrue(arrUBig64.hasOwnProperty(9))
    arktest.assertFalse(arrUBig64.hasOwnProperty(10))
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Object hasOwnProperty tests")
    suite.addTest('Object.hasOwnProperty for Array', ArrayHasOwnProperty);
    suite.addTest('Object.hasOwnProperty for TypedArray', TypedArrayHasOwnProperty);
    suite.addTest('Object.hasOwnProperty for TypedUArray', TypedUArrayHasOwnProperty);

    return suite.run()
}