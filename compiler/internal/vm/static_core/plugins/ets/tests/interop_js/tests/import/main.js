'use strict';
/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

export let tsNumber = 1;
export let tsString = 'Alice';
export let tsTrue = true;
export let tsFalse = false;
export let tsNull = null;
export let tsSymbol = Symbol('id');
export let tsNewSymbol = Symbol('new');
export let tsBigInt = 9007199254740991n;
export let tsUndefined = undefined;
export function increment() {
    return tsNumber++;
}
export function concatString() {
    return tsString += tsString;
}
export function changeTrue() {
    return tsTrue = !tsTrue;
}
export function changeFalse() {
    return tsFalse = !tsFalse;
}
export function changeUndefined() {
    return tsUndefined = tsNumber;
}
export function changeNull() {
    return tsNull = tsNumber;
}
export function changeSymbol() {
    return tsSymbol = tsNewSymbol;
}
export function changeBigInt() {
    return tsBigInt = tsBigInt - tsBigInt;
}
//Objects
export let TsClass = /** @class */ (function () {
    function tsClass() {
        this.name = tsString;
    }
    return tsClass;
}());
export function changeTsClass(obj) {
    print(obj, 'TS LOG<<');
    obj.name = tsString + tsString;
}
export let tsObject = {
    value: 'tsString',
};
export function changeObject() {
    tsObject.value = tsString + tsString;
}
export let arr = [tsNumber];
export function changeArray() {
    arr.push(tsNumber);
}
export let tsFunction = function () { return tsNumber; };

export function changeFunction() {
    tsFunction = function () { return tsNumber + tsNumber; };
}
export let tsDate = new Date();
export function changeDate() {
    tsDate.setFullYear(tsDate.getFullYear() + 1);
}

export let tsReg = new RegExp('abc');
export function changeRegExp() {
    tsReg = new RegExp('xyz');
}

export function defaultFu() {
    return tsNumber;
}
export let User = /** @class */ (function () {
    function user() {
    }
    return user;
}());
