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
export const tsNumber = 1;
export const tsString = 'string';
export class AnyTypeMethodClass {
    get(a) {
        return a;
    }
}
export function createInterfaceClassAnyTypeMethod() {
    return new AnyTypeMethodClass();
}
export class UnionTypeMethodClass {
    get(a) {
        return a;
    }
}
export function createInterfaceClassUnionTypeMethod() {
    return new AnyTypeMethodClass();
}
export function subsetByRefInterface(obj) {
    return obj.get();
}
class UserClass {
    constructor() {
        this.value = 1;
    }
}
export class SubsetByValueClass {
    get() {
        return new UserClass();
    }
}
export function createSubsetByValueClassFromTs() {
    return new SubsetByValueClass();
}
export class WithOptionalMethodClass {
    getNum() {
        return tsNumber;
    }
    getStr() {
        return tsString;
    }
}
export class WithoutOptionalMethodClass {
    getStr() {
        return tsString;
    }
}
export function createClassWithOptionalMethod() {
    return new WithOptionalMethodClass();
}
export function createClassWithoutOptionalMethod() {
    return new WithoutOptionalMethodClass();
}
export function optionalArg(arg, optional) {
    if (optional) {
        return { with: arg, without: optional };
    }
    return { with: arg };
}
export function optionalArgArray(...arg) {
    const withOptional = arg[0];
    const withoutOptional = arg[1];
    if (withoutOptional) {
        return { with: withOptional, without: withoutOptional };
    }
    return { with: withOptional };
}
export class TupleTypeMethodClass {
    get(arg) {
        return arg;
    }
}
export function createInterfaceClassTupleTypeMethodFromTs() {
    return new TupleTypeMethodClass();
}
export const unionTypeMethodInstanceClass = new UnionTypeMethodClass();
export const subsetByValueInstanceClass = new SubsetByValueClass();
export const tupleInstanceClass = new TupleTypeMethodClass();
export const withoutOptionalMethodInstanceClass = new WithoutOptionalMethodClass();
export const withOptionalMethodInstanceClass = new WithOptionalMethodClass();
