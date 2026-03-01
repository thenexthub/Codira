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

const checkBoolean = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkBoolean');
const checkBooleanBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkBooleanBoxed');
const checkByte = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkByte');
const checkByteBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkByteBoxed');
const checkByteOverflow = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkByteOverflow');
const checkShort = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkShort');
const checkShortBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkShortBoxed');
const checkShortOverflow = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkShortOverflow');
const checkInt = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkInt');
const checkIntBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkIntBoxed');
const checkIntOverflow = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkIntOverflow');
const checkLong = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkLong');
const checkLongBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkLongBoxed');
const checkFloat = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkFloat');
const checkFloatBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkFloatBoxed');
const checkFloatOverflow = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkFloatOverflow');
const checkDouble = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkDouble');
const checkDoubleBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkDoubleBoxed');
const checkNumber = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkNumber');
const checkNumberBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkNumberBoxed');
const checkChar = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkChar');
const checkCharBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkCharBoxed');
const checkString = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkString');
const checkStringBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkStringBoxed');
const checkBigint = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkBigint');
const checkBigintBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkBigintBoxed');
const checkNull = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkNull');
const checkUndefined = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'checkUndefined');


function testCheckBoolean(): void {
    let val = true;
    let valBoxed = Boolean(val);
    let valObject = new Boolean(val);
    checkBoolean(val);
    checkBoolean(valBoxed);
    checkBoolean(valObject);
    checkBooleanBoxed(val);
    checkBooleanBoxed(valBoxed);
    checkBooleanBoxed(valObject);
    ASSERT_THROWS(TypeError, () => checkBoolean(1));
    ASSERT_THROWS(TypeError, () => checkBooleanBoxed(1));
}

function testCheckByte(): void {
    let val = 1;
    let valBoxed = Number(val);
    let valObject = new Number(val);
    checkByte(val);
    checkByte(valBoxed);
    checkByte(valObject);
    checkByteBoxed(val);
    checkByteBoxed(valBoxed);
    checkByteBoxed(valObject);
    let overflowValUp = 127 + 1;
    let overflowValDown = -128 - 1;
    checkByteOverflow(overflowValUp, overflowValUp);
    checkByteOverflow(overflowValDown, overflowValDown);
    ASSERT_THROWS(TypeError, () => checkByte('1'));
    ASSERT_THROWS(TypeError, () => checkByteBoxed('1'));
}

function testCheckShort(): void {
    let val = 1;
    let valBoxed = Number(val);
    let valObject = new Number(val);
    checkShort(val);
    checkShort(valBoxed);
    checkShort(valObject);
    checkShortBoxed(val);
    checkShortBoxed(valBoxed);
    checkShortBoxed(valObject);
    let overflowValUp = 32767 + 1;
    let overflowValDown = -32768 - 1;
    checkShortOverflow(overflowValUp, overflowValUp);
    checkShortOverflow(overflowValDown, overflowValDown);
    ASSERT_THROWS(TypeError, () => checkShort('1'));
    ASSERT_THROWS(TypeError, () => checkShortBoxed('1'));
}

function testCheckInt(): void {
    let val = 1;
    let valBoxed = Number(val);
    let valObject = new Number(val);
    checkInt(val);
    checkInt(valBoxed);
    checkInt(valObject);
    checkIntBoxed(val);
    checkIntBoxed(valBoxed);
    checkIntBoxed(valObject);
    let overflowValUp = 2147483647 + 1;
    let overflowValDown = -2147483648 - 1;
    checkIntOverflow(overflowValUp, overflowValUp);
    checkIntOverflow(overflowValDown, overflowValDown);
    ASSERT_THROWS(TypeError, () => checkInt('1'));
    ASSERT_THROWS(TypeError, () => checkIntBoxed('1'));
}

function testCheckLong(): void {
    let val = 1;
    let valBoxed = Number(val);
    let valObject = new Number(val);
    checkLong(val);
    checkLong(valBoxed);
    checkLong(valObject);
    checkLongBoxed(val);
    checkLongBoxed(valBoxed);
    checkLongBoxed(valObject);
    ASSERT_THROWS(TypeError, () => checkLong('1'));
    ASSERT_THROWS(TypeError, () => checkLongBoxed('1'));
}

