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

const etsVm = globalThis.gtest.etsVm;

let STValue = etsVm.STValue;
const epsilon = 1e-5;
let stvalueWrap;
let SType = etsVm.SType;

function testWrapByte(): void {
    stvalueWrap = STValue.wrapByte(1);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 1); // ASCII code for '1'
}

function testWrapChar(): void {
    stvalueWrap = STValue.wrapChar('1');
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 49); // ASCII code for '1'
}

function testWrapShort(): void {
    stvalueWrap = STValue.wrapShort(32767);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 32767);

    stvalueWrap = STValue.wrapShort(-32768);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === -32768);
}

function testWrapInt(): void {
    let test_value: number = 44;
    let stvalueWrap = STValue.wrapInt(test_value);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === test_value);

    const MAX_INT = 2147483647;
    stvalueWrap = STValue.wrapInt(MAX_INT);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === MAX_INT);

    const MIN_INT = -2147483648;
    stvalueWrap = STValue.wrapInt(MIN_INT);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === MIN_INT);

    const OVER_MAX_INT = 2147483648;
    let overMaxIntErrorHappen: boolean = false;
    try {
        STValue.wrapInt(OVER_MAX_INT);
    } catch (e) {
        overMaxIntErrorHappen = true;
    }
    ASSERT_TRUE(overMaxIntErrorHappen);

    const OVER_MIN_INT = 2147483648;
    let overMinIntErrorHappen: boolean = false;
    try {
        STValue.wrapInt(OVER_MIN_INT);
    } catch (e) {
        overMinIntErrorHappen = true;
    }
    ASSERT_TRUE(overMinIntErrorHappen);
}

function testWrapLongWithNumberInput(): void {
    let test_value: number = 44;
    let stvalueWrap = STValue.wrapLong(test_value);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === test_value);

    const MAX_SAFE_LONG = Number.MAX_SAFE_INTEGER;
    stvalueWrap = STValue.wrapLong(MAX_SAFE_LONG);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === MAX_SAFE_LONG);

    const MIN_SAFE_LONG = Number.MIN_SAFE_INTEGER;
    stvalueWrap = STValue.wrapLong(MIN_SAFE_LONG);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === MIN_SAFE_LONG);

    /*
    const OVER_MAX_LONG = MAX_SAFE_LONG + 1;
    let overMaxLongErrorHappen: boolean = false;
    try {
        STValue.wrapLong(OVER_MAX_LONG);
    } catch (e) {
        overMaxLongErrorHappen = true;
    }
    ASSERT_TRUE(overMaxLongErrorHappen);

    const OVER_MIN_LONG = -OVER_MAX_LONG;
    let overMinLongErrorHappen: boolean = false;
    try {
        STValue.wrapLong(OVER_MIN_LONG);
    } catch (e) {
        overMinLongErrorHappen = true;
    }
    ASSERT_TRUE(overMinLongErrorHappen);
    */
}

function testWrapLongWithBigIntInput(): void {
    let test_value = 44n;
    let stvalueWrap = STValue.wrapLong(test_value);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === Number(test_value));

    const longKlass = STValue.findClass("std.core.Long");

    const MAX_LONG = (1n << 63n) - 1n;
    stvalueWrap = STValue.wrapLong(MAX_LONG);
    let value = longKlass.classInstantiate('l:', [stvalueWrap]);
    let res = value.objectInvokeMethod('toString', ':C{std.core.String}', []);
    print("res.unwrapToString() = " + res.unwrapToString());
    print("MAX_LONG.toString() = " + MAX_LONG.toString());
    ASSERT_TRUE(res.unwrapToString() === MAX_LONG.toString());

    const MIN_LONG = -(1n << 63n);
    stvalueWrap = STValue.wrapLong(MIN_LONG);
    value = longKlass.classInstantiate('l:', [stvalueWrap]);
    res = value.objectInvokeMethod('toString', ':C{std.core.String}', []);
    ASSERT_TRUE(res.unwrapToString() === MIN_LONG.toString());

    const OVER_MAX_LONG = MAX_LONG + 1n;
    let overMaxLongErrorHappen: boolean = false;
    try {
        STValue.wrapLong(OVER_MAX_LONG);
    } catch (e) {
        overMaxLongErrorHappen = true;
    }
    ASSERT_TRUE(overMaxLongErrorHappen);

    const OVER_MIN_LONG = MIN_LONG - 1n;
    let overMinLongErrorHappen: boolean = false;
    try {
        STValue.wrapLong(OVER_MIN_LONG);
    } catch (e) {
        overMinLongErrorHappen = true;
    }
    ASSERT_TRUE(overMinLongErrorHappen);
}

function testWrapLong(): void {
    testWrapLongWithNumberInput();
    testWrapLongWithBigIntInput();
}

function testWrapFloat(): void {
    let test_value: number = 44.4;
    let stvalueWrap = STValue.wrapFloat(test_value);
    ASSERT_TRUE(Math.abs(stvalueWrap.unwrapToNumber() - test_value) <= epsilon);

    const NAN = Number.NaN;
    stvalueWrap = STValue.wrapFloat(NAN);
    ASSERT_TRUE(Number.isNaN(stvalueWrap.unwrapToNumber()));

    const P_INF = Number.POSITIVE_INFINITY;
    stvalueWrap = STValue.wrapFloat(P_INF);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === P_INF);

    const N_INF = Number.NEGATIVE_INFINITY;
    stvalueWrap = STValue.wrapFloat(N_INF);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === N_INF);
}

function testWrapNumber(): void {
    stvalueWrap = STValue.wrapNumber(44.4);
    ASSERT_TRUE(Math.abs(stvalueWrap.unwrapToNumber() - 44.4) <= epsilon);

    const NAN = Number.NaN;
    stvalueWrap = STValue.wrapNumber(NAN);
    ASSERT_TRUE(Number.isNaN(stvalueWrap.unwrapToNumber()));

    const P_INF = Number.POSITIVE_INFINITY;
    stvalueWrap = STValue.wrapNumber(P_INF);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === P_INF);

    const N_INF = Number.NEGATIVE_INFINITY;
    stvalueWrap = STValue.wrapNumber(N_INF);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === N_INF);
}

function testWrapString(): void {
    stvalueWrap = STValue.wrapString('magicString1');
    ASSERT_TRUE(stvalueWrap.unwrapToString() === 'magicString1');
}

function testWrapBoolean(): void {
    stvalueWrap = STValue.wrapBoolean(true);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);
}

function testWrapBigInt(): void {
    stvalueWrap = STValue.wrapBigInt(1234567890123456789n);
    ASSERT_TRUE(stvalueWrap.unwrapToBigInt() === 1234567890123456789n);
}

function testGetNull(): void {
    stvalueWrap = STValue.getNull();
    ASSERT_TRUE(stvalueWrap.isEqualTo(STValue.getNull()));
    ASSERT_TRUE(!stvalueWrap.isEqualTo(STValue.wrapString('magicString1')))
}

function testGetUndefined(): void {
    stvalueWrap = STValue.getUndefined();
    ASSERT_TRUE(stvalueWrap.isEqualTo(STValue.getUndefined()));
    ASSERT_TRUE(!stvalueWrap.isEqualTo(STValue.wrapString('magicString1')))
}

function main(): void {
    testWrapByte();
    testWrapChar();
    testWrapShort();
    testWrapInt();
    testWrapLong();
    testWrapFloat();
    testWrapNumber();
    testWrapString();
    testWrapBoolean();
    testWrapBigInt();
    testGetNull();
    testGetUndefined();
}

main();
