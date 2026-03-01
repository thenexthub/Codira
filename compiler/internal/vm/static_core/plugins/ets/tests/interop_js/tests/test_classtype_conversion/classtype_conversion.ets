/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let module = ESValue.load('js_code');

let jsBoolean = module.getProperty('b');
let jsNumber = module.getProperty('n');
let jsString = module.getProperty('s');
let jsUndefined = module.getProperty('jsUndefined');
let jsBigInt = module.getProperty('bi');
let jsNull = module.getProperty('jsNull');
let jsFunc = module.getProperty('jsFunc');
let jsSymbol = module.getProperty('symbolTest');
let jsArray = module.getProperty('jsArray');
let jsObjectA = module.getProperty('A');
let externalObj = module.getProperty('externalObj');

function checkIsBoolean(): boolean {
    return jsBoolean.isBoolean();
}

function checkIsNumber(): boolean {
    return jsNumber.isNumber();
}

function checkIsString(): boolean {
    return jsString.isString();
}

function checkIsBigInt(): boolean {
    return jsBigInt.isBigInt();
}

function checkIsUndefined(): boolean {
    
    let obj = ESValue.Undefined;
    
    return obj.typeOf() == jsUndefined.typeOf();

}

function checkIsFunction(): boolean {
    return jsFunc.isFunction();
}

function checkTypeOfBoolean(): boolean {
    return jsBoolean.typeOf() === 'boolean';
}

function checkTypeOfNumber(): boolean {
    return jsNumber.typeOf() === 'number';
}

function checkTypeOfString(): boolean {
    return jsString.typeOf() === 'string';
}

function checkTypeOfBigInt(): boolean {
    return jsBigInt.typeOf() === 'bigint';
}

function checkTypeOfUndefined(): boolean {
    return jsUndefined.typeOf() === 'undefined';
}

function checkTypeOfNull(): boolean {
    return jsNull.typeOf() === 'object'; 
}

function checkTypeOfFunction(): boolean {
    return jsFunc.typeOf() === 'function';
}

function checkTypeOfSymbol(): boolean {
    return jsSymbol.typeOf() === 'object';
}

function checkTypeOfArray(): boolean {
    return jsArray.typeOf() === 'object';
}

function checkTypeOfObject(): boolean {
    return jsObjectA.typeOf() === 'object';
}

function checkTypeOfExternal(): boolean {
    return externalObj.typeOf() === 'object';
}

