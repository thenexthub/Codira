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
export let tsString = 'string';
export let tsBool = true;
export let tsArr = [tsNumber];
export let tsObj = {};
export let tsUnion = tsNumber;
export let tsTuple = [tsNumber, tsBool];
export let tsLiteral = tsNumber;
export function anyTypeParameter(arg) {
    return arg;
}
export function anyTypeParameterExplicitCallFromTsInt() {
    return anyTypeParameter(tsNumber);
}
export function anyTypeParameterExplicitCallFromTsString() {
    return anyTypeParameter(tsString);
}
export function anyTypeParameterExplicitCallFromTsBool() {
    return anyTypeParameter(tsBool);
}
export function anyTypeParameterExplicitCallFromTsArr() {
    return anyTypeParameter(tsArr);
}
export function anyTypeParameterExplicitCallFromTsObj() {
    return anyTypeParameter(tsObj);
}
export function anyTypeParameterExplicitCallFromTsUnion() {
    return anyTypeParameter(tsUnion);
}
export function anyTypeParameterExplicitCallFromTsTuple() {
    return anyTypeParameter(tsTuple);
}
export function anyTypeParameterExplicitCallFromTsLiteral() {
    return anyTypeParameter(tsLiteral);
}
export let genericTypeFunctionReturnAny = function (arg) {
    return arg;
};
export function genericTypeFunctionExplicitCallFromTsInt() {
    return (0, genericTypeFunctionReturnAny)(tsNumber);
}
export function genericTypeFunctionExplicitCallFromTsString() {
    return (0, genericTypeFunctionReturnAny)(tsString);
}
export function genericTypeFunctionExplicitCallFromTsBool() {
    return (0, genericTypeFunctionReturnAny)(tsBool);
}
export function genericTypeFunctionExplicitCallFromTsArr() {
    return (0, genericTypeFunctionReturnAny)(tsArr);
}
export function genericTypeFunctionExplicitCallFromTsObj() {
    return (0, genericTypeFunctionReturnAny)(tsObj);
}
export function genericTypeFunctionExplicitCallFromTsUnion() {
    return (0, genericTypeFunctionReturnAny)(tsUnion);
}
export function genericTypeFunctionExplicitCallFromTsTuple() {
    return (0, genericTypeFunctionReturnAny)(tsTuple);
}
export function genericTypeFunctionExplicitCallFromTsLiteral() {
    return (0, genericTypeFunctionReturnAny)(tsLiteral);
}
export let extendGenericNumber = function (arg) { return arg; };
export let extendGenericString = function (arg) { return arg; };
export let extendGenericBool = function (arg) { return arg; };
export let extendGenericArr = function (arg) { return arg; };
export let extendGenericObj = function (arg) { return arg; };
export let extendGenericUnion = function (arg) { return arg; };
export let extendGenericTuple = function (arg) { return arg; };
export let extendGenericLiteral = function (arg) { return arg; };
export let GInterfaceClass = /** @class */ (function () {
    function gInterfaceClass() {
        this.data = 'string';
    }
    return gInterfaceClass;
}());
export let GTypeClass = /** @class */ (function () {
    function gTypeClass() {
        this.data = 'string';
    }
    return gTypeClass;
}());
export function genericExtendInterface(arg) {
    return arg;
}
export function genericExtendType(arg) {
    return arg;
}
export function tupleGeneric(arg) {
    return [arg[1], arg[0]];
}
export function collectGeneric(...arg) {
    let args = [];
    for (let _i = 0; _i < arg.length; _i++) {
        args[_i] = arg[_i];
    }
    return args;
}
export let UserClass = /** @class */ (function () {
    function userClass(name, age) {
        this.name = name;
        this.age = age;
    }
    userClass.prototype.getName = function () {
        return this.name;
    };
    userClass.prototype.getAge = function () {
        return this.age;
    };
    return userClass;
}());
export function createClassFromTs() {
    return new UserClass(tsString, tsNumber);
}
export let userClassInstance = new UserClass(tsString, tsNumber);
export function genericDefaultInt(arg) {
    return arg;
}
export function genericDefaultString(arg) {
    return arg;
}
export function genericDefaultBool(arg) {
    return arg;
}
export function genericDefaultArr(arg) {
    return arg;
}
export function genericDefaultObj(arg) {
    return arg;
}
export function genericDefaultUnion(arg) {
    return arg;
}
export function genericDefaultTuple(arg) {
    return arg;
}
export function genericDefaultLiteral(arg) {
    return arg;
}
export function genericDefaultIntCallFromTs() {
    return genericDefaultInt(tsNumber);
}
export function genericDefaultStringCallFromTs() {
    return genericDefaultString(tsString);
}
export function genericDefaultBoolCallFromTs() {
    return genericDefaultBool(tsBool);
}
export function genericDefaultArrCallFromTs() {
    return genericDefaultArr(tsArr);
}
export function genericDefaultObjCallFromTs() {
    return genericDefaultObj(tsObj);
}
export function genericDefaultUnionCallFromTs() {
    return genericDefaultUnion(tsUnion);
}
export function genericDefaultTupleCallFromTs() {
    return genericDefaultTuple(tsTuple);
}
export function genericDefaultLiteralCallFromTs() {
    return genericDefaultLiteral(tsLiteral);
}
export let DataClass = /** @class */ (function () {
    function dataClass() {
        this.data = 'string';
    }
    return dataClass;
}());
export function genericExtendClass(arg) {
    return arg;
}
