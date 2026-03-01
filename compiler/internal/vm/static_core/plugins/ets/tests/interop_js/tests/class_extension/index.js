'use strict';
/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */
import { EventEmitter } from 'stream';
const etsVm = globalThis.gtest.etsVm;
const ETS_MODULE_NAME = 'interop_class_extension';
const makePrefix = (packageName) => 'L' + packageName.replace('.', '/') + '/';
const getClass = (name, packageName) => etsVm.getClass(makePrefix(packageName !== null && packageName !== void 0 ? packageName : ETS_MODULE_NAME) + name + ';');
export const DEFAULT_STRING_VALUE = 'Panda';
export const DEFAULT_NUMERIC_VALUE = Math.round(Math.random() * Number.MAX_SAFE_INTEGER);
export const helperIsJSInstanceOf = (obj, classObj) => {
    return obj instanceof classObj;
};
export class TestUserClass {
    static isMyInstance(obj) {
        return obj instanceof TestUserClass;
    }
    constructor(constructorArg) {
        this._privateValue = DEFAULT_STRING_VALUE;
        this.protectedValue = DEFAULT_STRING_VALUE;
        this.publicValue = DEFAULT_STRING_VALUE;
        this.readonlyValue = DEFAULT_STRING_VALUE;
        this.constructorSetValue = constructorArg;
    }
}
export const TestNativeClass = EventEmitter;
export const extendUserClass = () => {
    const TSTestUserClass = getClass('TSTestUserClass');
    class ExtendedEtsUserClass extends TSTestUserClass {
    }
    const classInstance = new ExtendedEtsUserClass();
    return classInstance instanceof ExtendedEtsUserClass;
};
export const extendNativeClass = () => {
    const TSTestNativeClass = getClass('ArrayBuffer', 'escompat');
    class ExtendedEtsNativeClass extends TSTestNativeClass {
        constructor(length) {
            super(length);
        }
    }
    const classInstance = new ExtendedEtsNativeClass(2);
    return classInstance instanceof TSTestNativeClass;
};
export const jsRespectsProtectedModifier = () => {
    const TSTestUserClass = getClass('TSTestUserClass');
    try {
        class ExtendedEtsUserClass extends TSTestUserClass {
            constructor() {
                super();
                this.protectedProperty = 'redefinedProtected';
            }
            validProtectedGetter() {
                return this.protectedProperty;
            }
        }
        const classInstance = new ExtendedEtsUserClass();
        return classInstance.validProtectedGetter() === 'redefinedProtected';
    }
    catch (e) {
        return false;
    }
};
export const jsRespectsStaticModifier = () => {
    const TSTestUserClass = getClass('TSTestUserClass');
    class ExtendedEtsUserClass extends TSTestUserClass {
    }
    return (ExtendedEtsUserClass === null || ExtendedEtsUserClass === void 0 ? void 0 : ExtendedEtsUserClass.staticProperty) === 'static';
};
