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
    let extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (let p in b) { if (Object.prototype.hasOwnProperty.call(b, p)) { d[p] = b[p]; } } };
        return extendStatics(d, b);
    };
    return function (d, b) {
        if (typeof b !== 'function' && b !== null) {
            throw new TypeError('Class extends value ' + String(b) + ' is not a constructor or null'); }
        extendStatics(d, b);
        function FuC() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (FuC.prototype = b.prototype, new FuC());
    };
})();
export let tsInt = 1;
export let NamedClass = /** @class */ (function () {
    function NamedClass(value) {
        this._value = value;
    }
    return NamedClass;
}());
export function createNamedClassFromTs() {
    return new NamedClass(tsInt);
}
export let namedClassInstance = new NamedClass(tsInt);

export let AnonymousClass = /** @class */ (function () {
    function class1(value) {
        this.value = value;
        this._value = value;
    }
    return class1;
}());
export function createAnonymousClassFromTs() {
    return new AnonymousClass(tsInt);
}
export let anonymousClassInstance = new AnonymousClass(tsInt);

export function FunctionConstructor(value) {
    this._value = value;
}
export function createFunctionConstructorFromTs() {
    return new FunctionConstructor(tsInt);
}
export let functionConstructorInstance = new FunctionConstructor(tsInt);

export let IIFEClass = (function () {
    return /** @class */ (function () {
        function class2(value) {
            this._value = value;
        }
        return class2;
    }());
})();
export function createIIFEClassFromTs() {
    return new IIFEClass(tsInt);
}
export let IIFEClassInstance = new IIFEClass(tsInt);

export let IIFEConstructor = (function () {
    function constructorFoo(value) {
        this._value = value;
    }
    return constructorFoo;
})();
export function createIIFEConstructorFromTs() {
    return new IIFEConstructor(tsInt);
}
export let IIFEConstructorInstance = new IIFEConstructor(tsInt);

export let MethodCreateConstructor = /** @class */ (function () {
    function MethodCreateConstructor() {
    }
    MethodCreateConstructor.prototype.Constructor = function () {
        return /** @class */ (function () {
            function class3(value) {
                this._value = value;
            }
            return class3;
        }());
    };
    return MethodCreateConstructor;
}());
export function createMethodConstructorClass() {
    return new MethodCreateConstructor();
}
export let methodConstructorInstance = new MethodCreateConstructor();
export let methodCreateAnonymousClass = methodConstructorInstance.Constructor();
export let methodCreateClassInstance = new methodCreateAnonymousClass(tsInt);

export let SimpleObject = {
    _value: tsInt,
};
export let simpleArrowFunction = function (int) {
    return {
        int: int,
    };
};
let Abstract = /** @class */ (function () {
    function abstract(value) {
        this._value = value;
    }
    return abstract;
}());

export let AbstractClass = /** @class */ (function (_super) {
    __extends(AbstractClass, _super);
    function AbstractClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return AbstractClass;
}(Abstract));
export function createAbstractClassFromTs() {
    return new AbstractClass(tsInt);
}
export let abstractClassInstance = new AbstractClass(tsInt);

export let ChildClass = /** @class */ (function (_super) {
    __extends(ChildClass, _super);
    function ChildClass(value) {
        return _super.call(this, value) || this;
    }
    return ChildClass;
}(NamedClass));
export function createChildClassFromTs() {
    return new ChildClass(tsInt);
}
export let childClassInstance = new ChildClass(tsInt);
