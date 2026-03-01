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
let ns = STValue.findNamespace("stvalue_unwrap.Unwrap");
let stvalueWrap;
const epsilon = 1e-5;
let SType = etsVm.SType;

function testUnwrapToNumber(): void {
    stvalueWrap = STValue.wrapByte(1);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 1);

    stvalueWrap = STValue.wrapChar('1');
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 49); // ASCII code for '1'

    stvalueWrap = STValue.wrapShort(32767);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 32767);

    stvalueWrap = STValue.wrapShort(-32768);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === -32768);

    stvalueWrap = STValue.wrapInt(44);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 44);

    stvalueWrap = STValue.wrapLong(44);
    ASSERT_TRUE(stvalueWrap.unwrapToNumber() === 44);


    stvalueWrap = STValue.wrapFloat(44.4);
    ASSERT_TRUE(Math.abs(stvalueWrap.unwrapToNumber() - 44.4) <= epsilon);

    stvalueWrap = STValue.wrapNumber(44.4);
    ASSERT_TRUE(Math.abs(stvalueWrap.unwrapToNumber() - 44.4) <= epsilon);

    stvalueWrap = STValue.wrapBoolean(true);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    let res = false;
    try {
        let magicSTValueNull = STValue.getNull();
        let magicNull = magicSTValueNull.unwrapToNumber();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type primitive");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueBigIntFromString = ns.namespaceGetVariable('bigIntFromString', SType.REFERENCE);
        let magicDouble = magicSTValueBigIntFromString.unwrapToNumber();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type primitive");
    }
    ASSERT_TRUE(res);
}

function testUnwrapToString(): void {
    let res = false;
    try {
        let magicSTValueNull = STValue.getNull();
        let magicNull = magicSTValueNull.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type std.core.String");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueUndefined = STValue.getUndefined();
        let magicUndefined = magicSTValueUndefined.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type std.core.String");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueBoolean = ns.namespaceGetVariable('magicBoolean', SType.BOOLEAN);
        let magicFloat = magicSTValueBoolean.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueByte = ns.namespaceGetVariable('magicByte', SType.BYTE);
        let magicByte = magicSTValueByte.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueChar = ns.namespaceGetVariable('magicChar', SType.CHAR);
        let magicChar = magicSTValueChar.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueShort = ns.namespaceGetVariable('magicShort', SType.SHORT);
        let magicShort = magicSTValueShort.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueInt = ns.namespaceGetVariable('magicInt', SType.INT);
        let magicInt = magicSTValueInt.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueLong = ns.namespaceGetVariable('magicLong', SType.LONG);
        let magicLong = magicSTValueLong.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueFloat = ns.namespaceGetVariable('magicFloat', SType.FLOAT);
        let magicFloat = magicSTValueFloat.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueDouble = ns.namespaceGetVariable('magicDouble', SType.DOUBLE);
        let magicDouble = magicSTValueDouble.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueBigIntFromString = ns.namespaceGetVariable('bigIntFromString', SType.REFERENCE);
        let magicDouble = magicSTValueBigIntFromString.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type std.core.String");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        let magicSTValueBigIntFromLiteral = ns.namespaceGetVariable('bigIntFromLiteral', SType.REFERENCE);
        let magicDouble = magicSTValueBigIntFromLiteral.unwrapToString();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type std.core.String");
    }
    ASSERT_TRUE(res);

    let magicSTValue1 = ns.namespaceGetVariable('magicString1', SType.REFERENCE);
    let magicString1 = magicSTValue1.unwrapToString();
    print('magicString1: ', magicString1);
    ASSERT_TRUE(magicString1 === 'Hello World');

    let magicSTValue2 = ns.namespaceGetVariable('magicString2', SType.REFERENCE);
    let magicString2 = magicSTValue2.unwrapToString();
    print('magicString2: ', magicString2);
    ASSERT_TRUE(magicString2 === 'Hello World!!!');

}

