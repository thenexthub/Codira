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
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace')
let ns = STValue.findNamespace('stvalue_accessor.magicNamespace');

let SType = etsVm.SType;
let colorEnum = STValue.findEnum('stvalue_accessor.COLOR');
let mediaEnum = STValue.findEnum('stvalue_accessor.MEDIA');
let zeroFixedArray = tns.namespaceGetVariable('zeroFixedArray', SType.REFERENCE);
let intFixedArray = tns.namespaceGetVariable('intFixedArray', SType.REFERENCE);
let boolFixedArray = tns.namespaceGetVariable('boolFixedArray', SType.REFERENCE);
let strFixedArray = tns.namespaceGetVariable('strFixedArray', SType.REFERENCE);
let stuFixedArray = tns.namespaceGetVariable('stuFixedArray', SType.REFERENCE);
let zeroArray = tns.namespaceGetVariable('zeroArray', SType.REFERENCE);
let intArray = tns.namespaceGetVariable('intArray', SType.REFERENCE);
let boolArray = tns.namespaceGetVariable('boolArray', SType.REFERENCE);
let byteArray = tns.namespaceGetVariable('byteArray', SType.REFERENCE);
let charArray = tns.namespaceGetVariable('charArray', SType.REFERENCE);
let shortArray = tns.namespaceGetVariable('shortArray', SType.REFERENCE);
let longArray = tns.namespaceGetVariable('longArray', SType.REFERENCE);
let floatArray = tns.namespaceGetVariable('floatArray', SType.REFERENCE);
let doubleArray = tns.namespaceGetVariable('doubleArray', SType.REFERENCE);
let strArray = tns.namespaceGetVariable('strArray', SType.REFERENCE);
let stuArray = tns.namespaceGetVariable('stuArray', SType.REFERENCE);
let studentCls = STValue.findClass('stvalue_accessor.Student');

function testFixedArrayGetLength(): void {
    ASSERT_TRUE(zeroFixedArray.fixedArrayGetLength() === 0);
    ASSERT_TRUE(strFixedArray.fixedArrayGetLength() === 3);
    ASSERT_TRUE(boolFixedArray.fixedArrayGetLength() === 4);
    ASSERT_TRUE(intFixedArray.fixedArrayGetLength() === 5);

    try {
        ASSERT_TRUE(colorEnum.fixedArrayGetLength() === 1);
    } catch (e: Error) { }

    try {
        strFixedArray.fixedArrayGetLength(11);
    } catch (e: Error) { }
}

function testArrayGetLength(): void {
    ASSERT_TRUE(zeroArray.arrayGetLength() === 0);
    ASSERT_TRUE(intArray.arrayGetLength() === 5);
    ASSERT_TRUE(boolArray.arrayGetLength() === 4);
    ASSERT_TRUE(strArray.arrayGetLength() === 3);
    ASSERT_TRUE(stuArray.arrayGetLength() === 1);

    let checkRes = false;
    try {
        ASSERT_TRUE(intArray.arrayGetLength(5) === 5);
    } catch (e: Error) {
        checkRes = e.message.includes('Expect 0 args, but got 1 args');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        let nullArray = STValue.getNull();
        nullArray.arrayGetLength();
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        let undefArray = STValue.getUndefined();
        undefArray.arrayGetLength();
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);
}

function testEnumAll(): void {
    ASSERT_TRUE(colorEnum.enumGetIndexByName('Red') === 0);
    ASSERT_TRUE(colorEnum.enumGetIndexByName('Green') === 1);
    ASSERT_TRUE(colorEnum.enumGetIndexByName('Blue') === 2);
    ASSERT_TRUE(colorEnum.enumGetIndexByName('Yellow') === 3);
    ASSERT_TRUE(mediaEnum.enumGetIndexByName('YAML') === 2);

    ASSERT_TRUE(colorEnum.enumGetValueByName('Red', SType.INT).unwrapToNumber() === 1);
    ASSERT_TRUE(mediaEnum.enumGetValueByName('XML', SType.REFERENCE).unwrapToString() === 'app/xml');

    try {
        colorEnum.enumGetIndexByName('Black', SType.INT);
    } catch (e: Error) { }

    try {
        colorEnum.enumGetIndexByName('Black');
    } catch (e: Error) { }

    try {
        colorEnum.enumGetValueByName('Red', SType.REFERENCE);
    } catch (e: Error) { }

    try {
        let nullEmun = STValue.getNull();
        nullEnum.enumGetValueByName('Red', SType.REFERENCE);
    } catch (e: Error) { }
}

function testClassGetAndSetStaticField(): void {
    let personClass = STValue.findClass('stvalue_accessor.Person');
    ASSERT_TRUE(personClass.classGetStaticField('name', SType.REFERENCE).unwrapToString() === 'Person');
    ASSERT_TRUE(personClass.classGetStaticField('age', SType.INT).unwrapToNumber() === 18);
    ASSERT_TRUE(personClass.classGetStaticField('male', SType.BOOLEAN).unwrapToBoolean() === true);
    ASSERT_TRUE(personClass.classGetStaticField('id', SType.LONG).unwrapToNumber() === 1000000000);
    ASSERT_TRUE(personClass.classGetStaticField('bits', SType.BYTE).unwrapToNumber() === 0x01);
    ASSERT_TRUE(personClass.classGetStaticField('height', SType.DOUBLE).unwrapToNumber() === 1.78);

    personClass.classSetStaticField('name', STValue.wrapString('Bob'), SType.REFERENCE);
    personClass.classSetStaticField('height', STValue.wrapNumber(1.79), SType.DOUBLE);
    personClass.classSetStaticField('id', STValue.wrapLong(20000000001), SType.LONG);
    personClass.classSetStaticField('age', STValue.wrapInt(28), SType.INT);
    personClass.classSetStaticField('bits', STValue.wrapByte(0x02), SType.BYTE);
    personClass.classSetStaticField('male', STValue.wrapBoolean(false), SType.BOOLEAN);

    ASSERT_TRUE(personClass.classGetStaticField('name', SType.REFERENCE).unwrapToString() === 'Bob');
    ASSERT_TRUE(personClass.classGetStaticField('age', SType.INT).unwrapToNumber() === 28);
    ASSERT_TRUE(personClass.classGetStaticField('male', SType.BOOLEAN).unwrapToBoolean() === false);
    ASSERT_TRUE(personClass.classGetStaticField('bits', SType.BYTE).unwrapToNumber() === 0x02);
    ASSERT_TRUE(personClass.classGetStaticField('height', SType.DOUBLE).unwrapToNumber() === 1.79);
    ASSERT_TRUE(personClass.classGetStaticField('id', SType.LONG).unwrapToNumber() === 20000000001);

    try {
        personClass.classGetStaticField('xx', SType.INT);
    } catch (e: Error) { }
    try {
        personClass.classGetStaticField('male', SType.REFERENCE);
    } catch (e: Error) { }

    try {
        personClass.classSetStaticField('male', STValue.wrapBoolean(false), SType.INT);
    } catch (e: Error) { }

    try {
        personClass.classSetStaticField('male', STValue.wrapNumber(11), SType.BOOLEAN);
    } catch (e: Error) { }

    try {
        let nonClass = STValue.wrapInt(111);
        nonClass.classGetStaticField('male', SType.BOOLEAN);
    } catch (e: Error) { }

    try {
        personClass.classSetStaticField('pet', STValue.getNull(), SType.REFERENCE);
        ASSERT_TRUE(personClass.classGetStaticField('pet', SType.REFERENCE).isNull());
    } catch (e: Error) { }

    try {
        let nullClass = STValue.getNull();
        nullClass.classGetStaticField('male', SType.BOOLEAN);
    } catch (e: Error) { }

    try {
        personClass.classGetStaticField('male', SType.REFERENCE, STValue.getNull());
    } catch (e: Error) { }
}

