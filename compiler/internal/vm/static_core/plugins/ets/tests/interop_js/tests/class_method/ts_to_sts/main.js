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
        function ConstrFoo() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (ConstrFoo.prototype = b.prototype, new ConstrFoo());
    };
})();
export let tsInt = 1;
export let UserClass = /** @class */ (function () {
    function UserClass() {
        this.value = tsInt;
    }
    UserClass.prototype.get = function () {
        return this.value;
    };
    UserClass.prototype.compare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    return UserClass;
}());
export function createUserClassFromTs() {
    return new UserClass();
}
export let instanceUserClass = new UserClass();

export let ChildClass = /** @class */ (function (_super) {
    __extends(ChildClass, _super);
    function ChildClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    ChildClass.prototype.childMethodCompare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    return ChildClass;
}(UserClass));
export function createChildClassFromTs() {
    return new ChildClass();
}
export let instanceChildClass = new ChildClass();

export let InterfaceClass = /** @class */ (function () {
    function InterfaceClass() {
        this.value = tsInt;
    }
    InterfaceClass.prototype.get = function () {
        return this.value;
    };
    InterfaceClass.prototype.compare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    return InterfaceClass;
}());
export function createInterfaceClassFromTs() {
    return new InterfaceClass();
}
export let instanceInterfaceClass = new InterfaceClass();

export let StaticClass = /** @class */ (function () {
    function staticClass() {
    }
    staticClass.get = function () {
        return this.value;
    };
    staticClass.compare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    staticClass.value = tsInt;
    return staticClass;
}());
export function createStaticClassFromTs() {
    return StaticClass;
}

export let PrivateClass = /** @class */ (function () {
    function PrivateClass() {
        this.value = tsInt;
    }
    PrivateClass.prototype.get = function () {
        return this.value;
    };
    PrivateClass.prototype.compare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    return PrivateClass;
}());
export function createPrivateClassFromTs() {
    return new PrivateClass();
}
export let instancePrivateClass = new PrivateClass();

export let ProtectedClass = /** @class */ (function () {
    function ProtectedClass() {
        this.value = tsInt;
    }
    ProtectedClass.prototype.get = function () {
        return this.value;
    };
    ProtectedClass.prototype.compare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    return ProtectedClass;
}());
export function createProtectedClassFromTs() {
    return new ProtectedClass();
}
export let instanceProtectedClass = new ProtectedClass();

export let ChildProtectedClass = /** @class */ (function (_super) {
    __extends(ChildProtectedClass, _super);
    function ChildProtectedClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ChildProtectedClass;
}(ProtectedClass));
export function createChildProtectedClassFromTs() {
    return new ChildProtectedClass();
}
export let instanceChildProtectedClass = new ProtectedClass();

let AbstractClass = /** @class */ (function () {
    function abstractClass() {
        this.value = tsInt;
    }
    abstractClass.prototype.get = function () {
        return this.value;
    };
    abstractClass.prototype.compare = function (x) {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }
        return false;
    };
    return abstractClass;
}());
export let ChildAbstractClass = /** @class */ (function (_super) {
    __extends(ChildAbstractClass, _super);
    function ChildAbstractClass(...arg) {
        return _super !== null && _super.apply(this, arg) || this;
    }
    return ChildAbstractClass;
}(AbstractClass));

export function createChildAbstractClassFromTs() {
    return new ChildAbstractClass();
}
export let instanceChildAbstractClass = new ChildAbstractClass();