function testUnwrapToBigInt(): void {
    // testUnwrapToBigInt
    const bigIntFromStringValue = ns.namespaceGetVariable('bigIntFromString', SType.REFERENCE);
    const bigIntFromString = bigIntFromStringValue.unwrapToBigInt();

    ASSERT_TRUE(bigIntFromString === 999999999999999999n);
    ASSERT_TRUE(typeof bigIntFromString === 'bigint');

    const bigIntFromLiteralValue = ns.namespaceGetVariable('bigIntFromLiteral', SType.REFERENCE);
    const bigIntFromLiteral = bigIntFromLiteralValue.unwrapToBigInt();

    ASSERT_TRUE(bigIntFromLiteral === 123456789012345678901234567890n);
    ASSERT_TRUE(typeof bigIntFromLiteral === 'bigint');

    // testArithmeticOperations
    const bigIntValue = ns.namespaceGetVariable('bigIntFromString', SType.REFERENCE).unwrapToBigInt();

    const additionResult = bigIntValue + 1n;
    const multiplicationResult = bigIntValue * 2n;
    const subtractionResult = bigIntValue - 1000n;

    ASSERT_TRUE(additionResult === 1000000000000000000n);
    ASSERT_TRUE(multiplicationResult === 1999999999999999998n);
    ASSERT_TRUE(subtractionResult === 999999999999998999n);

    // testTypeCheck
    try {
        const numberValue = ns.namespaceGetVariable('defaultNumber', SType.REFERENCE);
        const result = numberValue.unwrapToBigInt();
        ASSERT_TRUE(false, 'Should have thrown error for non-BigInt conversion');
    } catch (error) {
        print('Expected error for non-BigInt conversion:', error.message);
        ASSERT_TRUE(true, 'Correctly threw error for non-BigInt conversion');
    }

    try {
        const stringVariable = ns.namespaceGetVariable('stringVariable', SType.REFERENCE);
        const result = stringVariable.unwrapToBigInt();
        print('Non-BigInt conversion result:', result);
        ASSERT_TRUE(false, 'Should have thrown error for non-BigInt conversion');
    } catch (error) {
        print('Expected error for non-BigInt conversion:', error.message);
        ASSERT_TRUE(true, 'Correctly threw error for non-BigInt conversion');
    }

    // testEdgeCases
    let zeroSTValue = ns.namespaceGetVariable('zeroBigInt', SType.REFERENCE);
    let zeroBigInt = zeroSTValue.unwrapToBigInt();

    ASSERT_TRUE(zeroBigInt === 0n);

    let negativeSTValue = ns.namespaceGetVariable('negativeBigInt', SType.REFERENCE);
    let negativeBigInt = negativeSTValue.unwrapToBigInt();

    ASSERT_TRUE(negativeBigInt === -1234567890123456789n);

    let res = false;
    try {
        let magicSTValueNull = STValue.getNull();
        magicSTValueNull.unwrapToBigInt();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("Expected BigInt object, but got different type.");
    }
    ASSERT_TRUE(res);
}
function testUnwrapToBoolean(): void {
    stvalueWrap = STValue.wrapByte(127);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapByte(0);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === false);

    stvalueWrap = STValue.wrapByte(-44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapByte(-128);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true); // -128 = 0b(10000000) ?

    stvalueWrap = STValue.wrapInt(44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapInt(0);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === false);

    stvalueWrap = STValue.wrapInt(-44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapInt(-2147483648);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === false);

    stvalueWrap = STValue.wrapShort(32767);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapShort(-32767);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapShort(-32768);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === false); //-32768 = 0b(10000000 00000000)

    stvalueWrap = STValue.wrapLong(44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapLong(-44);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapChar('1');
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true); // ASCII code for '1'

    stvalueWrap = STValue.wrapChar('0');
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true); // ASCII code for '0'

    stvalueWrap = STValue.wrapFloat(44.4);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapFloat(0);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === false);

    stvalueWrap = STValue.wrapNumber(44.4);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === true);

    stvalueWrap = STValue.wrapNumber(0);
    ASSERT_TRUE(stvalueWrap.unwrapToBoolean() === false);

    let res = false;
    try {
        let magicSTValueNull = STValue.getNull();
        magicSTValueNull.unwrapToBoolean();
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type primitive");
    }
    ASSERT_TRUE(res);
    print('UnwrapToBoolean Test SUCCESS!')
}


function main(): void {
    testUnwrapToNumber();
    testUnwrapToString();
    testUnwrapToBigInt();
    testUnwrapToBoolean();
}

main();