function testCheckFloat(): void {
    let val = 2.1;
    let valBoxed = Number(val);
    let valObject = new Number(val);
    checkFloat(val);
    checkFloat(valBoxed);
    checkFloat(valObject);
    checkFloatBoxed(val);
    checkFloatBoxed(valBoxed);
    checkFloatBoxed(valObject);
    let overflowVal = 1.11555555444113;
    checkFloatOverflow(overflowVal, overflowVal);
    ASSERT_THROWS(TypeError, () => checkFloat('1'));
    ASSERT_THROWS(TypeError, () => checkFloatBoxed('1'));
}

function testCheckDouble(): void {
    let val = 1;
    let valBoxed = Number(val);
    let valObject = new Number(val);
    checkDouble(val);
    checkDouble(valBoxed);
    checkDouble(valObject);
    checkDoubleBoxed(val);
    checkDoubleBoxed(valBoxed);
    checkDoubleBoxed(valObject);
    ASSERT_THROWS(TypeError, () => checkDouble('1'));
    ASSERT_THROWS(TypeError, () => checkDoubleBoxed('1'));
}

function testCheckNumber(): void {
    let val = 1;
    let valBoxed = Number(val);
    let valObject = new Number(val);
    checkNumber(val);
    checkNumber(valBoxed);
    checkNumber(valObject);
    checkNumberBoxed(val);
    checkNumberBoxed(valBoxed);
    checkNumberBoxed(valObject);
    ASSERT_THROWS(TypeError, () => checkNumber('1'));
    ASSERT_THROWS(TypeError, () => checkNumberBoxed('1'));
}

function testCheckChar(): void {
    let val = '1';
    let valBoxed = String(val);
    let valObject = new String(val);
    checkChar(val);
    checkChar(valBoxed);
    checkChar(valObject);
    checkCharBoxed(val);
    checkCharBoxed(valBoxed);
    checkCharBoxed(valObject);
    let valNumber = 49;
    checkChar(valNumber);
    checkChar(Number(valNumber));
    checkChar(new Number(valNumber));
    checkCharBoxed(valNumber);
    checkCharBoxed(Number(valNumber));
    checkCharBoxed(new Number(valNumber));
    ASSERT_THROWS(TypeError, () => checkChar('𬌗'));
    ASSERT_THROWS(TypeError, () => checkCharBoxed('𬌗'));
    ASSERT_THROWS(TypeError, () => checkChar(65536));
    ASSERT_THROWS(TypeError, () => checkCharBoxed(65536));
}

function testCheckString(): void {
    let val = '1';
    let valBoxed = String(val);
    let valObject = new String(val);
    checkString(val);
    checkString(valBoxed);
    checkString(valObject);
    checkStringBoxed(val);
    checkStringBoxed(valBoxed);
    checkStringBoxed(valObject);
    ASSERT_THROWS(TypeError, () => checkString(1));
    ASSERT_THROWS(TypeError, () => checkStringBoxed(1));
}

function testCheckBigint(): void {
    let val = 1n;
    let valBoxed = BigInt(val);
    checkBigint(val);
    checkBigint(valBoxed);
    checkBigintBoxed(val);
    checkBigintBoxed(valBoxed);
    ASSERT_THROWS(TypeError, () => checkBigint(1));
    ASSERT_THROWS(TypeError, () => checkBigintBoxed(1));
}

function testNullAndUndefined(): void {
    ASSERT_TRUE(checkNull(null) === true);
    ASSERT_TRUE(checkNull(undefined) === false);
    ASSERT_THROWS(TypeError, () => checkNull(1));
    ASSERT_THROWS(TypeError, () => checkUndefined(1));
    ASSERT_TRUE(checkUndefined(null) === false );
    ASSERT_TRUE(checkUndefined(undefined) === true);
}


testCheckBoolean();
testCheckByte();
testCheckShort();
testCheckInt();
testCheckLong();
testCheckFloat();
testCheckDouble();
testCheckNumber();
testCheckChar();
testCheckString();
testCheckBigint();
testNullAndUndefined();