function testObjectGetAndSetProperty(): void {
    let studentInstance = tns.namespaceGetVariable('student', SType.REFERENCE);
    ASSERT_TRUE(studentInstance.objectGetProperty('name', SType.REFERENCE).unwrapToString() === 'Student');
    ASSERT_TRUE(studentInstance.objectGetProperty('age', SType.INT).unwrapToNumber() === 18);
    ASSERT_TRUE(studentInstance.objectGetProperty('male', SType.BOOLEAN).unwrapToBoolean() === true);
    ASSERT_TRUE(studentInstance.objectGetProperty('bits', SType.BYTE).unwrapToNumber() === 0x01);
    ASSERT_TRUE(studentInstance.objectGetProperty('id', SType.LONG).unwrapToNumber() === 1000000000);
    ASSERT_TRUE(studentInstance.objectGetProperty('height', SType.DOUBLE).unwrapToNumber() === 1.78);
    ASSERT_TRUE(studentInstance.objectGetProperty('isChild', SType.BOOLEAN).unwrapToBoolean() === true);

    studentInstance.objectSetProperty('name', STValue.wrapString('Ark'), SType.REFERENCE);
    studentInstance.objectSetProperty('age', STValue.wrapInt(28), SType.INT);
    studentInstance.objectSetProperty('male', STValue.wrapBoolean(false), SType.BOOLEAN);
    studentInstance.objectSetProperty('id', STValue.wrapLong(1000000001), SType.LONG);
    studentInstance.objectSetProperty('bits', STValue.wrapByte(0x02), SType.BYTE);
    studentInstance.objectSetProperty('height', STValue.wrapNumber(1.75), SType.DOUBLE);
    studentInstance.objectSetProperty('isChild', STValue.wrapBoolean(false), SType.BOOLEAN);

    ASSERT_TRUE(studentInstance.objectGetProperty('name', SType.REFERENCE).unwrapToString() === 'Ark');
    ASSERT_TRUE(studentInstance.objectGetProperty('age', SType.INT).unwrapToNumber() === 28);
    ASSERT_TRUE(studentInstance.objectGetProperty('male', SType.BOOLEAN).unwrapToBoolean() === false);
    ASSERT_TRUE(studentInstance.objectGetProperty('bits', SType.BYTE).unwrapToNumber() === 0x02);
    ASSERT_TRUE(studentInstance.objectGetProperty('id', SType.LONG).unwrapToNumber() === 1000000001);
    ASSERT_TRUE(studentInstance.objectGetProperty('height', SType.DOUBLE).unwrapToNumber() === 1.75);
    ASSERT_TRUE(studentInstance.objectGetProperty('isChild', SType.BOOLEAN).unwrapToBoolean() === false);

    studentInstance.objectSetProperty('name', STValue.getNull(), SType.REFERENCE);
    ASSERT_TRUE(studentInstance.objectGetProperty('name', SType.REFERENCE).isNull());
    try {
        studentInstance.objectGetProperty('name', SType.INT);
    } catch (e: Error) { }

    try {
        studentInstance.objectGetProperty('xxx', SType.INT);
    } catch (e: Error) { }

    try {
        studentInstance.objectSetProperty('name', STValue.wrapNumber(111), SType.REFERENCE);
    } catch (e: Error) { }

    try {
        studentInstance.objectSetProperty('name', STValue.wrapString('Ark'), SType.INT);
    } catch (e: Error) { }

    try {
        let priObject = STValue.wrapInt(111);
        priObject.objectGetProperty('name', SType.REFERENCE);
    } catch (e: Error) { }

    try {
        let nullObject = STValue.getNull();
        nullObject.objectGetProperty('name', SType.REFERENCE);
    } catch (e: Error) { }

    try {
        studentInstance.objectSetProperty('name', STValue.wrapString('Ark'));
    } catch (e: Error) { }
}

