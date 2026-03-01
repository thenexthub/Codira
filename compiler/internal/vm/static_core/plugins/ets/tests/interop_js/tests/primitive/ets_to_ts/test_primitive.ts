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

const getBooleanPrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getBooleanPrimitive');
const getBooleanBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getBooleanBoxed');
const getBytePrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getBytePrimitive');
const getByteBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getByteBoxed');
const getShortPrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getShortPrimitive');
const getShortBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getShortBoxed');
const getIntPrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getIntPrimitive');
const getIntBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getIntBoxed');
const getLongPrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getLongPrimitive');
const getLongBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getLongBoxed');
const getFloatPrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getFloatPrimitive');
const getFloatBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getFloatBoxed');
const getNumberPrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getNumberPrimitive');
const getNumberBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getNumberBoxed');
const getDoublePrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getDoublePrimitive');
const getDoubleBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getDoubleBoxed');
const getStringPrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getStringPrimitive');
const getStringBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getStringBoxed');
const getCharPrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getCharPrimitive');
const getCharPrimitiveNumber = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getCharPrimitiveNumber');
const getCharBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getCharBoxed');
const getCharBoxedNumber = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getCharBoxedNumber');
const getBigIntPrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getBigIntPrimitive');
const getBigIntBoxed = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getBigIntBoxed');
const getNullPrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getNullPrimitive');
const getUndefinedPrimitive = etsVm.getFunction('Lprimitive/ETSGLOBAL;', 'getUndefinedPrimitive');


function testGetBooleanPrimitive(): void {
    let val = getBooleanPrimitive();
    ASSERT_TRUE(typeof val === 'boolean');
    ASSERT_TRUE(val === true);
}

function testGetBooleanBoxed(): void {
    let val = getBooleanBoxed();
    ASSERT_TRUE(typeof val === 'boolean');
    ASSERT_TRUE(val === true);
}

function testGetBytePrimitive(): void {
    let val = getBytePrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetByteBoxed(): void {
    let val = getByteBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetShortPrimitive(): void {
    let val = getShortPrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetShortBoxed(): void {
    let val = getShortBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetIntPrimitive(): void {
    let val = getIntPrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetIntBoxed(): void {
    let val = getIntBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetLongPrimitive(): void {
    let val = getLongPrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetLongBoxed(): void {
    let val = getLongBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetFloatPrimitive(): void {
    let val = getFloatPrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetFloatBoxed(): void {
    let val = getFloatBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetNumberPrimitive(): void {
    let val = getNumberPrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetNumberBoxed(): void {
    let val = getNumberBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetDoublePrimitive(): void {
    let val = getDoublePrimitive();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetDoubleBoxed(): void {
    let val = getDoubleBoxed();
    ASSERT_TRUE(typeof val === 'number');
    ASSERT_TRUE(val === 1);
}

function testGetStringPrimitive(): void {
    let val = getStringPrimitive();
    ASSERT_TRUE(typeof val === 'string');
    ASSERT_TRUE(val === '1');
}

function testGetStringBoxed(): void {
    let val = getStringBoxed();
    ASSERT_TRUE(typeof val === 'string');
    ASSERT_TRUE(val === '1');
}

function testGetCharPrimitive(): void {
    let val = getCharPrimitive();
    ASSERT_TRUE(typeof val === 'string');
    ASSERT_TRUE(val === '1');
}

function testGetCharPrimitiveNumber(): void {
    let val = getCharPrimitiveNumber();
    ASSERT_TRUE(typeof val === 'string');
    ASSERT_TRUE(val === '1');
}

function testGetCharBoxed(): void {
    let val = getCharBoxed();
    ASSERT_TRUE(typeof val === 'string');
    ASSERT_TRUE(val === '1');
}

function testGetCharBoxedNumber(): void {
    let val = getCharBoxedNumber();
    ASSERT_TRUE(typeof val === 'string');
    ASSERT_TRUE(val === '1');
}

function testGetBigIntPrimitive(): void {
    let val = getBigIntPrimitive();
    ASSERT_TRUE(typeof val === 'bigint');
    ASSERT_TRUE(val === 1n);
}

function testGetBigIntBoxed(): void {
    let val = getBigIntBoxed();
    ASSERT_TRUE(typeof val === 'bigint');
    ASSERT_TRUE(val === 1n);
}

function testGetNullPrimitive(): void {
    let val = getNullPrimitive();
    ASSERT_TRUE(typeof val === 'object');
    ASSERT_TRUE(val === null);
}

function testGetUndefinedPrimitive(): void {
    let val = getUndefinedPrimitive();
    ASSERT_TRUE(typeof val === 'undefined');
    ASSERT_TRUE(val === undefined);
}


testGetBooleanPrimitive();
testGetBooleanBoxed();
testGetBytePrimitive();
testGetByteBoxed();
testGetShortPrimitive();
testGetShortBoxed();
testGetIntPrimitive();
testGetIntBoxed();
testGetLongPrimitive();
testGetLongBoxed();
testGetIntBoxed();
testGetFloatPrimitive();
testGetFloatBoxed();
testGetNumberPrimitive();
testGetNumberBoxed();
testGetDoublePrimitive();
testGetDoubleBoxed();
testGetCharPrimitive();
testGetCharPrimitiveNumber();
testGetCharBoxed();
testGetCharBoxedNumber();
testGetStringPrimitive();
testGetStringBoxed();
testGetBigIntPrimitive();
testGetBigIntBoxed();
testGetNullPrimitive();
testGetUndefinedPrimitive();