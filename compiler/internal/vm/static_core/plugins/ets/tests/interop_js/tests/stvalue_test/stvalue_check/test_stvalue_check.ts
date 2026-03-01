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
let ns = STValue.findNamespace('stvalue_check.Check')
let SType = etsVm.SType;

let studentCls = STValue.findClass('stvalue_check.Student');
let subStudentCls = STValue.findClass('stvalue_check.SubStudent');

function testIsStringInvalidParam(): void {
    // invalid method
    try {
        ASSERT_TRUE(STValue.isString() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    try {
        ASSERT_TRUE('STValue'.isString() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // param arg invalid
    let data = STValue.wrapString('I am a Chinese');
    try {
        ASSERT_TRUE(data.isString(1) === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testIsString(): void {
    // 1. invalid param
    testIsStringInvalidParam();

    // 2. null and undefined
    let data = STValue.getNull();
    ASSERT_TRUE(data.isString() === false);

    data = STValue.getUndefined();
    ASSERT_TRUE(data.isString() === false);

    // 3. not string type
    data = STValue.wrapInt(123);
    ASSERT_TRUE(data.isString() === false);

    data = STValue.wrapBigInt(123456789n);
    ASSERT_TRUE(data.isString() === false);

    data = ns.namespaceGetVariable('subStu', SType.REFERENCE);
    ASSERT_TRUE(data.isString() === false);

    // 4. linestring , treestring and slicedstring
    data = STValue.wrapString('I am a Chinese');
    ASSERT_TRUE(data.isString() === true);

    data = ns.namespaceGetVariable('shouldBeString', SType.REFERENCE);
    ASSERT_TRUE(data.isString() === true);

    data = ns.namespaceGetVariable('treeString', SType.REFERENCE);
    ASSERT_TRUE(data.isString() === true);

    data = ns.namespaceGetVariable("slicedString", SType.REFERENCE);
    ASSERT_TRUE(data.isString() === true);
}

function testIsBigIntInvalidParam(): void {
    // invalid method
    try {
        ASSERT_TRUE(STValue.isBigInt() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    try {
        ASSERT_TRUE('STValue'.isBigInt() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // param arg invalid
    let data = STValue.wrapBigInt(1234567n);
    try {
        ASSERT_TRUE(data.isBigInt(1) === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testIsBigInt(): void {
    // 1. invalid param
    testIsBigIntInvalidParam();
    let data = STValue.wrapInt(123);
    ASSERT_TRUE(data.isBigInt() === false);

    // 2. null and undefined check
    data = STValue.getNull();
    ASSERT_TRUE(data.isBigInt() === false);

    data = STValue.getUndefined();
    ASSERT_TRUE(data.isBigInt() === false);

    // 3. not bigint type
    data = ns.namespaceGetVariable('shouldBeBooleanTrue', SType.BOOLEAN);
    ASSERT_TRUE(data.isBigInt() === false);

    data = STValue.wrapString('I am a Chinese');
    ASSERT_TRUE(data.isBigInt() === false);

    data = ns.namespaceGetVariable('shouldBeString', SType.REFERENCE);
    ASSERT_TRUE(data.isBigInt() === false);

    data = ns.namespaceGetVariable('subStu', SType.REFERENCE);
    ASSERT_TRUE(data.isBigInt() === false);

    // 4. bigint type
    data = STValue.wrapBigInt(123456789n);
    ASSERT_TRUE(data.isBigInt() === true);

    data = STValue.wrapBigInt(BigInt('123456789'));
    ASSERT_TRUE(data.isBigInt() === true);

    data = ns.namespaceGetVariable('shouldBeBigInt', SType.REFERENCE);
    ASSERT_TRUE(data.isBigInt() === true);
}

function testIsBoolean(): void {
    let shouldBeBooleanTrue = ns.namespaceGetVariable("shouldBeBooleanTrue", SType.BOOLEAN);
    let shouldBeByte = ns.namespaceGetVariable("shouldBeByte", SType.BYTE);
    ASSERT_TRUE(shouldBeBooleanTrue.isBoolean());
    ASSERT_TRUE(!shouldBeByte.isBoolean());
}

function testIsByte(): void {
    let shouldBeByte = ns.namespaceGetVariable("shouldBeByte", SType.BYTE);
    let shouldBeBooleanTrue = ns.namespaceGetVariable("shouldBeBooleanTrue", SType.BOOLEAN);
    ASSERT_TRUE(shouldBeByte.isByte());
    ASSERT_TRUE(!shouldBeBooleanTrue.isByte());
}

function testIsChar(): void {
    let shouldBeChar = ns.namespaceGetVariable("shouldBeChar", SType.CHAR);
    let shouldBeByte = ns.namespaceGetVariable("shouldBeByte", SType.BYTE);
    ASSERT_TRUE(shouldBeChar.isChar());
    ASSERT_TRUE(!shouldBeByte.isChar());
}

function testIsShort(): void {
    let shouldBeShort = ns.namespaceGetVariable("shouldBeShort", SType.SHORT);
    let shouldBeChar = ns.namespaceGetVariable("shouldBeChar", SType.CHAR);
    ASSERT_TRUE(shouldBeShort.isShort());
    ASSERT_TRUE(!shouldBeChar.isShort());
}

function testIsInt(): void {
    let shouldBeInt = ns.namespaceGetVariable("shouldBeInt", SType.INT);
    let shouldBeShort = ns.namespaceGetVariable("shouldBeShort", SType.SHORT);
    ASSERT_TRUE(shouldBeInt.isInt());
    ASSERT_TRUE(!shouldBeShort.isInt());
}

function testIsLong(): void {
    let shouldBeLong = ns.namespaceGetVariable("shouldBeLong", SType.LONG);
    let shouldBeInt = ns.namespaceGetVariable("shouldBeInt", SType.INT);
    ASSERT_TRUE(shouldBeLong.isLong());
    ASSERT_TRUE(!shouldBeInt.isLong());
}

function testIsFloat(): void {
    let shouldBeFloat = ns.namespaceGetVariable("shouldBeFloat", SType.FLOAT);
    let shouldBeLong = ns.namespaceGetVariable("shouldBeLong", SType.LONG);
    ASSERT_TRUE(shouldBeFloat.isFloat());
    ASSERT_TRUE(!shouldBeLong.isFloat());
}

function testIsNumber(): void {
    let shouldBeNumber = ns.namespaceGetVariable("shouldBeNumber", SType.DOUBLE);
    let shouldBeFloat = ns.namespaceGetVariable("shouldBeFloat", SType.FLOAT);
    ASSERT_TRUE(shouldBeNumber.isNumber());
    ASSERT_TRUE(!shouldBeFloat.isNumber());
}

function testIsNullishValue(): void {
    let shouldBeNull = ns.namespaceGetVariable('shouldBeNull', SType.REFERENCE);
    let shouldBeUndefined = ns.namespaceGetVariable('shouldBeUndefined', SType.REFERENCE);
    let shouldBePrimitive = ns.namespaceGetVariable('shouldBePrimitive', SType.INT);
    let shouldBeRef = ns.namespaceGetVariable('shouldBeRef', SType.REFERENCE);

    ASSERT_TRUE(shouldBeNull.isNull());
    ASSERT_TRUE(!shouldBePrimitive.isNull());
    ASSERT_TRUE(!shouldBeUndefined.isNull());
    ASSERT_TRUE(!shouldBeRef.isNull());

    ASSERT_TRUE(shouldBeUndefined.isUndefined());
    ASSERT_TRUE(!shouldBeNull.isUndefined());
    ASSERT_TRUE(!shouldBePrimitive.isUndefined());
    ASSERT_TRUE(!shouldBeRef.isUndefined());

    try {
        shouldBeNull.isNull(11);
    } catch (e: Error) { }

    try {
        shouldBeUndefined.isUndefined(11);
    } catch (e: Error) { }
}

function testIsEqualTo(): void {
    let leftRef = ns.namespaceGetVariable('leftRef', SType.REFERENCE);
    ASSERT_TRUE(leftRef.isEqualTo(leftRef));
    let rightRef = ns.namespaceGetVariable('rightRef', SType.REFERENCE);
    ASSERT_TRUE(leftRef.isEqualTo(rightRef));

    let rightRefNotEqual = ns.namespaceGetVariable('rightRefNotEqual', SType.REFERENCE);
    ASSERT_TRUE(!leftRef.isEqualTo(rightRefNotEqual));

    let rightRefNotSameType = ns.namespaceGetVariable('rightRefNotSameType', SType.REFERENCE);
    ASSERT_TRUE(!leftRef.isEqualTo(rightRefNotSameType));

    let equalNull = STValue.getNull();
    let equalUndefined = STValue.getUndefined();
    ASSERT_TRUE(equalNull.isEqualTo(equalUndefined));
    ASSERT_TRUE(equalUndefined.isEqualTo(equalNull));
    ASSERT_TRUE(!equalNull.isEqualTo(rightRef));
    ASSERT_TRUE(!leftRef.isEqualTo(equalNull));
    ASSERT_TRUE(!equalUndefined.isEqualTo(rightRef));
    ASSERT_TRUE(!leftRef.isEqualTo(equalUndefined));

    try {
        leftRef.isEqualTo(rightRef, 111);
    } catch (e: Error) { }
}

function testIsStrictlyEqualTo(): void {
    let magicNull = STValue.getNull();
    let magicUndefined = STValue.getUndefined();
    let magicBoolean = ns.namespaceGetVariable('magicBoolean', SType.BOOLEAN);
    let magicByte = ns.namespaceGetVariable('magicByte', SType.BYTE);
    let magicChar = ns.namespaceGetVariable('magicChar', SType.CHAR);
    let magicShort = ns.namespaceGetVariable('magicShort', SType.SHORT);
    let magicInt = ns.namespaceGetVariable('magicInt', SType.INT);
    let magicLong = ns.namespaceGetVariable('magicLong', SType.LONG);
    let magicFloat = ns.namespaceGetVariable('magicFloat', SType.FLOAT);
    let magicDouble = ns.namespaceGetVariable('magicDouble', SType.DOUBLE);
    let magicString1 = ns.namespaceGetVariable('magicString1', SType.REFERENCE);
    let magicString2 = ns.namespaceGetVariable('magicString2', SType.REFERENCE);

    let result1 = magicNull.isStrictlyEqualTo(magicUndefined);
    ASSERT_TRUE(!result1);

    let res = false;
    try {
        magicUndefined.isStrictlyEqualTo(magicBoolean);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'other\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        magicBoolean.isStrictlyEqualTo(magicByte);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        magicByte.isStrictlyEqualTo(magicChar);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        magicChar.isStrictlyEqualTo(magicShort);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        magicShort.isStrictlyEqualTo(magicInt);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        magicInt.isStrictlyEqualTo(magicLong);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        magicLong.isStrictlyEqualTo(magicFloat);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        magicFloat.isStrictlyEqualTo(magicDouble);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        magicDouble.isStrictlyEqualTo(magicString1);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes("\'this\' STValue instance does not wrap a value of type reference");
    }
    ASSERT_TRUE(res);

    let result2 = magicString1.isStrictlyEqualTo(magicString2);
    print('Is magicString2 strictly equal to magicString1: ', result2);
    ASSERT_TRUE(!result2);

    let result3 = magicString1.isStrictlyEqualTo(magicString1);
    print('Is magicString1 strictly equal to magicString1: ', result3);
    ASSERT_TRUE(result3);
}

// objectInstanceOf(typeArg: STValue): boolean
function testObjectInstanceOf(): void {
    let stuObj = studentCls.classInstantiate(':', []);
    let isInstance = stuObj.objectInstanceOf(studentCls);
    print('isInstance:', isInstance)
    ASSERT_EQ(isInstance, true);

    let res = false;
    try {
        stuObj.objectInstanceOf(STValue.wrapNumber(123));
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('type');
        res = res && e.message.includes('reference');
        res = res && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        STValue.getUndefined().objectInstanceOf(studentCls);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('this');
        res = res && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        stuObj.objectInstanceOf(STValue.getUndefined());
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('type');
        res = res && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        stuObj.objectInstanceOf([studentCls]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('type');
        res = res && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(res);
}

// typeIsAssignableFrom(from_type: STValue, to_type: STValue): boolean
function testTypeIsAssignableFrom(): void {
    ASSERT_TRUE(STValue.typeIsAssignableFrom(subStudentCls, studentCls));
    ASSERT_TRUE(STValue.typeIsAssignableFrom(subStudentCls, subStudentCls))
    ASSERT_EQ(STValue.typeIsAssignableFrom(studentCls, subStudentCls), false)
    ASSERT_EQ(STValue.typeIsAssignableFrom(studentCls, ns), false)

    let res = false;
    try {
        STValue.typeIsAssignableFrom(STValue.wrapNumber(10), STValue.wrapBoolean(true));
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('fromType');
        res = res && e.message.includes('reference');
        res = res && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        STValue.typeIsAssignableFrom(subStudentCls, STValue.getUndefined());
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('toType STValue instance does not wrap a value of type reference');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        STValue.typeIsAssignableFrom(STValue.getNull(), STValue.getUndefined());
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('fromType STValue instance does not wrap a value of type reference');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        STValue.typeIsAssignableFrom(subStudentCls, [STValue.getUndefined()]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('toType STValue instance does not wrap a value of type reference');
    }
    ASSERT_TRUE(res);
}

function main(): void {
    testIsString();
    testIsBigInt();
    testIsBoolean();
    testIsByte();
    testIsChar();
    testIsShort();
    testIsInt();
    testIsLong();
    testIsFloat();
    testIsNumber();
    testIsNullishValue();
    testIsEqualTo();
    testIsStrictlyEqualTo();
    testTypeIsAssignableFrom();
    testObjectInstanceOf();
}

main();
