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

let SType = etsVm.SType;
let STValue = etsVm.STValue;
let nsp = STValue.findNamespace('stvalue_invoke.Invoke');

let studentCls = STValue.findClass('stvalue_invoke.Student');
let subStudentCls = STValue.findClass('stvalue_invoke.SubStudent');

// functionalObjectInvoke(args: Array<STValue>): STValue
function testFunctionalObjectInvoke(): void {
    let getNumberFn = nsp.namespaceGetVariable('getNumberFn', SType.REFERENCE);
    let numRes = getNumberFn.functionalObjectInvoke([]);
    let jsNum = numRes.objectInvokeMethod('toInt', ':i', []);
    ASSERT_TRUE(jsNum.unwrapToNumber() === 123);

    let numberCls = STValue.findClass('std.core.Double');
    let numberObj3 = numberCls.classInstantiate('d:', [STValue.wrapNumber(3)]);
    let numberObj5 = numberCls.classInstantiate('d:', [STValue.wrapNumber(5)]);
    let getSumFn = nsp.namespaceGetVariable('getSumFn', SType.REFERENCE);
    let sumRes = getSumFn.functionalObjectInvoke([numberObj3, numberObj5]);
    let sumNum = sumRes.objectInvokeMethod('toDouble', ':d', []);
    ASSERT_TRUE(sumNum.unwrapToNumber() === 8);

    let res = false;
    try {
        getSumFn.functionalObjectInvoke([numberObj3]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('ANI error occurred');
        res = res && e.message.includes('got error');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        STValue.getUndefined().functionalObjectInvoke([]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('STValue instance does not wrap a value of type reference');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        getSumFn.functionalObjectInvoke([numberObj3, STValue.getUndefined()]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('undefined cannot be cast to std.core.Double');
    }
    ASSERT_TRUE(res);
}

// objectInvokeMethod(name: string, signature: string, args: Array<STValue>): STValue
function testObjectInvokeMethod(): void {
    let clsObj = studentCls.classInstantiate('iC{std.core.String}:', [STValue.wrapInt(18), STValue.wrapString('stu1')]);
    let stuAge = clsObj.objectInvokeMethod('getStudentAge', ':i', []);
    let stuName = clsObj.objectInvokeMethod('getStudentName', ':C{std.core.String}', []);
    clsObj.objectInvokeMethod('setStudentAge', 'i:', [STValue.wrapInt(20)]);
    let newStuAge = clsObj.objectInvokeMethod('getStudentAge', ':i', []);
    ASSERT_TRUE(stuAge.unwrapToNumber() === 18);
    ASSERT_TRUE(stuName.unwrapToString() === 'stu1');
    ASSERT_TRUE(newStuAge.unwrapToNumber() === 20);

    let subClsObj = subStudentCls.classInstantiate('iC{std.core.String}:', [STValue.wrapInt(18), STValue.wrapString('stu1')]);
    subClsObj.objectInvokeMethod('setStudentAge', 'i:', [STValue.wrapInt(32)]);
    let subStuAge = subClsObj.objectInvokeMethod('getStudentAge', ':i', []);
    ASSERT_TRUE(subStuAge.unwrapToNumber() === 32);

    clsObj.objectInvokeMethod('setStudentAge', 'd:', [STValue.wrapNumber(55.55)]);
    let doubleStuAge = clsObj.objectInvokeMethod('getStudentAge', ':i', []);
    ASSERT_TRUE(doubleStuAge.unwrapToNumber() === 55);

    let res = false;
    try {
        subClsObj.objectInvokeMethod('setStudentAge', 'C{std.core.String}:', [STValue.wrapInt(99)]);
        let ssssubStuAge = subClsObj.objectInvokeMethod('getStudentAge', ':f', []);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('ANI error occurred');
        res = res && e.message.includes('status: 7');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        STValue.getUndefined().objectInvokeMethod('setStudentAge', 'C{std.core.String}:', [STValue.wrapInt(99)]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('STValue instance does not wrap a value of type reference');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        subClsObj.objectInvokeMethod('setStudentAge111', 'C{std.core.String}:', [STValue.wrapInt(99)]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Unknown ANI error occurred');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        subClsObj.objectInvokeMethod('setStudentAge', 'i:', [[STValue.getUndefined()]]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('param array element are not STValue type');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        subClsObj.objectInvokeMethod('setStudentAge', ':', [STValue.wrapInt(32)]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Argument count mismatch with function signature.');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        subClsObj.objectInvokeMethod('setStudentAge', 'ii:', [STValue.wrapInt(32)]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Argument count mismatch with function signature.');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        subClsObj.objectInvokeMethod('setStudentAge', 'iC{std.core.String}:', [STValue.wrapInt(32)]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Argument count mismatch with function signature.');
    }
    ASSERT_TRUE(res);
}

// ClassInvokeStaticMethod(name: string, signature: string, args: Array<STValue>): STValue
function testClassInvokeStaticMethod(): void {
    let stuId = studentCls.classInvokeStaticMethod('getStudentId', ':i', []);
    studentCls.classInvokeStaticMethod('setStudentId', 'i:', [STValue.wrapInt(888)]);
    let newStuId = studentCls.classInvokeStaticMethod('getStudentId', ':i', []);
    ASSERT_TRUE(stuId.unwrapToNumber() === 999);
    ASSERT_TRUE(newStuId.unwrapToNumber() === 888);

    subStudentCls.classInvokeStaticMethod('setUId', 'i:', [STValue.wrapInt(888)]);
    let subUId1 = subStudentCls.classInvokeStaticMethod('getUId', ':i', []);
    ASSERT_TRUE(subUId1.unwrapToNumber() === 888);
    subStudentCls.classInvokeStaticMethod('setUId', 'd:', [STValue.wrapNumber(123456.789)]);
    let subUId2 = subStudentCls.classInvokeStaticMethod('getUId', ':i', []);
    ASSERT_TRUE(subUId2.unwrapToNumber() === 123456);

    let res = false;
    try {
        subStudentCls.classInvokeStaticMethod('setUId', 's:', []);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Argument count mismatch with function signature.');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        STValue.getUndefined().classInvokeStaticMethod('setUId', 's:', []);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('STValue instance does not wrap a value of type reference');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        subStudentCls.classInvokeStaticMethod('setUId123', 'i:', [STValue.wrapInt(888)]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Unknown ANI error occurred');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        studentCls.classInvokeStaticMethod('setStudentId', ':', [STValue.wrapInt(888)]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Argument count mismatch with function signature.');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        studentCls.classInvokeStaticMethod('setStudentId', 'ii:', [STValue.wrapInt(888)]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Argument count mismatch with function signature.');
    }
    ASSERT_TRUE(res);

    res = false;
    try {
        studentCls.classInvokeStaticMethod('setStudentId', 'iC{std.core.String}:', [STValue.wrapInt(888)]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Argument count mismatch with function signature.');
    }
    ASSERT_TRUE(res);
}

function testNspFunctionMismatch(): void {
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(false);
    let b = null;
    let mod = null;
    // 1. namespaceInvokeFunction is an instance method
    try {
        b = STValue.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testNspFunctionInvalidParamCount(): void {
    let b = null;
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(false);
    // 1. param count = 0
    try {
        b = nsp.namespaceInvokeFunction();
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 2. param count = 1
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke');
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3. param count = 4
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2], b1);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 4. param count = 5
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2], b1, b2);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testNspFunctionInvalidParamType(): void {
    // 1. functionName is not string type
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let b = null;
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(false);
    try {
        b = nsp.namespaceInvokeFunction(b1, 'zz:z', [b1, b2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 2. signatureName is not string type
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', b1, [b1, b2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3. args is not array type
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z');
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', b1);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zzC{std.core.Boolean}:z', [b1, b2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 4. args element is not STValue type
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, null]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5. invalid returnType, no : in signature
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz', [b1, b2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 6. invalid returnType, no match type in signature
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:a', [b1, b2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testNspFunctionUnexist(): void {
    // 1. function name not exist
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let b = null;
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(false);
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvokeNotFound', 'zz:z', [b1, b2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testNspFunctionSignatureMismatch(): void {
    // 1. signature input param is more
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let b = null;
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(false);
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zzz:z', [b1, b2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 2. signature input param is less
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', 'z:z', [b1, b2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3. signature output param is less
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:', [b1, b2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 4. signature output param is more
    try {
        b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:zz', [b1, b2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5. signature input param is less
    try {
        let big1 = STValue.wrapBigInt(BigInt('12345678'));
        let big2 = STValue.wrapBigInt(BigInt('12345678'));
        b = nsp.namespaceInvokeFunction('BigIntInvoke', 'C{escompat.BigInt}:C{escompat.BigInt}', [big1, big2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 5. signature input param is more
    try {
        let big1 = STValue.wrapBigInt(BigInt('12345678'));
        let big2 = STValue.wrapBigInt(BigInt('12345678'));
        b = nsp.namespaceInvokeFunction('BigIntInvoke', 'C{escompat.BigInt}C{escompat.BigInt}C{escompat.BigInt}:C{escompat.BigInt}', [big1, big2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 6. signature output param is less
    try {
        let big1 = STValue.wrapBigInt(BigInt('12345678'));
        let big2 = STValue.wrapBigInt(BigInt('12345678'));
        b = nsp.namespaceInvokeFunction('BigIntInvoke', 'C{escompat.BigInt}C{escompat.BigInt}:', [big1, big2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 7. signature output param is more
    try {
        let big1 = STValue.wrapBigInt(BigInt('12345678'));
        let big2 = STValue.wrapBigInt(BigInt('12345678'));
        b = nsp.namespaceInvokeFunction('BigIntInvoke', 'C{escompat.BigInt}C{escompat.BigInt}:C{escompat.BigInt}C{escompat.BigInt}', [big1, big2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testNspFunctionParamArrayCountMismatch(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(false);
    let b3 = STValue.wrapBoolean(false);

    // 1. param array count is less
    try {
        let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', []);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 2. param array count is less
    try {
        let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

    // 3. param array count is more
    try {
        let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2, b3]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }
}

function testNspFunctionParamArrayTypeMismatch(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let s1 = STValue.wrapString('invoke1');
    let s2 = STValue.wrapString('invoke2');

    // 1. param array element is not boolean
    try {
        let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [s1, s2]);
        ASSERT_TRUE(b.unwrapToBoolean() === false);
    } catch (e) {
        print('Error code: ', e.code);
        print('Error message: ', e.message);
    }

}


function testNspFunctionInvalidParam(): void {
    testNspFunctionInvalidParamCount();
    testNspFunctionInvalidParamType();
    testNspFunctionUnexist();
    testNspFunctionSignatureMismatch();
    testNspFunctionParamArrayCountMismatch();
    testNspFunctionParamArrayTypeMismatch();
}

function testNspBooleanInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let b1 = STValue.wrapBoolean(false);
    let b2 = STValue.wrapBoolean(false);
    let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);

    b1 = STValue.wrapBoolean(false);
    b2 = STValue.wrapBoolean(true);
    b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);

    b1 = STValue.wrapBoolean(true);
    b2 = STValue.wrapBoolean(false);
    b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === false);

    b1 = STValue.wrapBoolean(true);
    b2 = STValue.wrapBoolean(true);
    b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]);
    ASSERT_TRUE(b.unwrapToBoolean() === true);

    b = nsp.namespaceInvokeFunction('BooleanEmptyInvoke', ':z', []);
    ASSERT_TRUE(b.unwrapToBoolean() === true);
}

function testNspCharInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let c = STValue.wrapChar('c');
    let r = nsp.namespaceInvokeFunction('CharInvoke', 'c:c', [c]);
    let charKlass = STValue.findClass('std.core.Char');
    let charObj = charKlass.classInstantiate('c:', [r]);
    let str = charObj.objectInvokeMethod('toString', ':C{std.core.String}', []);
    ASSERT_TRUE(str.unwrapToString() === 'c');

    r = nsp.namespaceInvokeFunction('CharEmptyInvoke', ':c', []);
    charKlass = STValue.findClass('std.core.Char');
    charObj = charKlass.classInstantiate('c:', [r]);
    str = charObj.objectInvokeMethod('toString', ':C{std.core.String}', []);
    ASSERT_TRUE(str.unwrapToString() === 'c');
}

function testNspByteInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let b = STValue.wrapByte(112);
    let r = nsp.namespaceInvokeFunction('ByteInvoke', 'b:b', [b]);
    ASSERT_TRUE(r.unwrapToNumber() === 112);

    r = nsp.namespaceInvokeFunction('ByteEmptyInvoke', ':b', []);
    ASSERT_TRUE(r.unwrapToNumber() === 112);
}

function testNspShortInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let s = STValue.wrapShort(3456);
    let r = nsp.namespaceInvokeFunction('ShortInvoke', 's:s', [s]);
    ASSERT_TRUE(r.unwrapToNumber() === 3456);

    r = nsp.namespaceInvokeFunction('ShortEmptyInvoke', ':s', []);
    ASSERT_TRUE(r.unwrapToNumber() === 3456);
}

function testNspIntInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let a1 = STValue.wrapInt(123);
    let a2 = STValue.wrapInt(345);
    let r = nsp.namespaceInvokeFunction('IntInvoke', 'ii:i', [a1, a2]);
    ASSERT_TRUE(r.unwrapToNumber() === 468);

    r = nsp.namespaceInvokeFunction('IntEmptyInvoke', ':i', []);
    ASSERT_TRUE(r.unwrapToNumber() === 468);
}

function testNspLongInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let a1 = STValue.wrapLong(123);
    let a2 = STValue.wrapLong(345);
    let r = nsp.namespaceInvokeFunction('LongInvoke', 'll:l', [a1, a2]);
    ASSERT_TRUE(r.unwrapToNumber() === 468);

    r = nsp.namespaceInvokeFunction('LongEmptyInvoke', ':l', []);
    ASSERT_TRUE(r.unwrapToNumber() === 468);
}

function testNspFloatInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let a1 = STValue.wrapFloat(1.4567);
    let a2 = STValue.wrapFloat(4.5678);
    nsp.namespaceInvokeFunction('FloatInvoke', 'ff:f', [a1, a2]);

    nsp.namespaceInvokeFunction('FloatEmptyInvoke', ':f', []);
}

function testNspDoubleInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let a1 = STValue.wrapNumber(1.4567);
    let a2 = STValue.wrapNumber(4.5678);
    nsp.namespaceInvokeFunction('DoubleInvoke', 'dd:d', [a1, a2]);

    nsp.namespaceInvokeFunction('DoubleEmptyInvoke', ':d', []);
}

function testNspNumberInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let a1 = STValue.wrapNumber(1.4567);
    let a2 = STValue.wrapNumber(4.5678);
    nsp.namespaceInvokeFunction('NumberInvoke', 'dd:d', [a1, a2]);

    nsp.namespaceInvokeFunction('NumberEmptyInvoke', ':d', []);
}

function testNspStringInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let s1 = STValue.wrapString('ABCDEFG');
    let s2 = STValue.wrapString('HIJKLMN');
    let s = nsp.namespaceInvokeFunction('StringInvoke', 'C{std.core.String}C{std.core.String}:C{std.core.String}', [s1, s2]);
    ASSERT_TRUE(s.unwrapToString() === 'ABCDEFGHIJKLMN');

    s = nsp.namespaceInvokeFunction('StringEmptyInvoke', ':C{std.core.String}', []);
    ASSERT_TRUE(s.unwrapToString() === 'string');
}

function testNspBigIntInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let b1 = STValue.wrapBigInt(BigInt('12345678'));
    let b2 = STValue.wrapBigInt(BigInt(12345678n));
    //let s = nsp.namespaceInvokeFunction('BigIntInvoke','C{std.core.BigInt}C{std.core.BigInt}:C{std.core.BigInt}',[b1,b2]);
    let s = nsp.namespaceInvokeFunction('BigIntInvoke', 'C{std.core.BigInt}C{std.core.BigInt}:C{std.core.BigInt}', [b1, b2]);
    ASSERT_TRUE(s.unwrapToBigInt() === BigInt(24691356n));

    s = nsp.namespaceInvokeFunction('BigIntEmptyInvoke', ':C{std.core.BigInt}', []);
    ASSERT_TRUE(s.unwrapToBigInt() === BigInt(1234n));
}

function testNspVoidInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let s1 = STValue.wrapString('ABCDEFG');
    let s2 = STValue.wrapString('HIJKLMN');
    nsp.namespaceInvokeFunction('VoidInvoke', 'C{std.core.String}C{std.core.String}:', [s1, s2]);

    nsp.namespaceInvokeFunction('VoidEmptyInvoke', ':', []);
}

function testLambdaFunInvokeFunction(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
    let staLam = nsp.namespaceGetVariable('lambdaFun', SType.REFERENCE);
    let res = nsp.namespaceInvokeFunction('testLambdaParam', 'C{std.core.Function0}:z', [staLam]);
    ASSERT_TRUE(res.unwrapToBoolean());
}

function testDefaultParamInvoke(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');

    let strValue = STValue.wrapString('xxx');
    let result1 = nsp.namespaceInvokeFunction('testDefaultParam', 'C{std.core.String}:C{std.core.String}', [strValue]);
    ASSERT_TRUE(result1.unwrapToString() === 'Hello_xxx');
    
    let strValue2 = STValue.getUndefined();
    let result2 = nsp.namespaceInvokeFunction('testDefaultParam', 'C{std.core.String}:C{std.core.String}', [strValue2]);
    ASSERT_TRUE(result2.unwrapToString() === 'Hello_World');
}

function testRestParamInvoke(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');

    let result0 = nsp.namespaceInvokeFunction('testRestParam', 'C{std.core.Array}:C{std.core.String}', [STValue.getUndefined()]);
    ASSERT_TRUE(result0.unwrapToString() === '');

    // Test with single argument
    let arg1 = STValue.wrapString('abc');
    let strArray = STValue.newArray(1, arg1);
    let result1 = nsp.namespaceInvokeFunction('testRestParam', 'C{std.core.Array}:C{std.core.String}', [strArray]);
    ASSERT_TRUE(result1.unwrapToString() === 'abc');

    // Test with multiple arguments
    let arg2 = STValue.wrapString('def');
    let arg3 = STValue.wrapString('ghi');
    strArray.arrayPush(arg2);
    strArray.arrayPush(arg3);
    let result2 = nsp.namespaceInvokeFunction('testRestParam', 'C{std.core.Array}:C{std.core.String}', [strArray]);
    ASSERT_TRUE(result2.unwrapToString() === 'abc,def,ghi');
}

function testOptionalParamInvoke(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.Invoke');

    let arg = STValue.wrapString('abc');
    let result1 = nsp.namespaceInvokeFunction('testOptionalParam', 'C{std.core.String}:C{std.core.String}', [arg]);
    ASSERT_TRUE(result1.unwrapToString() === 'got-abc');

    let arg2 = STValue.getUndefined();
    let result2 = nsp.namespaceInvokeFunction('testOptionalParam', 'C{std.core.String}:C{std.core.String}', [arg2]);
    ASSERT_TRUE(result2.unwrapToString() === 'got-no-arg');
}

function testNamespaceInvokeFunction(): void {
    testNspFunctionMismatch();
    testNspFunctionInvalidParam();
    testNspBooleanInvokeFunction();
    testNspCharInvokeFunction();
    testNspByteInvokeFunction();
    testNspShortInvokeFunction();
    testNspIntInvokeFunction();
    testNspLongInvokeFunction();
    testNspFloatInvokeFunction();
    testNspDoubleInvokeFunction();
    testNspNumberInvokeFunction();
    testNspStringInvokeFunction();
    testNspBigIntInvokeFunction();
    testNspVoidInvokeFunction();
    testLambdaFunInvokeFunction();
    testDefaultParamInvoke();
    testRestParamInvoke();
    testOptionalParamInvoke();
}

function funJsFullGC(): void {
    globalThis.ArkTools.GC.clearWeakRefForTest();
    let gcId = globalThis.ArkTools.GC.startGC('full');
    globalThis.ArkTools.GC.waitForFinishGC(gcId);
}

let module = STValue.findNamespace('stvalue_invoke.ETSGLOBAL');
function createWeakRefForSTValue(): void {
    const dateCls = STValue.findClass('std.core.Date');
    const dateObj = dateCls.classInstantiate(':', []);
    const staticWeakRef = module.namespaceInvokeFunction('createWeakRef',
        'C{std.core.Date}:C{std.core.WeakRef}', [dateObj]);
    return [new WeakRef(dateObj), staticWeakRef];
}

function testSTValueDestruct(): void {
    let [dyWeakRef, staWeakRef] = createWeakRefForSTValue();
    ASSERT_TRUE(dyWeakRef.deref() !== undefined);
    let staWeakObj = staWeakRef.objectInvokeMethod('deref', ':C{std.core.Object}', []);
    ASSERT_TRUE(!staWeakObj.isUndefined());

    etsVm.getFunction('Lstvalue_invoke/ETSGLOBAL;', 'runFullGC')();
    funJsFullGC();
    
    // NOTE(www) #31863, after this, it's also need to verify that the object held by staWeakObj is undefined.
    ASSERT_TRUE(dyWeakRef.deref() === undefined);
}

// Constructor invocation tests
function testConstructorInvoke(): void {
    let constructorCls = STValue.findClass('stvalue_invoke.ConstructorTest');

    let obj1 = constructorCls.classInstantiate(':', []);
    let val1 = obj1.objectInvokeMethod('getValue', ':i', []);
    ASSERT_TRUE(val1.unwrapToNumber() === 0);

    let obj2 = constructorCls.classInstantiate('i:', [STValue.wrapInt(42)]);
    let val2 = obj2.objectInvokeMethod('getValue', ':i', []);
    ASSERT_TRUE(val2.unwrapToNumber() === 42);

    let obj3 = constructorCls.classInstantiate('iC{std.core.String}:', [STValue.wrapInt(100), STValue.wrapString('test')]);
    let val3 = obj3.objectInvokeMethod('getValue', ':i', []);
    ASSERT_TRUE(val3.unwrapToNumber() === 100);

    let res = false;
    try {
        constructorCls.classInstantiate('d:', [STValue.wrapNumber(1.5)]);
    } catch (e: Error) {
        res = true
        res = res && e.message.includes('Unknown ANI error occurred');
    }
    ASSERT_TRUE(res);
}

// Method overload tests
function testMethodOverload(): void {
    let overloadCls = STValue.findClass('stvalue_invoke.OverloadTest');
    let obj = overloadCls.classInstantiate(':', []);

    let res1 = obj.objectInvokeMethod('process', 'i:C{std.core.String}', [STValue.wrapInt(123)]);
    ASSERT_TRUE(res1.unwrapToString() === 'int:123');

    let res2 = obj.objectInvokeMethod('process', 'd:C{std.core.String}', [STValue.wrapNumber(45.67)]);
    ASSERT_TRUE(res2.unwrapToString() === 'double:45.67');

    let res3 = obj.objectInvokeMethod('process', 'C{std.core.String}:C{std.core.String}', [STValue.wrapString('hello')]);
    ASSERT_TRUE(res3.unwrapToString() === 'string:hello');

    let res4 = obj.objectInvokeMethod('process', 'ii:C{std.core.String}', [STValue.wrapInt(10), STValue.wrapInt(5)]);
    ASSERT_TRUE(res4.unwrapToString() === 'int,scale:50');

    let nsp = STValue.findNamespace('stvalue_invoke.ETSGLOBAL');
    let overloadRes1 = nsp.namespaceInvokeFunction('invokeOverloadTest', 'i:C{std.core.String}', [STValue.wrapInt(123)]);
    ASSERT_TRUE(overloadRes1.unwrapToString() === 'int:123');
    let overloadRes2 = nsp.namespaceInvokeFunction('invokeOverloadTest', 'C{std.core.String}:C{std.core.String}', [STValue.wrapString('hello')]);
    ASSERT_TRUE(overloadRes2.unwrapToString() === 'string:hello');
    let overloadRes3 = nsp.namespaceInvokeFunction('invokeOverloadTest', 'iC{std.core.String}:C{std.core.String}', [STValue.wrapInt(123), STValue.wrapString('hello')]);
    ASSERT_TRUE(overloadRes3.unwrapToString() === 'int,string:123,hello');
}

// Boundary value tests
function testBoundaryValues(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.ETSGLOBAL');

    let maxVal = nsp.namespaceInvokeFunction('TestBoundaryMax', ':i', []);
    ASSERT_TRUE(maxVal.unwrapToNumber() === 2147483647);

    let minVal = nsp.namespaceInvokeFunction('TestBoundaryMin', ':i', []);
    ASSERT_TRUE(minVal.unwrapToNumber() === -2147483648);

    let a1 = STValue.wrapInt(2147483647);
    let b1 = STValue.wrapInt(0);
    let sum1 = nsp.namespaceInvokeFunction('TestBoundaryAdd', 'ii:i', [a1, b1]);
    ASSERT_TRUE(sum1.unwrapToNumber() === 2147483647);

    let a2 = STValue.wrapInt(0);
    let b2 = STValue.wrapInt(0);
    let sum2 = nsp.namespaceInvokeFunction('TestBoundaryAdd', 'ii:i', [a2, b2]);
    ASSERT_TRUE(sum2.unwrapToNumber() === 0);

    let a3 = STValue.wrapInt(-100);
    let b3 = STValue.wrapInt(-200);
    let sum3 = nsp.namespaceInvokeFunction('TestBoundaryAdd', 'ii:i', [a3, b3]);
    ASSERT_TRUE(sum3.unwrapToNumber() === -300);
}

// null and undefined handling tests
function testNullUndefinedHandling(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.ETSGLOBAL');

    let nullResult = nsp.namespaceInvokeFunction('TestNullParam', 'X{C{std.core.Null}C{stvalue_invoke.Student}}:C{std.core.String}', [STValue.getNull()]);
    ASSERT_TRUE(nullResult.unwrapToString() === 'null');

    let studentCls = STValue.findClass('stvalue_invoke.Student');
    let studentObj = studentCls.classInstantiate('i:', [STValue.wrapInt(18)]);
    let validResult = nsp.namespaceInvokeFunction('TestNullParam', 'X{C{std.core.Null}C{stvalue_invoke.Student}}:C{std.core.String}', [studentObj]);
    ASSERT_TRUE(validResult.unwrapToString() === 'valid');

    let unResult = nsp.namespaceInvokeFunction('TestUndefinedParam', 'C{std.core.Object}:C{std.core.String}', [STValue.getUndefined()]);
    ASSERT_TRUE(unResult.unwrapToString() === 'processed');
}

// Exception handling tests
function testExceptionHandling(): void {
    let nsp = STValue.findNamespace('stvalue_invoke.ETSGLOBAL');

    // Test throwing exception
    let res = false;
    try {
        nsp.namespaceInvokeFunction('TestThrowException', ':i', []);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Test exception');
    }
    ASSERT_TRUE(res);

    // Test divide by zero exception
    res = false;
    try {
        nsp.namespaceInvokeFunction('TestDivideByZero', 'ii:i', [STValue.wrapInt(10), STValue.wrapInt(0)]);
    } catch (e: Error) {
        res = true;
        res = res && e.message.includes('Division by zero');
    }
    ASSERT_TRUE(res);

    // Test normal division (no exception)
    let normalResult = nsp.namespaceInvokeFunction('TestDivideByZero', 'ii:i', [STValue.wrapInt(20), STValue.wrapInt(4)]);
    ASSERT_TRUE(normalResult.unwrapToNumber() === 5);
}

function main(): void {
    testFunctionalObjectInvoke();
    testObjectInvokeMethod();
    testClassInvokeStaticMethod();
    testNamespaceInvokeFunction();
    testSTValueDestruct();
    testConstructorInvoke();
    testMethodOverload();
    testBoundaryValues();
    testNullUndefinedHandling();
    testExceptionHandling();
}

main();