function testFixedArrayGetAndSet(): void {
    ASSERT_TRUE(intFixedArray.fixedArrayGet(0, SType.INT).unwrapToNumber() === 1);
    ASSERT_TRUE(boolFixedArray.fixedArrayGet(3, SType.BOOLEAN).unwrapToBoolean() === false);
    ASSERT_TRUE(strFixedArray.fixedArrayGet(1, SType.REFERENCE).unwrapToString() === 'cd');
    ASSERT_TRUE(stuFixedArray.fixedArrayGet(0, SType.REFERENCE).objectGetProperty('age', SType.INT).unwrapToNumber() === 28);

    intFixedArray.fixedArraySet(0, STValue.wrapInt(10), SType.INT);
    boolFixedArray.fixedArraySet(3, STValue.wrapBoolean(true), SType.BOOLEAN);
    strFixedArray.fixedArraySet(1, STValue.wrapString('xy'), SType.REFERENCE);

    ASSERT_TRUE(intFixedArray.fixedArrayGet(0, SType.INT).unwrapToNumber() === 10);
    ASSERT_TRUE(boolFixedArray.fixedArrayGet(3, SType.BOOLEAN).unwrapToBoolean() === true);

    try {
        strFixedArray.fixedArraySet(0, STValue.getNull(), SType.REFERENCE);
        ASSERT_TRUE(strFixedArray.fixedArrayGet(0, SType.REFERENCE).isNull());
    } catch (e: Error) { }

    try {
        intFixedArray.fixedArrayGet(5, SType.INT);
    } catch (e: Error) { }

    try {
        intFixedArray.fixedArrayGet(-1, SType.INT);
    } catch (e: Error) { }

    try {
        let nullArray = STValue.getNull();
        nullArray.fixedArraySet(0, STValue.wrapNumber(111), SType.FLOAT);
    } catch (e: Error) { }

    try {
        intFixedArray.fixedArraySet(0, STValue.wrapNumber(111), SType.FLOAT);
    } catch (e: Error) { }

    try {
        intFixedArray.fixedArraySet(0, STValue.wrapString('11'), SType.INT);
    } catch (e: Error) { }

    try {
        colorEnum.fixedArraySet(0, STValue.wrapNumber(111), SType.INT);
    } catch (e: Error) { }

    try {
        boolFixedArray.fixedArrayGet(3, SType.BOOLEAN, STValue.wrapNumber(111));
    } catch (e: Error) { }

}

function testArrayGetAndSet(): void {
    ASSERT_TRUE(intArray.arrayGet(0).objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 1);
    ASSERT_TRUE(boolArray.arrayGet(3).objectInvokeMethod('toBoolean', ':z', []).unwrapToBoolean() === false);
    ASSERT_TRUE(byteArray.arrayGet(1).objectInvokeMethod('toByte', ':b', []).unwrapToNumber() === 0x02);
    ASSERT_TRUE(charArray.arrayGet(1).objectInvokeMethod('toChar', ':c', []).unwrapToNumber() === 98);
    ASSERT_TRUE(shortArray.arrayGet(1).objectInvokeMethod('toShort', ':s', []).unwrapToNumber() === 200);
    ASSERT_TRUE(longArray.arrayGet(1).objectInvokeMethod('toLong', ':l', []).unwrapToNumber() === 2000000000);
    ASSERT_TRUE(Math.abs(floatArray.arrayGet(1).objectInvokeMethod('toFloat', ':f', []).unwrapToNumber() - 2.718) < 0.001);
    ASSERT_TRUE(Math.abs(doubleArray.arrayGet(1).objectInvokeMethod('toDouble', ':d', []).unwrapToNumber() - 2.718281) < 0.000001);
    ASSERT_TRUE(strArray.arrayGet(1).objectInvokeMethod('toString', ':C{std.core.String}', []).unwrapToString() === 'cd');
    ASSERT_TRUE(stuArray.arrayGet(0).objectGetProperty('age', SType.INT).unwrapToNumber() === 28);

    let intClass = STValue.findClass('std.core.Int');
    let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(10)]);
    intArray.arraySet(0, intObj);

    let boolClass = STValue.findClass('std.core.Boolean');
    let boolObj = boolClass.classInstantiate('z:', [STValue.wrapBoolean(true)]);
    boolArray.arraySet(3, boolObj);

    let byteClass = STValue.findClass('std.core.Byte');
    let byteObj = byteClass.classInstantiate('b:', [STValue.wrapByte(0x04)]);
    byteArray.arraySet(1, byteObj);

    let charClass = STValue.findClass('std.core.Char');
    let charObj = charClass.classInstantiate('c:', [STValue.wrapChar('d')]);
    charArray.arraySet(1, charObj);
    
    let shortClass = STValue.findClass('std.core.Short');
    let shortObj = shortClass.classInstantiate('s:', [STValue.wrapShort(300)]);
    shortArray.arraySet(1, shortObj);

    let longClass = STValue.findClass('std.core.Long');
    let longObj = longClass.classInstantiate('l:', [STValue.wrapLong(3000000000)]);
    longArray.arraySet(1, longObj);

    let floatClass = STValue.findClass('std.core.Float');
    let floatObj = floatClass.classInstantiate('f:', [STValue.wrapFloat(2.719)]);
    floatArray.arraySet(1, floatObj);

    let doubleClass = STValue.findClass('std.core.Double');
    let doubleObj = doubleClass.classInstantiate('d:', [STValue.wrapNumber(2.718282)]);
    doubleArray.arraySet(1, doubleObj);
    
    strArray.arraySet(1, STValue.wrapString('xy'));

    ASSERT_TRUE(intArray.arrayGet(0).objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 10);
    ASSERT_TRUE(boolArray.arrayGet(3).objectInvokeMethod('toBoolean', ':z', []).unwrapToBoolean() === true);
    ASSERT_TRUE(byteArray.arrayGet(1).objectInvokeMethod('toByte', ':b', []).unwrapToNumber() === 0x04);
    ASSERT_TRUE(charArray.arrayGet(1).objectInvokeMethod('toChar', ':c', []).unwrapToNumber() === 100);
    ASSERT_TRUE(shortArray.arrayGet(1).objectInvokeMethod('toShort', ':s', []).unwrapToNumber() === 300);
    ASSERT_TRUE(longArray.arrayGet(1).objectInvokeMethod('toLong', ':l', []).unwrapToNumber() === 3000000000);
    ASSERT_TRUE(Math.abs(floatArray.arrayGet(1).objectInvokeMethod('toFloat', ':f', []).unwrapToNumber() - 2.719) < 0.001);
    ASSERT_TRUE(Math.abs(doubleArray.arrayGet(1).objectInvokeMethod('toDouble', ':d', []).unwrapToNumber() - 2.718282) < 0.000001);
    ASSERT_TRUE(strArray.arrayGet(1).objectInvokeMethod('toString', ':C{std.core.String}', []).unwrapToString() === 'xy');

    strArray.arraySet(0, STValue.wrapString(''));
    ASSERT_TRUE(strArray.arrayGet(0).objectInvokeMethod('toString', ':C{std.core.String}', []).unwrapToString() === '');
}

