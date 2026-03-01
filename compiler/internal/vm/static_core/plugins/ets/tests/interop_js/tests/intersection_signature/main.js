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
        function ConstFoo() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (ConstFoo.prototype = b.prototype, new ConstFoo());
    };
})();
export let tsNumber = 1;
export let tsString = 'string';
export let AgeNameInterfaceClass = /** @class */ (function () {
    function AgeNameInterfaceClass(name, age) {
        throwExceptionInterface({ name: name, age: age });
        this.name = name;
        this.age = age;
    }
    AgeNameInterfaceClass.prototype.createUser = function (name, age) {
        throwExceptionInterface({ name: name, age: age });
        return new AgeNameInterfaceClass(name, age);
    };
    return AgeNameInterfaceClass;
}());
export function createAgeNameClassInterfaceFromTs() {
    new AgeNameInterfaceClass(tsString, tsNumber);
    return true;
}
export function callMethodAgeNameClassInterfaceFromTs() {
    let user = new AgeNameInterfaceClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}
export let instanceAgeNameInterfaceClass = new AgeNameInterfaceClass(tsString, tsNumber);
export let ChildAgeNameInterfaceClass = /** @class */ (function (_super) {
    __extends(ChildAgeNameInterfaceClass, _super);
    function ChildAgeNameInterfaceClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ChildAgeNameInterfaceClass;
}(AgeNameInterfaceClass));
export function createChildAgeNameClassInterfaceFromTs() {
    new ChildAgeNameInterfaceClass(tsString, tsNumber);
    return true;
}
export function callMethodChildAgeNameClassInterfaceFromTs() {
    let user = new ChildAgeNameInterfaceClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}
export let instanceChildAgeNameInterfaceClass = new ChildAgeNameInterfaceClass(tsString, tsNumber);
export function checkIntersectionInterface(arg) {
    if (checkInterfaceAge(arg) && checkInterfaceName(arg)) {
        return true;
    }
    ;
    return false;
}
export function checkInterfaceAge(object) {
    let key = 'age';
    return key in object && typeof object[key] === 'number';
}
export function checkInterfaceName(object) {
    let key = 'name';
    return key in object && typeof object[key] === 'string';
}
export function throwExceptionInterface(arg) {
    let check = checkIntersectionInterface(arg);
    if (!check) {
        throw new Error('Invalid input');
    }
}
//<< generic type
export function checkAgeNameGeneric(arg) {
    if (checkTypeGenericAge(arg) && checkTypeGenericName(arg)) {
        return true;
    }
    ;
    return false;
}
function checkTypeGenericAge(object) {
    let key = 'age';
    return key in object && typeof object[key] === 'number';
}
function checkTypeGenericName(object) {
    let key = 'name';
    return key in object && typeof object[key] === 'string';
}
export let AgeNameGenericClass = /** @class */ (function () {
    function AgeNameGenericClass(name, age) {
        throwExceptionGeneric({ name: name, age: age });
        this.name = name;
        this.age = age;
    }
    AgeNameGenericClass.prototype.createUser = function (name, age) {
        throwExceptionGeneric({ name: name, age: age });
        return new AgeNameInterfaceClass(name, age);
    };
    return AgeNameGenericClass;
}());
export function createAgeNameClassGenericFromTs() {
    new AgeNameGenericClass(tsString, tsNumber);
    return true;
}
export function callMethodAgeNameClassGenericFromTs() {
    let user = new AgeNameGenericClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}
export let instanceAgeNameGenericClass = new AgeNameGenericClass(tsString, tsNumber);
export let ChildAgeNameGenericClass = /** @class */ (function (_super) {
    __extends(ChildAgeNameGenericClass, _super);
    function ChildAgeNameGenericClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ChildAgeNameGenericClass;
}(AgeNameGenericClass));
export function createChildAgeNameClassGenericFromTs() {
    new ChildAgeNameGenericClass(tsString, tsNumber);
    return true;
}
export function callMethodChildAgeNameClassGenericFromTs() {
    let user = new ChildAgeNameGenericClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}
