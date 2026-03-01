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
export const tsString = 'string';
export const tsNumber = 1;
export class LiteralClass {
    constructor(value) {
        this._value = value;
    }
    set(arg) {
        this._value = arg;
    }
    get() {
        return this._value;
    }
}
export class UnionClass {
    constructor(value) {
        this._value = value;
    }
    set(arg) {
        this._value = arg;
    }
    get() {
        return this._value;
    }
}
export class InterfaceClass {
    constructor(value) {
        this.value = value;
    }
    set(arg) {
        this.value = arg;
    }
    get() {
        return this.value;
    }
}
export class GAbstract {
    constructor(value) {
        this._value = value;
    }
    set(arg) {
        this._value = arg;
    }
    get() {
        return this._value;
    }
}
export class AbstractClass extends GAbstract {
    constructor(value) {
        super(value);
    }
}
export class GClass {
    constructor(content) {
        this.content = content;
    }
    get() {
        return this.content;
    }
}
export function genericFunction(arg) {
    return arg;
}
export function tupleDeclaredType(items) {
    return items;
}
export function genericSubsetRef(items) {
    return items;
}
export const explicitlyDeclaredType = () => {
    return tsString;
};
export const literalClass = new LiteralClass(tsString);
export const unionClass = new UnionClass(tsString);
export const interfaceClass = new InterfaceClass(tsString);
export const abstractClass = new AbstractClass(tsString);