function testArrayGetAndSetInvailidParam(): void {
    let checkRes = false;
    try {
        intArray.arrayGet(5);
    } catch (e: Error) {
        checkRes = e.message.includes('ANI error occurred');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        intArray.arrayGet(-1);
    } catch (e: Error) {
        checkRes = e.message.includes('ANI error occurred');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        intArray.arrayGet(0, SType.INT);
    } catch (e: Error) {
        checkRes = e.message.includes('Expect 1 args, but got 2 args');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        let undefinedArray = STValue.getUndefined();
        undefinedArray.arrayGet(0);
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        let nullArray = STValue.getNull();
        nullArray.arrayGet(0);
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    let intClass = STValue.findClass('std.core.Int');
    let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(10)]);
    
    checkRes = false;
    try {
        intArray.arraySet(-1, intObj);
    } catch (e: Error) {
        checkRes = e.message.includes('ANI error occurred');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        intArray.arraySet(5, intObj);
    } catch (e: Error) {
        checkRes = e.message.includes('ANI error occurred');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        intArray.arraySet(0, STValue.wrapNumber(111));
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('value');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        let undefinedArray = STValue.getUndefined();
        undefinedArray.arraySet(0, STValue.wrapString('xy'));
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        let nullArray = STValue.getNull();
        nullArray.arraySet(0, STValue.wrapString('xy'));
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        strArray.arraySet(0, STValue.getNull());
        ASSERT_TRUE(strArray.arrayGet(0).objectInvokeMethod('toString', ':C{std.core.String}', []).isNull());
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);
}

function testArrayPushAndPop(): void {
    let intClass = STValue.findClass('std.core.Int');
    let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(10)]);

    let boolClass = STValue.findClass('std.core.Boolean');
    let boolObj = boolClass.classInstantiate('z:', [STValue.wrapBoolean(true)]);
    
    let byteClass = STValue.findClass('std.core.Byte');
    let byteObj = byteClass.classInstantiate('b:', [STValue.wrapByte(0x04)]);

    let charClass = STValue.findClass('std.core.Char');
    let charObj = charClass.classInstantiate('c:', [STValue.wrapChar('d')]);
    
    let shortClass = STValue.findClass('std.core.Short');
    let shortObj = shortClass.classInstantiate('s:', [STValue.wrapShort(300)]);

    let longClass = STValue.findClass('std.core.Long');
    let longObj = longClass.classInstantiate('l:', [STValue.wrapLong(3000000000)]);

    let floatClass = STValue.findClass('std.core.Float');
    let floatObj = floatClass.classInstantiate('f:', [STValue.wrapFloat(2.719)]);

    let doubleClass = STValue.findClass('std.core.Double');
    let doubleObj = doubleClass.classInstantiate('d:', [STValue.wrapNumber(2.718282)]);

    intArray.arrayPush(intObj);
    boolArray.arrayPush(boolObj);
    byteArray.arrayPush(byteObj);
    charArray.arrayPush(charObj);
    shortArray.arrayPush(shortObj);
    longArray.arrayPush(longObj);
    floatArray.arrayPush(floatObj);
    doubleArray.arrayPush(doubleObj);
    strArray.arrayPush(STValue.wrapString('gh'));

    ASSERT_TRUE(intArray.arrayGetLength() === 6);
    ASSERT_TRUE(boolArray.arrayGetLength() === 5);
    ASSERT_TRUE(byteArray.arrayGetLength() === 4);
    ASSERT_TRUE(charArray.arrayGetLength() === 4);
    ASSERT_TRUE(shortArray.arrayGetLength() === 3);
    ASSERT_TRUE(longArray.arrayGetLength() === 3);
    ASSERT_TRUE(floatArray.arrayGetLength() === 3);
    ASSERT_TRUE(doubleArray.arrayGetLength() === 3);
    ASSERT_TRUE(strArray.arrayGetLength() === 4);

    ASSERT_TRUE(intArray.arrayGet(5).objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 10);
    ASSERT_TRUE(boolArray.arrayGet(4).objectInvokeMethod('toBoolean', ':z', []).unwrapToBoolean() === true);
    ASSERT_TRUE(byteArray.arrayGet(3).objectInvokeMethod('toByte', ':b', []).unwrapToNumber() === 0x04);
    ASSERT_TRUE(charArray.arrayGet(3).objectInvokeMethod('toChar', ':c', []).unwrapToNumber() === 100);
    ASSERT_TRUE(shortArray.arrayGet(2).objectInvokeMethod('toShort', ':s', []).unwrapToNumber() === 300);
    ASSERT_TRUE(longArray.arrayGet(2).objectInvokeMethod('toLong', ':l', []).unwrapToNumber() === 3000000000);
    ASSERT_TRUE(Math.abs(floatArray.arrayGet(2).objectInvokeMethod('toFloat', ':f', []).unwrapToNumber() - 2.719) < 0.001);
    ASSERT_TRUE(Math.abs(doubleArray.arrayGet(2).objectInvokeMethod('toDouble', ':d', []).unwrapToNumber() - 2.718282) < 0.000001);
    ASSERT_TRUE(strArray.arrayGet(3).objectInvokeMethod('toString', ':C{std.core.String}', []).unwrapToString() === 'gh');
    
    ASSERT_TRUE(intArray.arrayPop().objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 10);
    ASSERT_TRUE(boolArray.arrayPop().objectInvokeMethod('toBoolean', ':z', []).unwrapToBoolean() === true);
    ASSERT_TRUE(byteArray.arrayPop().objectInvokeMethod('toByte', ':b', []).unwrapToNumber() === 0x04);
    ASSERT_TRUE(charArray.arrayPop().objectInvokeMethod('toChar', ':c', []).unwrapToNumber() === 100);
    ASSERT_TRUE(shortArray.arrayPop().objectInvokeMethod('toShort', ':s', []).unwrapToNumber() === 300);
    ASSERT_TRUE(longArray.arrayPop().objectInvokeMethod('toLong', ':l', []).unwrapToNumber() === 3000000000);
    ASSERT_TRUE(Math.abs(floatArray.arrayPop().objectInvokeMethod('toFloat', ':f', []).unwrapToNumber() - 2.719) < 0.001);
    ASSERT_TRUE(Math.abs(doubleArray.arrayPop().objectInvokeMethod('toDouble', ':d', []).unwrapToNumber() - 2.718282) < 0.000001);
    ASSERT_TRUE(strArray.arrayPop().objectInvokeMethod('toString', ':C{std.core.String}', []).unwrapToString() === 'gh');
    
    ASSERT_TRUE(intArray.arrayGetLength() === 5);
    ASSERT_TRUE(boolArray.arrayGetLength() === 4);
    ASSERT_TRUE(byteArray.arrayGetLength() === 3);
    ASSERT_TRUE(charArray.arrayGetLength() === 3);
    ASSERT_TRUE(shortArray.arrayGetLength() === 2);
    ASSERT_TRUE(longArray.arrayGetLength() === 2);
    ASSERT_TRUE(floatArray.arrayGetLength() === 2);
    ASSERT_TRUE(doubleArray.arrayGetLength() === 2);
    ASSERT_TRUE(strArray.arrayGetLength() === 3);

    let intObj1 = intClass.classInstantiate('i:', [STValue.wrapInt(1)]);
    let intObj2 = intClass.classInstantiate('i:', [STValue.wrapInt(2)]);
    let intObj3 = intClass.classInstantiate('i:', [STValue.wrapInt(3)]);
    let intObj4 = intClass.classInstantiate('i:', [STValue.wrapInt(4)]);

    let intLIFOArray = STValue.newArray(1, intObj1);
    intLIFOArray.arrayPush(intObj2);
    intLIFOArray.arrayPush(intObj3);
    intLIFOArray.arrayPush(intObj4);

    ASSERT_TRUE(intLIFOArray.arrayPop().objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 4);
    ASSERT_TRUE(intLIFOArray.arrayPop().objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 3);
    ASSERT_TRUE(intLIFOArray.arrayPop().objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 2);
    ASSERT_TRUE(intLIFOArray.arrayPop().objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 1);
}