export let instanceChildAgeNameGenericClass = new ChildAgeNameGenericClass(tsString, tsNumber);
function throwExceptionGeneric(arg) {
    let check = checkIntersectionInterface(arg);
    if (!check) {
        throw new Error('Invalid input');
    }
}
export function checkUnionType(arg) {
    if (checkTypeAgeUnion(arg) && checkTypeNameUnion(arg)) {
        return true;
    }
    ;
    return false;
}
function checkTypeAgeUnion(object) {
    let key = 'age';
    return key in object && (typeof object[key] === 'number' || typeof object[key] === 'boolean');
}
function checkTypeNameUnion(object) {
    let key = 'name';
    return key in object && (typeof object[key] === 'string' || typeof object[key] === 'number');
}
export let AgeNameUnionClass = /** @class */ (function () {
    function AgeNameUnionClass(name, age) {
        throwExceptionUnion({ name: name, age: age });
        this.name = name;
        this.age = age;
    }
    AgeNameUnionClass.prototype.createUser = function (name, age) {
        throwExceptionUnion({ name: name, age: age });
        return new AgeNameUnionClass(name, age);
    };
    return AgeNameUnionClass;
}());
export function createAgeNameUnionClassFromTs() {
    new AgeNameUnionClass(tsString, tsNumber);
    return true;
}
export function callMethodAgeNameUnionClassFromTs() {
    let user = new AgeNameUnionClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}
export let instanceAgeNameUnionClass = new AgeNameUnionClass(tsString, tsNumber);
export let ChildAgeNameUnionClass = /** @class */ (function (_super) {
    __extends(ChildAgeNameUnionClass, _super);
    function ChildAgeNameUnionClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ChildAgeNameUnionClass;
}(AgeNameUnionClass));
export function createChildAgeNameUnionClassFromTs() {
    new ChildAgeNameUnionClass(tsString, tsNumber);
    return true;
}
export function callMethodChildAgeNameUnionClassFromTs() {
    let user = new ChildAgeNameUnionClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}
export let instanceChildAgeNameUnionClass = new ChildAgeNameUnionClass(tsString, tsNumber);
function throwExceptionUnion(arg) {
    let check = checkUnionType(arg);
    if (!check) {
        throw new Error('Invalid input');
    }
}
export function checkLiteralType(arg) {
    if (checkTypeAgeLiteral(arg) && checkTypeNameLiteral(arg)) {
        return true;
    }
    ;
    return false;
}
function checkTypeAgeLiteral(object) {
    let key = 'age';
    return key in object && (object[key] === 1 || object[key] === true);
}
function checkTypeNameLiteral(object) {
    let key = 'name';
    return key in object && (object[key] === 'string' || object[key] === 1);
}
export let AgeNameLiteralClass = /** @class */ (function () {
    function AgeNameLiteralClass(name, age) {
        throwExceptionLiteral({ name: name, age: age });
        this.name = name;
        this.age = age;
    }
    AgeNameLiteralClass.prototype.createUser = function (name, age) {
        throwExceptionLiteral({ name: name, age: age });
        return new AgeNameLiteralClass(name, age);
    };
    return AgeNameLiteralClass;
}());
export function createAgeNameLiteralClassFromTs() {
    new AgeNameLiteralClass(tsString, tsNumber);
    return true;
}
export function callMethodAgeNameLiteralClassFromTs() {
    let user = new AgeNameLiteralClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}
export let instanceAgeNameLiteralClass = new AgeNameLiteralClass(tsString, tsNumber);
export let ChildAgeNameLiteralClass = /** @class */ (function (_super) {
    __extends(ChildAgeNameLiteralClass, _super);
    function ChildAgeNameLiteralClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ChildAgeNameLiteralClass;
}(AgeNameLiteralClass));
export function createChildAgeNameLiteralClassFromTs() {
    new ChildAgeNameLiteralClass(tsString, tsNumber);
    return true;
}
export function callMethodChildAgeNameLiteralClassFromTs() {
    let user = new ChildAgeNameLiteralClass(tsString, tsNumber);
    user.createUser(tsString, tsNumber);
    return true;
}
export let instanceChildAgeNameLiteralClass = new ChildAgeNameLiteralClass(tsString, tsNumber);
function throwExceptionLiteral(arg) {
    let check = checkLiteralType(arg);
    if (!check) {
        throw new Error('Invalid input');
    }
}
