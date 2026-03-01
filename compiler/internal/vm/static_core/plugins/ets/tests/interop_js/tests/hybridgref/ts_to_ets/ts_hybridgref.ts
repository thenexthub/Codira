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

function multiply(a: number, b: number): number {
    return a * b;
}

class MyObject {
    value: number;

    constructor(value: number) {
        this.value = value;
    }

    getDouble(): number {
        return this.value * 2;
    }
}

function testPrimitiveValues(etsVm: any): void {
    nativeSaveRef('NativeGref String');
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckNativeGrefString')());

    nativeSaveRef(123.456);
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckNativeGrefNumber')());

    nativeSaveRef(null);
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckNativeGrefNullSafety')());
}

function testObjectValues(etsVm: any): void {
    nativeSaveRef({ name: 'ArkTS', version: 1 });
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckNativeGrefObjectField')());

    nativeSaveRef({
        meta: {
            id: 123,
            tag: 'nested',
            inner: { active: true }
        }
    });
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckNestedObject')());

    const sym = Symbol('secret');
    nativeSaveRef({
        sym: 'hidden',
        visible: 'shown'
    });
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckSymbolFieldIgnored')());

    nativeSaveRef({ a: 1, b: undefined });
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckUndefinedField')());
}

function testArrayValues(etsVm: any): void {
    nativeSaveRef([10, 20, 30]);
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckNativeGrefArray')());

    nativeSaveRef([
        [1, 2],
        [3, 4],
        [5, 6]
    ]);
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckNestedArray')());

    nativeSaveRef([]);
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckEmptyArray')());
}

function testFunctionValues(etsVm: any): void {
    nativeSaveRef(multiply);
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckNativeGrefCallback')());

    const obj = new MyObject(42);
    nativeSaveRef(obj);
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckObjectWithMethod')());
}

function testBuiltInObjects(etsVm: any): void {
    nativeSaveRef(new Date('2025-01-01T12:00:00Z'));
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckDateObject')());

    nativeSaveRef(new ArrayBuffer(8));
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckArrayBuffer')());

    nativeSaveRef(new Uint8Array([1, 2, 3]));
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckTypedArray')());

    nativeSaveRef(Promise.resolve(42));
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckPromiseResolved')());

    let mySet: Set<number> = new Set<number>([1, 2, 3]);
    nativeSaveRef(mySet);
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckSetObject')());

    nativeSaveRef(new Error('hybridgref error test'));
    ASSERT_TRUE(etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsCheckErrorObject')());
}

function main(): void {
    let etsVm = globalThis.gtest.etsVm;

    testPrimitiveValues(etsVm);
    testObjectValues(etsVm);
    testArrayValues(etsVm);
    testFunctionValues(etsVm);
    // NOTE(www): #ICMNFA, Currently unable to get method object in 1.2.
    // should add testBuiltInObjects(etsVm);
}

(globalThis as any).nativeSaveRef('temp');

main();
