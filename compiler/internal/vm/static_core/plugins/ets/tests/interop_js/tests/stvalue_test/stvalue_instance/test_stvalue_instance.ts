/**
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

const etsVm = globalThis.gtest.etsVm;

let STValue = etsVm.STValue;
let ns = STValue.findNamespace('stvalue_instance.Instance');
let SType = etsVm.SType;

let studentCls = STValue.findClass('stvalue_instance.Student');
let subStudentCls = STValue.findClass('stvalue_instance.SubStudent');

// static newFixedArrayPrimitive(len: number, elementType: SType): STValue  boolean[]
function testNewFixedArrayPrimitive(): void {
    let checkRes = true;

    let boolArray = STValue.newFixedArrayPrimitive(5, SType.BOOLEAN);
    checkRes = checkRes && ns.namespaceInvokeFunction('checkFixPrimitiveBooleanArray', 'A{z}:z', [boolArray]).unwrapToBoolean();

    let byteArray = STValue.newFixedArrayPrimitive(5, SType.BYTE);
    checkRes = checkRes && ns.namespaceInvokeFunction('checkFixPrimitiveByteArray', 'A{b}:z', [byteArray]).unwrapToBoolean();

    let charArray = STValue.newFixedArrayPrimitive(5, SType.CHAR);
    checkRes = checkRes && ns.namespaceInvokeFunction('checkFixPrimitiveCharArray', 'A{c}:z', [charArray]).unwrapToBoolean();

    let shortArray = STValue.newFixedArrayPrimitive(5, SType.SHORT);
    checkRes = checkRes && ns.namespaceInvokeFunction('checkFixPrimitiveShortArray', 'A{s}:z', [shortArray]).unwrapToBoolean();

    let intArray = STValue.newFixedArrayPrimitive(5, SType.INT);
    checkRes = checkRes && ns.namespaceInvokeFunction('checkFixPrimitiveIntArray', 'A{i}:z', [intArray]).unwrapToBoolean();

    let longArray = STValue.newFixedArrayPrimitive(5, SType.LONG);
    checkRes = checkRes && ns.namespaceInvokeFunction('checkFixPrimitiveLongArray', 'A{l}:z', [longArray]).unwrapToBoolean();

    let floatArray = STValue.newFixedArrayPrimitive(5, SType.FLOAT);
    checkRes = checkRes && ns.namespaceInvokeFunction('checkFixPrimitiveFloatArray', 'A{f}:z', [floatArray]).unwrapToBoolean();

    let doubleArray = STValue.newFixedArrayPrimitive(5, SType.DOUBLE);
    checkRes = checkRes && ns.namespaceInvokeFunction('checkFixPrimitiveDoubleArray', 'A{d}:z', [doubleArray]).unwrapToBoolean();
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayPrimitive(-1, SType.DOUBLE);
    } catch (e: Error) {
        checkRes = e.message.includes('ANI error occurred');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayPrimitive(null, SType.DOUBLE);
    } catch (e: Error) {
        checkRes = e.message.includes('length type is not number type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayPrimitive(5, SType.REFERENCE);
    } catch (e: Error) {
        checkRes = e.message.includes('Unsupported SType: ');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayPrimitive(5, SType.VOID); // SType.STREING
    } catch (e: Error) {
        checkRes = e.message.includes('Unsupported SType: ');
    }
    ASSERT_TRUE(checkRes);
}

// static newFixedArrayReference(len: number, elementType: STValue, initialElement: STValue): STValue
function testNewFixedArrayReference(): void {
    let intClass = STValue.findClass('std.core.Int');
    let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(5)]);
    let refArray = STValue.newFixedArrayReference(3, intClass, intObj);
    let res = ns.namespaceInvokeFunction('checkFixReferenceObjectArray', 'A{C{std.core.Object}}:z', [refArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let undefArray = STValue.newFixedArrayReference(3, intClass, STValue.getUndefined());
    res = ns.namespaceInvokeFunction('checkFixReferenceUndefinedArray', 'A{C{std.core.Object}}:z', [undefArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let strClass = STValue.findClass('std.core.String');
    let strArray = STValue.newFixedArrayReference(3, strClass, STValue.wrapString('hello world!'));
    res = ns.namespaceInvokeFunction('checkFixReferenceStringArray', 'A{C{std.core.String}}:z', [strArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let checkRes = false;
    try {
        STValue.newFixedArrayReference(-3, intClass, intObj);
    } catch (e: Error) {
        checkRes = e.message.includes('ANI error occurred');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayReference(999, intClass, [STValue.getNull()]);
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('initialElement');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayReference(999, intClass, STValue.wrapInt(123));
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('initialElement');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newFixedArrayReference(999, STValue.getUndefined(), intObj);
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('elementType');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
        checkRes = checkRes && e.message.includes('reference');
    }
    ASSERT_TRUE(checkRes);
}

function testNewArray(): void {
    let boolClass = STValue.findClass('std.core.Boolean');
    let boolObj = boolClass.classInstantiate('z:', [STValue.wrapBoolean(false)]);
    let boolArray = STValue.newArray(5, boolObj);
    let res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [boolArray, boolObj]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let byteClass = STValue.findClass('std.core.Byte');
    let byteObj = byteClass.classInstantiate('b:', [STValue.wrapByte(0x02)]);
    let byteArray = STValue.newArray(5, byteObj);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [byteArray, byteObj]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let charClass = STValue.findClass('std.core.Char');
    let charObj = charClass.classInstantiate('c:', [STValue.wrapChar('a')]);
    let charArray = STValue.newArray(5, charObj);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [charArray, charObj]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let shortClass = STValue.findClass('std.core.Short');
    let shortObj = shortClass.classInstantiate('s:', [STValue.wrapShort(42)]);
    let shortArray = STValue.newArray(5, shortObj);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [shortArray, shortObj]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let intClass = STValue.findClass('std.core.Int');
    let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(0)]);
    let intArray = STValue.newArray(5, intObj);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [intArray, intObj]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let longClass = STValue.findClass('std.core.Long');
    let longObj = longClass.classInstantiate('l:', [STValue.wrapLong(1000000001)]);
    let longArray = STValue.newArray(5, longObj);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [longArray, longObj]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let floatClass = STValue.findClass('std.core.Float');
    let floatObj = floatClass.classInstantiate('f:', [STValue.wrapFloat(3.14)]);
    let floatArray = STValue.newArray(5, floatObj);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [floatArray, floatObj]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let doubleClass = STValue.findClass('std.core.Double');
    let doubleObj = doubleClass.classInstantiate('d:', [STValue.wrapNumber(3.1415926)]);
    let doubleArray = STValue.newArray(5, doubleObj);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [doubleArray, doubleObj]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let undefArray = STValue.newArray(5, STValue.getUndefined());
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [undefArray, STValue.getUndefined()]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let strArray = STValue.newArray(5, STValue.wrapString('hello world!'));
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [strArray, STValue.wrapString('hello world!')]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let emptyArray = STValue.newArray(0, STValue.getUndefined());
    ASSERT_TRUE(emptyArray.arrayGetLength() === 0);

    let checkRes = false;
    try {
        STValue.newArray(999, [STValue.getNull()]);
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('initialElement');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newArray(999, STValue.wrapInt(123));
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('initialElement');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        STValue.newArray(-5, intObj);
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('length must be non-negative');
    }
    ASSERT_TRUE(checkRes);
}

function testNewTwoDimensionalArray(): void {
    let boolClass = STValue.findClass('std.core.Boolean');
    let boolObj = boolClass.classInstantiate('z:', [STValue.wrapBoolean(false)]);
    let boolArray = STValue.newArray(5, boolObj);
    let twoDimBoolArray = STValue.newArray(5, boolArray);
    let res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [twoDimBoolArray, boolArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let byteClass = STValue.findClass('std.core.Byte');
    let byteObj = byteClass.classInstantiate('b:', [STValue.wrapByte(0x02)]);
    let byteArray = STValue.newArray(5, byteObj);
    let twoDimByteArray = STValue.newArray(5, byteArray);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [twoDimByteArray, byteArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let charClass = STValue.findClass('std.core.Char');
    let charObj = charClass.classInstantiate('c:', [STValue.wrapChar('a')]);
    let charArray = STValue.newArray(5, charObj);
    let twoDimCharArray = STValue.newArray(5, charArray);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [twoDimCharArray, charArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let shortClass = STValue.findClass('std.core.Short');
    let shortObj = shortClass.classInstantiate('s:', [STValue.wrapShort(42)]);
    let shortArray = STValue.newArray(5, shortObj);
    let twoDimShortArray = STValue.newArray(5, shortArray);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [twoDimShortArray, shortArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let intClass = STValue.findClass('std.core.Int');
    let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(0)]);
    let intArray = STValue.newArray(5, intObj);
    let twoDimIntArray = STValue.newArray(5, intArray);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [twoDimIntArray, intArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let longClass = STValue.findClass('std.core.Long');
    let longObj = longClass.classInstantiate('l:', [STValue.wrapLong(1000000001)]);
    let longArray = STValue.newArray(5, longObj);
    let twoDimLongArray = STValue.newArray(5, longArray);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [twoDimLongArray, longArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let floatClass = STValue.findClass('std.core.Float');
    let floatObj = floatClass.classInstantiate('f:', [STValue.wrapFloat(3.14)]);
    let floatArray = STValue.newArray(5, floatObj);
    let twoDimFloatArray = STValue.newArray(5, floatArray);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [twoDimFloatArray, floatArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let doubleClass = STValue.findClass('std.core.Double');
    let doubleObj = doubleClass.classInstantiate('d:', [STValue.wrapNumber(3.1415926)]);
    let doubleArray = STValue.newArray(5, doubleObj);
    let twoDimDoubleArray = STValue.newArray(5, doubleArray);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [twoDimDoubleArray, doubleArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let undefArray = STValue.newArray(5, STValue.getUndefined());
    let twoDimUndefArray = STValue.newArray(5, undefArray);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [twoDimUndefArray, undefArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let strArray = STValue.newArray(5, STValue.wrapString('hello world!'));
    let twoDimStrArray = STValue.newArray(5, strArray);
    res = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [twoDimStrArray, strArray]);
    ASSERT_TRUE(res.unwrapToBoolean());

    let invalidRes = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [twoDimIntArray, intObj]);
    ASSERT_TRUE(!invalidRes.unwrapToBoolean());

    invalidRes = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [twoDimIntArray, shortArray]);
    ASSERT_TRUE(!invalidRes.unwrapToBoolean());

    invalidRes = ns.namespaceInvokeFunction('checkArray', 'C{std.core.Array}C{std.core.Object}:z', [intArray, intArray]);
    ASSERT_TRUE(!invalidRes.unwrapToBoolean());
}

// classFull.classInstantiateImpl(ctorSinature, array<Stvalue>): STValue
function testClassInstantiate(): void {
    let clsObj1 = studentCls.classInstantiate(':', []);
    let obj1Age = clsObj1.objectGetProperty('age', SType.INT).unwrapToNumber();
    let clsObj2 = studentCls.classInstantiate('i:', [STValue.wrapInt(23)]);
    let obj2Age = clsObj2.objectGetProperty('age', SType.INT).unwrapToNumber();
    ASSERT_EQ(obj1Age, 0);
    ASSERT_EQ(obj2Age, 23);

    let res = false;
    try {
        studentCls.classInstantiate('i:i', []);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('ANI error occurred');
        res = res && e.message.includes('status: 7');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        studentCls.classInstantiate(':', STValue.wrapNumber(1));
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('param are not array type');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        STValue.getUndefined().classInstantiate('i:', [STValue.wrapString('123')]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('STValue instance does not wrap a value of type reference');
    }
    ASSERT_TRUE(res);
}

function main(): void {
    testNewFixedArrayPrimitive();
    testNewFixedArrayReference();
    testNewArray();
    testNewTwoDimensionalArray();
    testClassInstantiate();
}

main();
