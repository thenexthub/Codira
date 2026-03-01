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

function checkBoolean(val: boolean) {
    arktest.assertTrue(val == true)
}

function checkBooleanBoxed(val: Boolean) {
    arktest.assertTrue(val == true)
}

function checkByte(val: byte) {
    arktest.assertTrue(val == 1)
}

function checkByteOverflow(val: byte, oriVal: int) {
    arktest.assertTrue(val == Int.toByte(oriVal))
}

function checkByteBoxed(val: Byte) {
    arktest.assertTrue(val == 1)
}

function checkShort(val: short) {
    arktest.assertTrue(val == 1)
}

function checkShortBoxed(val: Short) {
    arktest.assertTrue(val == 1)
}

function checkShortOverflow(val: short, oriVal: int) {
    arktest.assertTrue(val == Int.toShort(oriVal))
}

function checkInt(val: int) {
    arktest.assertTrue(val == 1)
}

function checkIntBoxed(val: Int) {
    arktest.assertTrue(val == 1)
}

function checkIntOverflow(val: int, oriVal: long) {
    arktest.assertTrue(val == Long.toInt(oriVal))
}

function checkLong(val: long) {
    arktest.assertTrue(val == 1)
}

function checkLongBoxed(val: Long) {
    arktest.assertTrue(val == 1)
}

function checkFloat(val: float) {
     arktest.assertTrue(val == 2.1f)
}

function checkFloatBoxed(val: Float) {
    arktest.assertTrue(val == 2.1f)
}

function checkFloatOverflow(val: float, oriVal: number) {
    arktest.assertTrue(val == Double.toFloat(oriVal))
}

function checkDouble(val: double) {
    arktest.assertTrue(val == 1)
}

function checkDoubleBoxed(val: Double) {
    arktest.assertTrue(val == 1)
}

function checkNumber(val: number) {
    arktest.assertTrue(val == 1)
}

function checkNumberBoxed(val: Number) {
    arktest.assertTrue(val == 1)
}

function checkChar(val: char) {
    arktest.assertTrue(val == c'1')
}

function checkCharBoxed(val: Char) {
    arktest.assertTrue(val == c'1')
}

function checkString(val: string) {
    arktest.assertTrue(val == '1')
}

function checkStringBoxed(val: String) {
    arktest.assertTrue(val == '1')
}

function checkBigint(val: bigint) {
    arktest.assertTrue(val == 1n)
}

function checkBigintBoxed(val: BigInt) {
    arktest.assertTrue(val == 1n)
}

function checkNull(val: null) {
    return val === null
}

function checkUndefined(val: null) {
    return val === undefined
}
