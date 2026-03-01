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
export let tsInt = 1;
export let tsString = 'str';
export let tsBool = true;
export let tsArr = [tsInt];
export let tsLiteralStart = 'start';
export let tsLiteralStop = 'stop';
export let tsTuple = [tsInt, tsBool];
export let BaseObj = { a: tsInt };
export let user = { tsInt: tsInt };
export let GenericStatic = /** @class */ (function () {
    function genericStatic() {
    }
    genericStatic.get = function (value) {
        return value;
    };
    return genericStatic;
}());
export function genericStaticMethodCallFromTs(arg) {
    return GenericStatic.get(arg);
}
export let GenericExtends = /** @class */ (function (_super) {
    __extends(genericExtends, _super);
    function genericExtends(...args) {
        return _super !== null && _super.apply(this, args) || this;
    }
    return genericExtends;
}(GenericStatic));

export function genericClassExtendsCallFromTs(arg) {
    return GenericExtends.get(arg);
}
export let LiteralStatic = /** @class */ (function () {
    function literalStatic() {
    }
    literalStatic.get = function (value) {
        return value;
    };
    return literalStatic;
}());
export function genericLiteralCallFromTs(arg) {
    return LiteralStatic.get(arg);
}
export let ExtraSetStatic = /** @class */ (function () {
    function extraSetStatic() {
    }
    extraSetStatic.get = function (value) {
        return value[0];
    };
    return extraSetStatic;
}());
export function genericExtraSetCallFromTs(arg) {
    return ExtraSetStatic.get(arg);
}
export let SubsetByRefStatic = /** @class */ (function () {
    function subsetByRefStatic() {
    }
    subsetByRefStatic.get = function (value) {
        return value;
    };
    return subsetByRefStatic;
}());
export function genericSubsetByRefStaticCallFromTs(arg) {
    return SubsetByRefStatic.get(arg);
}
export let SubsetByValueStatic = /** @class */ (function () {
    function subsetByValueStatic() {
    }
    subsetByValueStatic.get = function (value) {
        return value;
    };
    return subsetByValueStatic;
}());
export function genericSubsetByValueStaticCallFromTs(arg) {
    return SubsetByValueStatic.get(arg);
}
export let UnionStatic = /** @class */ (function () {
    function unionStatic() {
    }
    unionStatic.get = function (value) {
        return value;
    };
    return unionStatic;
}());
export function genericUnionStaticCallFromTs(arg) {
    return UnionStatic.get(arg);
}
export let User = /** @class */ (function () {
    function user(value) {
        this.value = value;
    }
    return user;
}());
export let UserClassStatic = /** @class */ (function () {
    function userClassStatic() {
    }
    userClassStatic.get = function (value) {
        return value;
    };
    return userClassStatic;
}());
export let checkInstance = function (a, b) {
    return a instanceof b;
};
export function userClassFromTs(arg) {
    return UserClassStatic.get(arg);
}
export let InterfaceStatic = /** @class */ (function () {
    function interfaceStatic() {
    }
    interfaceStatic.get = function (value) {
        return value;
    };
    return interfaceStatic;
}());
export function userInterfaceFromTs(arg) {
    return InterfaceStatic.get(arg);
}
