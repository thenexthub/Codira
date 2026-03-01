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
export class PublicGetterClass {
    constructor() {
        this._value = tsString;
    }
    get value() {
        return this._value;
    }
}
export function createPublicGetterClassFromTs() {
    return new PublicGetterClass();
}
export const publicGetterInstanceClass = new PublicGetterClass();
export class ProtectedGetterOrigenClass {
    constructor() {
        this._value = tsString;
    }
    get value() {
        return this._value;
    }
}
export function createProtectedGetterOrigenClassFromTs() {
    return new ProtectedGetterOrigenClass();
}
export const protectedGetterOrigenInstanceClass = new ProtectedGetterOrigenClass();
export class ProtectedGetterInheritanceClass extends ProtectedGetterOrigenClass {
}
export function createProtectedGetterInheritanceClassFromTs() {
    return new ProtectedGetterInheritanceClass();
}
export const protectedGetterInstanceInheritanceClass = new ProtectedGetterInheritanceClass();
export class PrivateGetterClass {
    constructor() {
        this._value = tsString;
    }
    get value() {
        return this._value;
    }
}
export function createPrivateGetterClassFromTs() {
    return new PrivateGetterClass();
}
export const privateGetterInstanceClass = new PrivateGetterClass();
export class UnionTypeClass {
    constructor(value) {
        this._value = value;
    }
    get value() {
        return this._value;
    }
}
export function createUnionTypeGetterClassFromTs(arg) {
    return new UnionTypeClass(arg);
}
export const unionTypeGetterInstanceClassInt = new UnionTypeClass(tsNumber);
export const unionTypeGetterInstanceClassString = new UnionTypeClass(tsString);
export class LiteralClass {
    constructor(value) {
        this._value = value;
    }
    get value() {
        return this._value;
    }
}
export function createLiteralTypeGetterClassFromTs(arg) {
    return new LiteralClass(arg);
}
export const literalTypeGetterInstanceClassInt = new LiteralClass(tsNumber);
export const literalTypeGetterInstanceClassString = new LiteralClass(tsString);
export class TupleTypeClass {
    constructor(value) {
        this._value = value;
    }
    get value() {
        return this._value;
    }
}
export function createTupleTypeGetterClassFromTs(arg) {
    return new TupleTypeClass(arg);
}
export const tupleTypeGetterInstanceClass = new TupleTypeClass([tsNumber, tsString]);
export class AnyTypeClass {
    get value() {
        return this._value;
    }
}
export function createAnyTypeGetterClassFromTs() {
    return new AnyTypeClass();
}
export const anyTypeGetterInstanceClass = new AnyTypeClass();
export const anyTypeExplicitGetterInstanceClass = new AnyTypeClass();
export class SubsetByRef {
    constructor() {
        this.RefClass = new PublicGetterClass();
    }
    get value() {
        return this.RefClass.value;
    }
}
export function createSubsetByRefGetterClassFromTs() {
    return new SubsetByRef();
}
export const subsetByRefInstanceClass = new SubsetByRef();
export class SubsetByValueClass {
    constructor(value) {
        this._value = value;
    }
    get value() {
        return this._value;
    }
}
const GClass = new PublicGetterClass();
export function createSubsetByValueGetterClassFromTs() {
    return new SubsetByValueClass(GClass.value);
}
export const subsetByValueInstanceClass = new SubsetByValueClass(GClass.value);