function testArrayPushAndPopInvailidParam(): void {
    let intClass = STValue.findClass('std.core.Int');
    let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(10)]);

    let checkRes = false;
    try {
        intArray.arrayPush();
    } catch (e: Error) {
        checkRes = e.message.includes('Expect 1 args, but got 0 args');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        intArray.arrayPush(intObj, intObj);
    } catch (e: Error) {
        checkRes = e.message.includes('Expect 1 args, but got 2 args');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        intArray.arrayPush(STValue.wrapNumber(111));
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('value');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        let nullArray = STValue.getNull();
        nullArray.arrayPush(intObj);
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        let undefinedArray = STValue.getUndefined();
        undefinedArray.arrayPush(intObj);
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        let nullArray = STValue.getNull();
        nullArray.arrayPush(intObj);
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        strArray.arrayPush(STValue.getNull());
        ASSERT_TRUE(strArray.arrayGet(3).objectInvokeMethod('toString', ':C{std.core.String}', []).isNull());
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        intArray.arrayPop(intObj);
    } catch (e: Error) {
        checkRes = e.message.includes('Expect 0 args, but got 1 args');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        let nullArray = STValue.getNull();
        nullArray.arrayPop();
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    checkRes = false;
    try {
        let undefinedArray = STValue.getUndefined();
        undefinedArray.arrayPop();
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('reference');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);

    while (intArray.arrayGetLength() > 0) {
        intArray.arrayPop();
    }
    ASSERT_TRUE(intArray.arrayGetLength() === 0);
    ASSERT_TRUE(intArray.arrayPop().isUndefined());
}

function testNamespaceVariable(): void {
    ASSERT_TRUE(ns.namespaceGetVariable('magic_int_n', SType.INT).unwrapToNumber() === 42);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_boolean_n', SType.BOOLEAN).unwrapToBoolean() === true);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_byte_n', SType.BYTE).unwrapToNumber() === 72);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_short_n', SType.SHORT).unwrapToNumber() === 128);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_long_n', SType.LONG).unwrapToNumber() === 1024);
    ASSERT_TRUE(Math.abs(ns.namespaceGetVariable('magic_float_n', SType.FLOAT).unwrapToNumber() - 3.14) < 0.01);
    ASSERT_TRUE(Math.abs(ns.namespaceGetVariable('magic_double_n', SType.DOUBLE).unwrapToNumber() - 3.141592) < 0.000001);

    ns.namespaceSetVariable('magic_int_n', STValue.wrapInt(44), SType.INT);
    ns.namespaceSetVariable('magic_boolean_n', STValue.wrapBoolean(false), SType.BOOLEAN);
    ns.namespaceSetVariable('magic_byte_n', STValue.wrapByte(74), SType.BYTE);
    ns.namespaceSetVariable('magic_short_n', STValue.wrapShort(124), SType.SHORT);
    ns.namespaceSetVariable('magic_long_n', STValue.wrapLong(1020), SType.LONG);
    ns.namespaceSetVariable('magic_float_n', STValue.wrapFloat(3.15), SType.FLOAT);
    ns.namespaceSetVariable('magic_double_n', STValue.wrapNumber(3.141593), SType.DOUBLE);

    ASSERT_TRUE(ns.namespaceGetVariable('magic_int_n', SType.INT).unwrapToNumber() === 44);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_boolean_n', SType.BOOLEAN).unwrapToBoolean() === false);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_byte_n', SType.BYTE).unwrapToNumber() === 74);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_short_n', SType.SHORT).unwrapToNumber() === 124);
    ASSERT_TRUE(ns.namespaceGetVariable('magic_long_n', SType.LONG).unwrapToNumber() === 1020);
    ASSERT_TRUE(Math.abs(ns.namespaceGetVariable('magic_float_n', SType.FLOAT).unwrapToNumber() - 3.15) < 0.01);
    ASSERT_TRUE(Math.abs(ns.namespaceGetVariable('magic_double_n', SType.DOUBLE).unwrapToNumber() - 3.141593) < 0.000001);
    print('NP Success')
}

