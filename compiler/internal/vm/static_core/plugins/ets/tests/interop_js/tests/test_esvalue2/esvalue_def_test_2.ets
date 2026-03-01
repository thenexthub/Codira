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

let jsBoolean = module.getProperty('b');
let jsNumber = module.getProperty('n');
let jsString = module.getProperty('s');
let jsUndefined = module.getProperty('jsUndefined');
let jsBigInt = module.getProperty('bi');
let jsNull = module.getProperty('jsNull');
let jsFunc = module.getProperty('jsFunc');
let jsFuncThrowNumber = module.getProperty('jsFuncThrowNumber');
let jsFuncThrowString = module.getProperty('jsFuncThrowString');
let jsFuncNoReturn = module.getProperty('jsFuncNoReturn');
let jsFuncReturnNull = module.getProperty('jsFuncReturnNull');
let jsFuncReturnUndefined = module.getProperty('jsFuncReturnUndefined');
let jsFuncWithParam = module.getProperty('jsFuncWithParam');
let jsFuncWithParamThrowNumber = module.getProperty('jsFuncWithParamThrowNumber');
let jsFuncWithParamThrowString = module.getProperty('jsFuncWithParamThrowString');
let jsArray = module.getProperty('jsArray');
let jsArray1 = module.getProperty('jsArray1');
let jsObjectA = module.getProperty('A');
let jsObjectB = module.getProperty('B');
let jsObjWithFunc = module.getProperty('objWithFunc');
let jsObjWithFuncParam = module.getProperty('objWithFuncParam');
let jsIterable = module.getProperty('jsIterable');
let jsIterableObject = module.getProperty('testItetatorObject');
let jsUser = module.getProperty('User');
let jsUserThrowNumber = module.getProperty('UserThrowNumber');
let jsUserThrowString = module.getProperty('UserThrowString');
let jsDoubledObj = module.getProperty('doubledObj');

// get Undefined(): ESValue
function checkGetUndefined(): boolean {
    let obj = ESValue.Undefined;
    return obj.typeOf() === 'undefined';
}

// get Null(): ESValue
function checkGetNull(): boolean {
    let obj = ESValue.Null;
    return obj.isNull();
}

// wrapBoolean(b: boolean): ESValue
function checkWrapBoolean(): boolean {
    let val = false;
    let obj = ESValue.wrapBoolean(val);
    return obj.toBoolean() === val;
}

// wrapString(s: string): ESValue
function checkWrapString(): boolean {
    let val = 'hello';
    let obj = ESValue.wrapString(val);
    return obj.toString() === val;
}

// wrapBigInt(bi: BigInt): ESValue
function checkWrapBigInt(): boolean {
    let val = new BigInt(9007199254740991n);
    let obj = ESValue.wrapBigInt(val);
    return obj.toBigInt() === val;
}

// wrapNumber(n: number): ESValue
function checkWrapNumber(): boolean {
    let val = 111;
    let obj = ESValue.wrapNumber(val);
    return obj.toNumber() === val;
}

// wrapDouble(d: double): ESValue
function checkWrapDouble(): boolean {
    let val: double = 111;
    let obj = ESValue.wrapDouble(val);
    return obj.toNumber() === val;
}

// isBoolean(eo: ESValue): boolean
function checkIsBoolean(): boolean {
    return jsBoolean.isBoolean();
}

// isString(eo: ESValue): boolean
function checkIsString(): boolean {
    return jsString.isString();
}

// isNumber(eo: ESValue): boolean
function checkIsNumber(): boolean {
    return jsNumber.isNumber();
}

// isBigInt
function checkIsBigInt(): boolean {
    return jsBigInt.isBigInt();
}

// isUndefined
function checkIsUndefined(): boolean {
    let obj = ESValue.Undefined;
    return obj.isUndefined();
}

// isFunction
function checkIsFunction(): boolean {
    return jsFunc.isFunction();
}

// toBoolean(eo: ESValue): boolean
function checkToBoolean(): boolean {
    return jsBoolean.toBoolean() === false;
}

// toString(): string
function checkToString(): boolean {
    return jsString.toString() === 'hello';
}

// toNumber(): number
function checkToNumber(): boolean {
    return jsNumber.toNumber() === 1;
}

// toBigInt(): bigint
function checkToBigInt(): boolean {
    return jsBigInt.toBigInt() === 9007199254740991n;
}

// toUndefined(): undefined
function checkToUndefined(): boolean {
    return jsUndefined.toUndefined() === undefined;
}

function checkJsIsUndefined(): boolean {
    return jsUndefined.isUndefined();
}

function checkTypeOfUndefined(): boolean {
    return jsUndefined.typeOf() === 'undefined';
}

