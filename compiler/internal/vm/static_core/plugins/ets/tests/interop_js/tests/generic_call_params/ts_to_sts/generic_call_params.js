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
let __extends = (this && this.__extends) || (function () {
    let ExtendStatics = function (d, b) {
        ExtendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (let p in b) { if (Object.prototype.hasOwnProperty.call(b, p)) { d[p] = b[p]; } } };
        return ExtendStatics(d, b);
    };
    return function (d, b) {
        if (typeof b !== 'function' && b !== null) {
            throw new TypeError('Class extends value ' + String(b) + ' is not a constructor or null'); }
        ExtendStatics(d, b);
        function ConstrFoo() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (ConstrFoo.prototype = b.prototype, new ConstrFoo());
    };
})();
export let tsString = 'string';
export let tsInt = 1;
let add = function (a, b) { return a + b; };
export function applyFunctionGenericUnion(value1, value2, func) {
    return func(value1, value2);
}
export function applyFunctionGeneric(value1, value2, func) {
    return func(value1, value2);
}
export function applyFunctionGenericTuple(value1, value2, func) {
    return func(value1, value2);
}
export let Vector = /** @class */ (function () {
    function vector(x) {
        this.x = x;
    }
    vector.prototype.add = function () {
        this.x = this.x + this.x;
    };
    vector.prototype.get = function () {
        return this.x;
    };
    return vector;
}());
let Parent = /** @class */ (function () {
    function parent() {
    }
    parent.prototype.get = function (a, b, fu) {
        return fu(a, b);
    };
    return parent;
}());
export let ClassSubset = /** @class */ (function (_super) {
    __extends(ClassSubset, _super);
    function ClassSubset(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ClassSubset;
}(Parent));
export function subsetClassCallFromTs() {
    let GClass = new ClassSubset();
    return GClass.get(tsInt, tsInt, add);
}
export function genericExtendsClass(value1) {
    value1.add();
    return value1;
}
export function genericExtendsClassCallFromTs(arg) {
    return genericExtendsClass(arg);
}
export function applyFunctionGenericArray(values, func) {
    let result = values[0];
    for (let i = 1; i < values.length; i++) {
        result = func(result, values[i]);
    }
    return result;
}
export function applyFunctionGenericArrayCallFromTs(array) {
    let add = function (a, b) { return a + b; };
    return applyFunctionGenericArray(array, add);
}
export function applyFunWithConstraints(value1, value2, func) {
    return func(value1, value2);
}
export function genericKeyof(obj, keys) {
    return {
        obj: obj,
        keys: keys,
    };
}