function testNamespaceVariablesInvailidParam(): void {
    // variable not string type
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable(null, SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable(1, SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable(null, STValue.wrapInt(44), SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable(1, STValue.wrapInt(44), SType.INT));

    // variable not found
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.BOOLEAN));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.BYTE));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.SHORT));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.LONG));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.FLOAT));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('', SType.DOUBLE));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapBoolean(false), SType.BOOLEAN));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapByte(74), SType.BYTE));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapShort(74), SType.SHORT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapLong(1020), SType.LONG));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapFloat(3.15), SType.FLOAT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapNumber(3.141593), SType.DOUBLE));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('', STValue.wrapInt(44), SType.INT));

    // stype not match
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_int_n', SType.DOUBLE));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_boolean_n', SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_byte_n', SType.BOOLEAN));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_short_n', SType.BYTE));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_long_n', SType.SHORT));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_float_n', SType.LONG));
    ASSERT_THROWS(Error, () => ns.namespaceGetVariable('magic_double_n', SType.FLOAT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_int_n', STValue.wrapInt(44), SType.DOUBLE));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_boolean_n', STValue.wrapBoolean(false), SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_byte_n', STValue.wrapByte(74), SType.BOOLEAN));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_short_n', STValue.wrapShort(124), SType.BYTE));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_long_n', STValue.wrapLong(1020), SType.SHORT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_float_n', STValue.wrapFloat(3.15), SType.LONG));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_double_n', STValue.wrapNumber(3.141593), SType.FLOAT));

    // stvalue not match
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_int_n', STValue.wrapBoolean(false), SType.INT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_boolean_n', STValue.wrapByte(74), SType.BOOLEAN));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_byte_n', STValue.wrapShort(124), SType.BYTE));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_short_n', STValue.wrapLong(1020), SType.SHORT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_long_n', STValue.wrapFloat(3.15), SType.LONG));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_float_n', STValue.wrapNumber(3.141593), SType.FLOAT));
    ASSERT_THROWS(Error, () => ns.namespaceSetVariable('magic_double_n', STValue.wrapInt(44), SType.DOUBLE));

    // invalid call
    ASSERT_THROWS(Error, () => STValue.wrapInt(44).namespaceGetVariable('magic_int', SType.INT));
}

// objectGetType(): STValue
function testObjectGetType(): void {
    let stuObj = studentCls.classInstantiate(':', []);
    let stuType = stuObj.objectGetType();

    let strWrap = STValue.wrapString('string');
    let strType = strWrap.objectGetType();

    let biWrap = STValue.wrapBigInt(123n);
    let biTyep = biWrap.objectGetType();

    ASSERT_TRUE(stuObj.objectInstanceOf(stuType));
    ASSERT_TRUE(strWrap.objectInstanceOf(strType));
    ASSERT_TRUE(biWrap.objectInstanceOf(biTyep));

    let checkRes = false;
    try {
        STValue.getUndefined().objectGetType();
        print('NOT Error')
    } catch (e) {
        print('hasError:', e.message)
        checkRes = true;
        checkRes = checkRes && e.message.includes('this');
        checkRes = checkRes && e.message.includes('STValue instance does not wrap a value of type');
    }
    ASSERT_TRUE(checkRes);
}