function checkStrictlyEqualUndefined(): boolean {
    return ESValue.areStrictlyEqual(jsUndefined, ESValue.Undefined);
}

// toNull(): null
function checkToNull(): boolean {
    return jsNull.toNull() === null;
}

function checkIsNull(): boolean {
    return jsNull.isNull();
}

function checkStrictlyEqualNull(): boolean {
    return ESValue.areStrictlyEqual(jsNull, ESValue.Null);
}

function checkAreStrictEqual(): boolean {
    return ESValue.areStrictlyEqual(jsFunc, jsFunc);
}

// isEqualTo(other: ESValue): boolean
function checkIsEqualTo(): boolean {
    return jsFunc.isEqualTo(jsFunc);
}

// isEqualToSafe(other: ESValue): boolean
function checkIsEqualToSafe(): boolean {
    let nullObj1 = ESValue.Null;
    let nullObj2 = ESValue.Null;
    return nullObj1.isEqualToSafe(nullObj2);
}

// getProperty(name: string): ESValue
function checkGetPropertyByName(): boolean {
    let val = jsObjectA.getProperty('property2');
    return val.toNumber() === 2;
}

class StaticObjectA {
    aaa: string = '';
}
// getProperty(name: string): ESValue
function checkGetPropertyStaticObj(): boolean {
    let sObj = new StaticObjectA();
    sObj.aaa = 'bbb';
    let obj = ESValue.wrap(sObj);
    let value = obj.getPropertySafe('aaa');
    return value.toString() === 'bbb';
}

// getPropertySafe(name: string): ESValue
function checkGetPropertyByNameSafe(): boolean {
    let val = jsObjectA.getPropertySafe('property3');
    return val.isUndefined() === true;
}

// getProperty(index: number): ESValue
function checkGetPropertyByIndex(): boolean {
    let val = jsArray.getProperty(2);
    return val.toBoolean() === true;
}

// getProperty(index: number): ESValue
function checkGetPropertyByIndexDouble(): boolean {
    let index = 3.2;
    let val = jsArray.getPropertySafe(3.2);
    let val1 = jsDoubledObj.getProperty(index);
    return val1.toString() === 'aaa';
}

// getPropertySafe(index: number): ESValue
function checkGetPropertyByIndexSafe(): boolean {
    let val = jsArray.getPropertySafe(3);
    return val.isUndefined() === true;
}

// getProperty(property: ESValue): ESValue
function checkGetProperty(): boolean {
    let peroperty = ESValue.wrapString('property2');
    let val = jsObjectA.getProperty(peroperty);
    return val.toNumber() === 2;
}

// getPropertySafe(property: ESValue): ESValue
function checkGetPropertySafe(): boolean {
    let peroperty = ESValue.wrapString('property3');
    let val = jsObjectA.getPropertySafe(peroperty);
    return val.isUndefined() === true;
}

// setProperty(name: string, value: ESValue): void
function checkSetPropertyByName(): boolean {
    let value = ESValue.wrapNumber(5);
    jsObjectA.setProperty('property1', value);
    let val = jsObjectA.getProperty('property1');
    return val.toNumber() === 5;
}

// setProperty(index: number, value: StaticOrESValue): void
function checkSetPropertyByIndex(): boolean {
    let value = ESValue.wrapBoolean(false);
    jsArray1.setProperty(2, value);
    let val = jsArray1.getProperty(2);
    return val.toBoolean() === false;
}

// setProperty(property: ESValue, value: ESValue): void
function checkSetProperty(): boolean {
    let value = ESValue.wrapNumber(8);
    let property = ESValue.wrapString('property1');
    jsObjectA.setProperty(property, value);
    let val = jsObjectA.getProperty(property);
    return val.toNumber() === 8;
}

// hasProperty(property: ESValue): boolean
function checkHasProperty(): boolean {
    let property = ESValue.wrapString('property2');
    return jsObjectB.hasProperty(property) === true;
}

// hasProperty(name: String): boolean
function checkHasPropertyByName(): boolean {
    return jsObjectB.hasProperty('property2');
}

// typeOf(): String
function checkTypeOf(): boolean {
    let jsType = jsString.typeOf();
    return jsType === 'string';
}

// public invoke(...args: FixedArray<ESValue>): ESValue
function checkInvokeNoParam(): boolean {
    let result = jsFunc.invoke();
    if (!result.isNumber()) {
        return false;
    }
    return result.toNumber() === 6;
}

function checkInvokeNoParamThrowNumber(): boolean {
    let res: boolean;
    try {
        jsFuncThrowNumber.invoke();
        res = false;
    } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        res = errObj.toNumber() === 5;
    }
    return res;
}

