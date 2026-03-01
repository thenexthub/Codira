/*
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

let module = ESValue.load('esvalue_def_test');
let module2 = ESValue.load('esvalue_def_test2');

let jsBoolean = module.getProperty('b');
let jsNumber = module.getProperty('n');
let jsString = module.getProperty('s');
let jsBoxNum = module2.getProperty('boxNum');
let jsBoxBool = module2.getProperty('boxBool');
let jsBoxStr = module2.getProperty('boxStr');
let jsUndefined = module.getProperty('jsUndefined');
let jsBigInt = module.getProperty('bi');
let jsNull = module.getProperty('jsNull');
let jsFunc = module.getProperty('jsFunc');
let jsFuncWithParam = module.getProperty('jsFuncWithParam');
let jsArray = module.getProperty('jsArray');
let jsArray1 = module.getProperty('jsArray1');
let jsObjectA = module.getProperty('A');
let jsObjectB = module.getProperty('B');
let jsObjWithFunc = module.getProperty('objWithFunc');
let jsObjWithFuncParam = module.getProperty('objWithFuncParam');
let jsIterable = module.getProperty('jsIterable');
let jsIterableObject = module.getProperty('testItetatorObject');
let jsUser = module.getProperty('User');
let jsDoubledObj = module.getProperty('doubledObj');
let propObj = module.getProperty('propObj');
let throwNull = module.getProperty('throwNull');
let throwUndefined = module.getProperty('throwUndefined');

function checkGetUndefined(): boolean {
    let obj = ESValue.Undefined;
    return obj.typeOf() === 'undefined';
}

function checkGetNull(): boolean {
    let obj = ESValue.Null;
    return obj.isNull();
}

function checkWrapBoolean(): boolean {
    let val = false;
    let obj = ESValue.wrapBoolean(val);
    return obj.toBoolean() === val;
}

function checkWrapString(): boolean {
    let val = 'hello';
    let obj = ESValue.wrapString(val);
    return obj.toString() === val;
}

function checkWrapBigInt(): boolean {
    let val = new BigInt(9007199254740991n);
    let obj = ESValue.wrapBigInt(val);
    return obj.toBigInt() === val;
}

function checkWrapNumber(): boolean {
    let val = 111;
    let obj = ESValue.wrapNumber(val);
    return obj.toNumber() === val;
}

function checkWrapDouble(): boolean {
    let val: double = 111;
    let obj = ESValue.wrapDouble(val);
    return obj.toNumber() === val;
}

function checkIsBoolean(): boolean {
    return jsBoolean.isBoolean();
}

function checkIsString(): boolean {
    return jsString.isString();
}

function checkIsNumber(): boolean {
    return jsNumber.isNumber();
}

function checkIsBigInt(): boolean {
    return jsBigInt.isBigInt();
}

function checkIsUndefined(): boolean {
    let obj = ESValue.Undefined;
    return obj.isUndefined();
}

function checkIsFunction(): boolean {
    return jsFunc.isFunction();
}

function checkToBoolean(): boolean {
    return jsBoolean.toBoolean() === false;
}

function checkToString(): boolean {
    return jsString.toString() === 'hello';
}

function checkToNumber(): boolean {
    return jsNumber.toNumber() === 1;
}

function checkToBigInt(): boolean {
    return jsBigInt.toBigInt() === 9007199254740991n;
}

function checkToUndefined(): boolean {
    return jsUndefined.toUndefined() === undefined;
}

function checkToNull(): boolean {
    return jsNull.toNull() === null;
}

function checkAreStrictEqual(): boolean {
    return ESValue.areStrictlyEqual(jsFunc, jsFunc);
}

function checkIsEqualTo(): boolean {
    return jsFunc.isEqualTo(jsFunc);
}

function checkIsEqualToSafe(): boolean {
    let nullObj1 = ESValue.Null;
    let nullObj2 = ESValue.Null;
    return nullObj1.isEqualToSafe(nullObj2);
}

function checkGetPropertyByName(): boolean {
    let val = jsObjectA.getProperty('property2');
    return val.toNumber() === 2;
}

class staticObjectA {
    aaa: string = ''
}
function checkGetPropertyStaticObj(): boolean {
    let sObj = new staticObjectA();
    sObj.aaa = 'bbb';
    let obj = ESValue.wrap(sObj);
    let value = obj.getPropertySafe('aaa');
    return value.toString() === 'bbb';
}

function checkGetPropertyByNameSafe(): boolean {
    let val = jsObjectA.getPropertySafe('property3');
    return val.isUndefined() === true;
}

function checkGetPropertyByIndex(): boolean {
    let val = jsArray.getProperty(2);
    return val.toBoolean() === true;
}

function checkGetPropertyByIndexDouble(): boolean {
    let index = 3.2;
    let val = jsArray.getPropertySafe(3.2);
    let val1 = jsDoubledObj.getProperty(index);
    return val1.toString() === 'aaa';
}

function checkGetPropertyByIndexSafe(): boolean {
    let val = jsArray.getPropertySafe(3);
    return val.isUndefined() === true;
}

function checkGetProperty(): boolean {
    let property = ESValue.wrapString('property2');
    let val = jsObjectA.getProperty(property);
    return val.toNumber() === 2;
}

function checkGetPropertySafe(): boolean {
    let peroperty = ESValue.wrapString('property3');
    let val = jsObjectA.getPropertySafe(peroperty);
    return val.isUndefined() === true;
}

function checkSetPropertyByName(): boolean {
    let value = ESValue.wrapNumber(5);
    jsObjectA.setProperty('property1', value);
    let val = jsObjectA.getProperty('property1');
    return val.toNumber() === 5;
}

function checkSetPropertyByIndex(): boolean {
    let value = ESValue.wrapBoolean(false);
    jsArray1.setProperty(2, value);
    let val = jsArray1.getProperty(2);
    return val.toBoolean() === false;
}

function checkSetPropertyByIndexDouble(): boolean {
    let index = 4.2;
    let val = 'cccc';
    jsDoubledObj.setProperty(index, val);
    jsDoubledObj.setProperty(ESValue.wrap(4.5), val);
    return jsDoubledObj.getProperty(index).toString() === val &&
        jsDoubledObj.getProperty(ESValue.wrap(4.5)).toString() === val;
}

function checkSetProperty(): boolean {
    let value = ESValue.wrapNumber(5);
    let property = ESValue.wrapString('property1');
    jsObjectA.setProperty(property, value);
    let val = jsObjectA.getProperty(property);
    return val.toNumber() === 5;
}

function checkHasProperty(): boolean {
    let property = ESValue.wrapString('property2');
    return jsObjectB.hasProperty(property) === true;
}

function checkHasPropertyByName(): boolean {
    return jsObjectB.hasProperty('property2');
}

function checkTypeOf(): boolean {
    let jsType = jsString.typeOf();
    let jsNumType = jsBoxNum.typeOf();
    let jsBoolType = jsBoxBool.typeOf();
    let jsStrType = jsBoxStr.typeOf();
    return jsType === 'string' &&
        jsNumType === 'number' &&
        jsBoolType === 'boolean' &&
        jsStrType === 'string';
}

function checkInvokeNoParam(): boolean {
    let result = jsFunc.invoke();
    if (!result.isNumber()) {
        return false;
    }
    return result.toNumber() === 6;
}

function checkInvokeHasParam(): boolean {
    let args: FixedArray<(Object | null | undefined)> = [ESValue.wrapDouble(2), ESValue.wrapDouble(3)];
    let result: ESValue = jsFuncWithParam.invoke(...args);
    if (!result.isNumber()) {
        return false;
    }
    return result.toNumber() === 5;
}

function checkInvokeMethod(): boolean {
    let result = jsObjWithFunc.invokeMethod('func');
    if (!result.isString()) {
        return false;
    }
    return result.toString() === 'hello';
}

function checkInvokeMethodHasParam(): boolean {
    let args: FixedArray<ESValue> = [ESValue.wrapString('abc'), ESValue.wrapString('def')];
    let result = jsObjWithFuncParam.invokeMethod('hello', ...args);
    if (!result.isString()) {
        return false;
    }
    return result.toString() === 'abcdef';
}

function checkIterator(): boolean {
    let result: String = new String();
    for (const elem of jsIterable) {
        result += elem.toString();
    }
    return result === 'abcd';
}

function checkKeys(): boolean {
    let keys = ['a', 'b', 'c'];
    let index = 0;
    for (const key of jsIterableObject.keys()) {
        if (key.toString() !== keys[index]) {
            return false;
        }
        index++;
    }
    if (index !== 3) {
        return false;
    }
    return true;
}

function checkValues(): boolean {
    let values = [1, 2, 3];
    let index = 0;
    for (const value of jsIterableObject.values()) {
        if (value.toNumber() !== values[index]) {
            return false;
        }
        index++;
    }
    if (index !== 3) {
        return false;
    }
    return true;
}

function checkEntries(): boolean {
    let keys = ['a', 'b', 'c'];
    let values = [1, 2, 3];
    let index = 0;
    for (const entry of jsIterableObject.entries()) {
        if (entry[0].toString() !== keys[index]) {
            return false;
        }
        if (entry[1].toNumber() !== values[index]) {
            return false;
        }
        index++;
    }
    if (index !== 3) {
        return false;
    }
    return true;
}

class testObj { }
function checkInstanceOfStaticObj(): boolean {
    let staticObj = new testObj();
    let obj = ESValue.wrap(staticObj);
    return obj.instanceOf(Type.of(staticObj));
}

function checkInstanceOfNumeric(): boolean {
    let num: short = 3;
    let obj = ESValue.wrapNumber(num);
    return obj.instanceOf(Type.of(num));
}

function checkInstanceOfStaticPrimitive(): boolean {
    let boolObj = false;
    let obj = ESValue.wrapBoolean(boolObj);
    return obj.instanceOf(Type.of(boolObj));
}

function checkInstanceOfDynamic(): boolean {
    let num = ESValue.wrapInt(123);
    let user = jsUser.instantiate(num);
    return user.instanceOf(jsUser);
}

function checkInstanceOfNullOrUndefined(): boolean {
    let undefValue = module2.getProperty('undefValue');
    let nullValue = module2.getProperty('nullValue');
    let res = true;
    res = res && !nullValue.instanceOf(nullValue);
    try {
        res = res && undefValue.instanceOf(nullValue);
        res = false;
    } catch (e: Error) {
        res = res && e.message.includes('Need object');
    }
    try {
        res = res && nullValue.instanceOf(undefValue);
        res = false;
    } catch (e: Error) {
        res = res && e.message.includes('Need object');
    }
    try {
        res = res && undefValue.instanceOf(undefValue);
        res = false;
    } catch (e: Error) {
        res = res && e.message.includes('Need object');
    }
    return res;
}

function checkInstaniate(): boolean {
    let num = ESValue.wrapInt(123);
    let user = jsUser.instantiate(num);
    return user.getProperty('ID').toNumber() === 123;
}

function testUndefined(): boolean {
    propObj.setProperty('property1', undefined);
    propObj.setProperty('property2', ESValue.wrap(undefined));
    propObj.setProperty(123, undefined);
    return propObj.getProperty('property1').isUndefined() &&
        propObj.getProperty('property2').isUndefined() &&
        propObj.getProperty(123).isUndefined();
}

function checkThrowNullOrUndefined(): boolean {
    let res = false;
    try {
        throwNull.invoke();
        res = false;
    } catch (e) {
        e = e as ESError;
        res = e.getValue().toNull() === null;
    }
    try {
        throwUndefined.invoke();
        res = false;
    } catch (e) {
        e = e as ESError;
        res = e.getValue().toUndefined() === undefined;
    }
    return res;
}