function testFindClassInvalidParam(): void {
    // 1. no param
    let klass = null;
    try {
        klass = STValue.findClass();
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 2. param args not match
    try {
        klass = STValue.findClass('stvalue_accessor.Animal', 'stvalue_accessor.Animal');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.1 param not string type
    try {
        klass = STValue.findClass(null);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.2 param not string type
    try {
        klass = STValue.findClass(BigInt(123456n));
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.3 param not string type
    try {
        klass = STValue.findClass(1);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 4.1 empty param
    try {
        klass = STValue.findClass('');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.1 param contains special charactor
    try {
        klass = STValue.findClass('stvalue_accessor.Animal;');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.2 param contains special charactor
    try {
        klass = STValue.findClass('stvalue_accessor/Animal');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testFindClassNotFound(): void {
    let klass = null;
    try {
        klass = STValue.findClass('stvalue_accessor.NOT_FOUND');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testFindAbstractClass(): void {
    let absCls = STValue.findClass('stvalue_accessor.Shape');
    let cls = STValue.findClass('stvalue_accessor.Square');
    let superCls = cls.classGetSuperClass();
    ASSERT_TRUE(superCls.isStrictlyEqualTo(absCls));
}

function testFindFinalClass(): void {
    let finalCls = STValue.findClass('stvalue_accessor.City');
    let staticNameField = finalCls.classGetStaticField('name', SType.REFERENCE);
    ASSERT_TRUE(staticNameField.unwrapToString() === 'Shanghai');
}

function testFindClassInNsp(): void {
    let ccls = STValue.findClass('stvalue_accessor.Language.C');
    let staticNameField = ccls.classGetStaticField('name', SType.REFERENCE);
    ASSERT_TRUE(staticNameField.unwrapToString() === 'c language');
}

function testFindNestedClass(): void {
    let jscls = STValue.findClass('stvalue_accessor.Language.DynamicLanguage.Javascript');
    let staticNameField = jscls.classGetStaticField('name', SType.REFERENCE);
    ASSERT_TRUE(staticNameField.unwrapToString() === 'javascript language');
}

function testFindGenericClass(): void {
    let containerCls = STValue.findClass('stvalue_accessor.Container');
    let container = containerCls.classInstantiate(':', []);

    let intCls = STValue.findClass('std.core.Int');
    let data = STValue.wrapInt(100);
    let intObj = intCls.classInstantiate('i:', [data]);

    let area = container.objectInvokeMethod('calArea', 'C{std.core.Object}:C{std.core.Object}', [intObj]);
    let value = area.objectInvokeMethod('toInt', ':i', []);
    ASSERT_TRUE(value.unwrapToNumber() === 100);
}

function testFindClass(): void {
    testFindClassInvalidParam();
    testFindClassNotFound();
    testFindAbstractClass();
    testFindFinalClass();
    testFindClassInNsp();
    testFindNestedClass();
    testFindGenericClass();
}

function testClassGetSuperClass(): void {
    let superClass = STValue.findClass('stvalue_accessor.Animal');
    let subClass = STValue.findClass('stvalue_accessor.Dog');
    let objectClass = STValue.findClass('std.core.Object');

    let objectSuperClass = objectClass.classGetSuperClass();
    ASSERT_TRUE(objectSuperClass === null);

    let resultClass: STValue = subClass.classGetSuperClass();
    ASSERT_TRUE(resultClass.classGetStaticField(
        'name', SType.REFERENCE).unwrapToString() === superClass.classGetStaticField('name', SType.REFERENCE).unwrapToString());

    let nonClass = STValue.wrapInt(111);
    try {
        let resClass = nonClass.classGetSuperClass();
        ASSERT_TRUE(resClass === null);
    } catch (e: Error) { }
}

function testFindNamespaceInvalidParam(): void {
    // 1. no param
    let klass = null;
    try {
        klass = STValue.findNamespace();
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 2. param args not match
    try {
        klass = STValue.findNamespace('stvalue_accessor.Language', 'stvalue_accessor.Language');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.1 param not string type
    try {
        klass = STValue.findNamespace(null);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.2 param not string type
    try {
        klass = STValue.findNamespace(BigInt(123456n));
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.3 param not string type
    try {
        klass = STValue.findNamespace(1);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 4.1 empty param
    try {
        klass = STValue.findNamespace('');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.1 param contains special charactor
    try {
        klass = STValue.findNamespace('stvalue_accessor.Language;');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.2 param contains special charactor
    try {
        klass = STValue.findNamespace('stvalue_accessor/Language');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testFindNamespaceNotFound(): void {
    let klass = null;
    try {
        klass = STValue.findNamespace('stvalue_accessor.NOT_FOUND');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testCallFunctionInNamespace(): void {
    let nsp = STValue.findNamespace('stvalue_accessor.Language');
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(true);
    let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);
}

function testCallFunctionInNestedNamespace(): void {
    let nsp = STValue.findNamespace('stvalue_accessor.Language.DynamicLanguage');
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(true);
    let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === true);
}

function testCallFunctionInDifferentLevelNamespace(): void {
    let nsp = STValue.findNamespace('stvalue_accessor.Language.DynamicLanguage');
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(true);
    let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === true);

    nsp = STValue.findNamespace('stvalue_accessor.Language.DynamicLanguage.Python');
    b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);
}

function testFindNamespace(): void {
    testFindNamespaceInvalidParam();
    testFindNamespaceNotFound();
    testCallFunctionInNamespace();
    testCallFunctionInDifferentLevelNamespace();
}

function testFindEnumInvalidParam(): void {
    // 1. no param
    let klass = null;
    try {
        klass = STValue.findEnum();
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 2. param args not match
    try {
        klass = STValue.findEnum('stvalue_accessor.COLOR', 'stvalue_accessor.COLOR');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.1 param not string type
    try {
        klass = STValue.findEnum(null);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.2 param not string type
    try {
        klass = STValue.findEnum(BigInt(123456n));
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3.3 param not string type
    try {
        klass = STValue.findEnum(1);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 4.1 empty param
    try {
        klass = STValue.findEnum('');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.1 param contains special charactor
    try {
        klass = STValue.findEnum('stvalue_accessor.COLOR;');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5.2 param contains special charactor
    try {
        klass = STValue.findEnum('stvalue_accessor/COLOR');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testFindEnumNotFound(): void {
    let klass = null;
    try {
        klass = STValue.findEnum('stvalue_accessor.NOT_FOUND');
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testFindEnum(): void {
    testFindEnumInvalidParam();
    testFindEnumNotFound();
    testEnumAll();
}

// Test interface implementation
function testInterfaceImplementation(): void {
    let circleClass = STValue.findClass('stvalue_accessor.Circle');
    let circleInstance = circleClass.classInstantiate(':', []);

    let printResult = circleInstance.objectInvokeMethod('print', ':C{std.core.String}', []);
    ASSERT_TRUE(printResult.unwrapToString().includes('Circle'));
    ASSERT_TRUE(printResult.unwrapToString().includes('radius'));

    let radius = circleInstance.objectGetProperty('radius', SType.INT).unwrapToNumber();
    ASSERT_TRUE(radius === 10);

    circleInstance.objectSetProperty('radius', STValue.wrapInt(20), SType.INT);
    let newRadius = circleInstance.objectGetProperty('radius', SType.INT).unwrapToNumber();
    ASSERT_TRUE(newRadius === 20);
}

// Test primitive types
function testPrimitiveTypes(): void {
    let primitiveClass = STValue.findClass('stvalue_accessor.PrimitiveTypes');
    let primitiveInstance = primitiveClass.classInstantiate(':', []);

    ASSERT_TRUE(primitiveInstance.objectGetProperty('byteValue', SType.BYTE).unwrapToNumber() === 0x0A);

    ASSERT_TRUE(primitiveInstance.objectGetProperty('shortValue', SType.SHORT).unwrapToNumber() === 1000);

    ASSERT_TRUE(primitiveInstance.objectGetProperty('intValue', SType.INT).unwrapToNumber() === 100000);

    ASSERT_TRUE(primitiveInstance.objectGetProperty('longValue', SType.LONG).unwrapToNumber() === 10000000000);

    let floatValue = primitiveInstance.objectGetProperty('floatValue', SType.FLOAT).unwrapToNumber();
    ASSERT_TRUE(Math.abs(floatValue - 3.14159) < 0.001);

    let doubleValue = primitiveInstance.objectGetProperty('doubleValue', SType.DOUBLE).unwrapToNumber();
    ASSERT_TRUE(Math.abs(doubleValue - 2.718281828) < 0.000001);

    ASSERT_TRUE(primitiveInstance.objectGetProperty('charValue', SType.CHAR).unwrapToNumber() === 88);

    ASSERT_TRUE(primitiveInstance.objectGetProperty('booleanValue', SType.BOOLEAN).unwrapToBoolean() === true);

    primitiveInstance.objectSetProperty('byteValue', STValue.wrapByte(0x0B), SType.BYTE);
    primitiveInstance.objectSetProperty('shortValue', STValue.wrapShort(2000), SType.SHORT);
    primitiveInstance.objectSetProperty('intValue', STValue.wrapInt(200000), SType.INT);
    primitiveInstance.objectSetProperty('booleanValue', STValue.wrapBoolean(false), SType.BOOLEAN);

    ASSERT_TRUE(primitiveInstance.objectGetProperty('byteValue', SType.BYTE).unwrapToNumber() === 0x0B);
    ASSERT_TRUE(primitiveInstance.objectGetProperty('shortValue', SType.SHORT).unwrapToNumber() === 2000);
    ASSERT_TRUE(primitiveInstance.objectGetProperty('intValue', SType.INT).unwrapToNumber() === 200000);
    ASSERT_TRUE(primitiveInstance.objectGetProperty('booleanValue', SType.BOOLEAN).unwrapToBoolean() === false);
}

// Test null and undefined handling
function testNullAndUndefined(): void {
    let nullableClass = STValue.findClass('stvalue_accessor.NullableClass');
    let nullableInstance = nullableClass.classInstantiate(':', []);

    ASSERT_TRUE(nullableInstance.objectGetProperty('name', SType.REFERENCE).isNull());
    ASSERT_TRUE(nullableInstance.objectGetProperty('age', SType.REFERENCE).isUndefined());

    let stringParam = STValue.wrapString('Alice');
    nullableInstance.objectSetProperty('name', stringParam, SType.REFERENCE);

    let nameResult = nullableInstance.objectInvokeMethod('getName', ':C{std.core.String}', []);
    ASSERT_TRUE(nameResult.unwrapToString() === 'Alice');

    nullableInstance.objectSetProperty('name', STValue.getNull(), SType.REFERENCE);
    ASSERT_TRUE(nullableInstance.objectInvokeMethod('getName', ':C{std.core.String}', []).unwrapToString() === 'Unknown');

    nullableInstance.objectSetProperty('age', STValue.getUndefined(), SType.REFERENCE);
    ASSERT_TRUE(nullableInstance.objectGetProperty('age', SType.REFERENCE).isUndefined());
}

// Test access control
function testAccessControl(): void {
    let accessControlClass = STValue.findClass('stvalue_accessor.AccessControl');
    let accessInstance = accessControlClass.classInstantiate(':', []);

    let publicValue = accessInstance.objectGetProperty('publicValue', SType.INT).unwrapToNumber();
    ASSERT_TRUE(publicValue === 20);

    let privateValue = accessInstance.objectInvokeMethod('getPrivateValue', ':i', []).unwrapToNumber();
    ASSERT_TRUE(privateValue === 10);

    let publicValueGetter = accessInstance.objectInvokeMethod('getPublicValue', ':i', []).unwrapToNumber();
    ASSERT_TRUE(publicValueGetter === 20);

    let privateValueResult = accessInstance.objectGetProperty('privateValue', SType.INT);
    ASSERT_TRUE(privateValueResult.unwrapToNumber() === 10);
}

// Test method invocation with object parameters
function testMethodWithObjectParams(): void {
    let pointClass = STValue.findClass('stvalue_accessor.Point');

    let point1 = pointClass.classInstantiate('ii:', [STValue.wrapInt(0), STValue.wrapInt(0)]);
    let point2 = pointClass.classInstantiate('ii:', [STValue.wrapInt(3), STValue.wrapInt(4)]);

    let distance = point1.objectInvokeMethod('distanceTo', 'C{stvalue_accessor.Point}:d', [point2]);
    ASSERT_TRUE(Math.abs(distance.unwrapToNumber() - 5.0) < 0.001);

    let point1Str = point1.objectInvokeMethod('toString', ':C{std.core.String}', []).unwrapToString();
    ASSERT_TRUE(point1Str.includes('Point'));
    ASSERT_TRUE(point1Str.includes('0'));

    let point2Str = point2.objectInvokeMethod('toString', ':C{std.core.String}', []).unwrapToString();
    ASSERT_TRUE(point2Str.includes('Point'));
    ASSERT_TRUE(point2Str.includes('3'));
    ASSERT_TRUE(point2Str.includes('4'));

    let geometryClass = STValue.findClass('stvalue_accessor.Geometry');
    let staticDistance = geometryClass.classInvokeStaticMethod('calculateDistance',
        'C{stvalue_accessor.Point}C{stvalue_accessor.Point}:d',
        [point1, point2]);
    ASSERT_TRUE(Math.abs(staticDistance.unwrapToNumber() - 5.0) < 0.001);
}

// Test class instantiation
function testClassInstantiation(): void {
    let calculatorClass = STValue.findClass('stvalue_accessor.Calculator');
    let calcInstance = calculatorClass.classInstantiate(':', []);
    ASSERT_TRUE(calcInstance.objectInvokeMethod('getResult', ':i', []).unwrapToNumber() === 100);

    let pointClass = STValue.findClass('stvalue_accessor.Point');
    let pointInstance = pointClass.classInstantiate('ii:', [STValue.wrapInt(10), STValue.wrapInt(20)]);
    let xValue = pointInstance.objectGetProperty('x', SType.INT).unwrapToNumber();
    let yValue = pointInstance.objectGetProperty('y', SType.INT).unwrapToNumber();
    ASSERT_TRUE(xValue === 10);
    ASSERT_TRUE(yValue === 20);
}

// Test newArray creation
function testNewArray(): void {
    let emptyArray = STValue.newArray(0, STValue.findClass('std.core.Int'));
    ASSERT_TRUE(emptyArray.arrayGetLength() === 0);

    let intClass = STValue.findClass('std.core.Int');
    let intObj1 = intClass.classInstantiate('i:', [STValue.wrapInt(1)]);
    let intObj2 = intClass.classInstantiate('i:', [STValue.wrapInt(2)]);
    let intObj3 = intClass.classInstantiate('i:', [STValue.wrapInt(3)]);

    let arrayWithElements = STValue.newArray(3, intObj1);
    ASSERT_TRUE(arrayWithElements.arrayGetLength() === 3);

    let elem0 = arrayWithElements.arrayGet(0);
    let elem1 = arrayWithElements.arrayGet(1);
    let elem2 = arrayWithElements.arrayGet(2);
    ASSERT_TRUE(elem0.objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 1);
    ASSERT_TRUE(elem1.objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 1);
    ASSERT_TRUE(elem2.objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 1);

    arrayWithElements.arraySet(1, intObj2);
    arrayWithElements.arraySet(2, intObj3);
    ASSERT_TRUE(arrayWithElements.arrayGet(1).objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 2);
    ASSERT_TRUE(arrayWithElements.arrayGet(2).objectInvokeMethod('toInt', ':i', []).unwrapToNumber() === 3);
}

function stvalueAccessor() {
    testClassGetSuperClass();
    testFixedArrayGetLength();
    testArrayGetLength();
    testClassGetAndSetStaticField();
    testObjectGetAndSetProperty();
    testFixedArrayGetAndSet();
    testArrayGetAndSet();
    testArrayGetAndSetInvailidParam();
    testArrayPushAndPop();
    testArrayPushAndPopInvailidParam();
    testNamespaceVariable();
    testNamespaceVariablesInvailidParam();
    testObjectGetType();
    testFindClass();
    testFindNamespace();
    testFindEnum();
    testInterfaceImplementation();
    testPrimitiveTypes();
    testNullAndUndefined();
    testAccessControl();
    testMethodWithObjectParams();
    testClassInstantiation();
    testNewArray();
}

const result = stvalueAccessor();