function checkInvokeNoParamThrowString(): boolean {
    let res: boolean;
    try {
        jsFuncThrowString.invoke();
        res = false;
    } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        res = errObj.toString() === 'made error';
    }
    return res;
}

function checkInvokeNoReturn(): boolean {
    let result = jsFuncNoReturn.invoke();
    return result.isUndefined();
}

function checkInvokeReturnNull(): boolean {
    let result = jsFuncReturnNull.invoke();
    return result.isNull();
}

function checkInvokeReturnUndefined(): boolean {
    let result = jsFuncReturnUndefined.invoke();
    return result.isUndefined();
}

// public invoke(...args: FixedArray<ESValue>): ESValue
function checkInvokeHasParam(): boolean {
    let args: FixedArray<(Object | null | undefined)> = [ESValue.wrapDouble(2), ESValue.wrapDouble(3)];
    let result: ESValue = jsFuncWithParam.invoke(...args);
    if (!result.isNumber()) {
        return false;
    }
    return result.toNumber() === 5;
}

function checkInvokeHasParamThrowNumber(): boolean {
    let res: boolean;
    try {
        let args: FixedArray<(Object | null | undefined)> = [ESValue.wrapDouble(2), ESValue.wrapDouble(3)];
        jsFuncWithParamThrowNumber.invoke(...args);
        res = false;
    } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        res = errObj.toNumber() === 5;
    }
    return res;
}

function checkInvokeHasParamThrowString(): boolean {
    let res: boolean;
    try {
        let args: FixedArray<(Object | null | undefined)> = [ESValue.wrapDouble(2), ESValue.wrapDouble(3)];
        jsFuncWithParamThrowString.invoke(...args);
        res = false;
    } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        res = errObj.toString() === 'made error';
    }
    return res;
}

function checkInvokeMethod(): boolean {
    let result = jsObjWithFunc.invokeMethod('func');
    if (!result.isString()) {
        return false;
    }
    return result.toString() === 'hello';
}

function checkInvokeMethodThrowNumber(): boolean {
    let res: boolean;
    try {
        jsObjWithFunc.invokeMethod('funcThrowNumber');
        res = false;
    } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        res = errObj.toNumber() === 5;
    }
    return res;
}

function checkInvokeMethodThrowString(): boolean {
    let res: boolean;
    try {
        jsObjWithFunc.invokeMethod('funcThrowString');
        res = false;
    } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        res = errObj.toString() === 'made error';
    }
    return res;
}

function checkInvokeMethodHasParam(): boolean {
    let args: FixedArray<ESValue> = [ESValue.wrapString('abc'), ESValue.wrapString('def')];
    let result = jsObjWithFuncParam.invokeMethod('hello', ...args);
    if (!result.isString()) {
        return false;
    }
    return result.toString() === 'abcdef';
}

function checkInvokeMethodHasParamThrowNumber(): boolean {
    let res: boolean;
    try {
        let args: FixedArray<ESValue> = [ESValue.wrapString('abc'), ESValue.wrapString('def')];
        jsObjWithFuncParam.invokeMethod('helloThrowNumber', ...args);
        res = false;
    } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        res = errObj.toNumber() === 5;
    }
    return res;
}

function checkInvokeMethodHasParamThrowString(): boolean {
    let res: boolean;
    try {
        let args: FixedArray<ESValue> = [ESValue.wrapString('abc'), ESValue.wrapString('def')];
        jsObjWithFuncParam.invokeMethod('helloThrowString', ...args);
        res = false;
    } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        res = errObj.toString() === 'made error';
    }
    return res;
}

function checkIterator(): boolean {
    let result: String = '';
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

class TestObj {}
function checkInstanceOfStaticObj(): boolean {
    let staticObj = new TestObj();
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

function checkInstaniate(): boolean {
	let num = ESValue.wrapInt(123);
	let user = jsUser.instantiate(num);
    return user.getProperty('ID').toNumber() === 123;
}

function checkInstaniateThrowNumber(): boolean {
    let res: boolean;
    try {
	    jsUserThrowNumber.instantiate();
        res = false;
    } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        res = errObj.toNumber() === 5;
    }
    return res;
}

function checkInstaniateThrowString(): boolean {
    let res: boolean;
    try {
	    jsUserThrowString.instantiate();
        res = false;
    } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        res = errObj.toString() === 'made error';
    }
    return res;
}

class Foo {
    public bar(): number {
        return 0x55aa;
    }
}

function checkCallEtsFromJS(): boolean {
    let jsFuncFromJS = module.getProperty('callEtsFunc');
    let param = new Foo();
    let ret = jsFuncFromJS.invoke(ESValue.wrap(param)).toNumber();
    return (ret === 0x55aa);
}
