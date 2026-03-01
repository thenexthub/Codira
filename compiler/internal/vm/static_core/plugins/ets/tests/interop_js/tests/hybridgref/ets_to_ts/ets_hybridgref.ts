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

let plusOne = (x: number) => {
    return x + 1;
};

function testPrimitiveTypes(etsVm: any): void {
    etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefChar')();
    ASSERT_EQ(typeof nativeGetRef(), "string");
    ASSERT_EQ(nativeGetRef(), "A");

    etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefByte')();
    ASSERT_EQ(nativeGetRef(), 0x12);

    etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefShort')();
    ASSERT_EQ(nativeGetRef(), 300);

    etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefInt')();
    ASSERT_EQ(nativeGetRef(), 123456);

    etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefDouble')();
    ASSERT_TRUE(Math.abs(nativeGetRef() - 100.111) < 1e-10);

    etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefFloat')();
    ASSERT_TRUE(Math.abs(nativeGetRef() - 1.5) < 1e-6);

    etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'etsSaveNativeGrefString')();
    ASSERT_EQ(nativeGetRef(), "hello world");
}

function testArrayAndObjectTypes(etsVm: any): void {
    etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefNumberArray")();
    const arr = nativeGetRef();
    ASSERT_TRUE(arr instanceof Array);
    ASSERT_EQ(arr.length, 5);
    ASSERT_EQ(arr[2], 3);

    etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefObject")();
    const obj = nativeGetRef();
    ASSERT_EQ(obj.name, "ArkTS");
    ASSERT_EQ(obj.version, 1);
}

function testMapAndSetTypes(etsVm: any): void {
    etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefMap")();
    const map = nativeGetRef();
    ASSERT_TRUE(map instanceof Map);
    ASSERT_EQ(map.get("a"), 10);
    ASSERT_EQ(map.get("b"), 20);

    etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefSet")();
    const set = nativeGetRef();
    ASSERT_TRUE(set instanceof Set);
    ASSERT_TRUE(set.has("x"));
    ASSERT_TRUE(set.has("y"));
}

function testFunctionCallback(etsVm: any): void {
    etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefObjectFromTS")(plusOne);
    const fn = nativeGetRef();
    ASSERT_TRUE(fn === plusOne);
}

function testBufferAndViewTypes(etsVm: any): void {
    etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefArrayBuffer")();
    const arrbuf = nativeGetRef();
    ASSERT_TRUE(arrbuf instanceof ArrayBuffer);
    ASSERT_EQ(arrbuf.byteLength, 8);

    etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefTypedArray")();
    const typed = nativeGetRef();
    ASSERT_TRUE(typed instanceof Int8Array);
    ASSERT_EQ(typed.length, 2);
    ASSERT_EQ(typed[1], 2);

    etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefDataView")();
    const view = nativeGetRef();
    ASSERT_TRUE(view instanceof DataView);
    ASSERT_TRUE(view.byteLength === 8);
    ASSERT_TRUE(view.getUint32(0, true) === 0xbabe);
    ASSERT_TRUE(view.getUint32(4, true) === 0xcafe);
    view.setUint32(0, 0xbeef, true);
    view.setUint32(4, 0xf00d, true);
    ASSERT_TRUE(view.getUint32(0, true) === 0xbeef);
    ASSERT_TRUE(view.getUint32(4, true) === 0xf00d);
}

function testPromiseAndErrorTypes(etsVm: any): void {
    etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefPromise")();
    const promise = nativeGetRef();
    ASSERT_TRUE(promise instanceof Promise);

    let resolved = false;
    promise.then((val: number) => {
        ASSERT_EQ(val, 42);
        resolved = true;
    });

    etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefError")();
    const err = nativeGetRef();
    ASSERT_TRUE(err instanceof Error);
    ASSERT_EQ(err.message, "Test error");
}

function testDateType(etsVm: any): void {
    etsVm.getFunction("Lets_functions/ETSGLOBAL;", "etsSaveNativeGrefDate")();
    const date = nativeGetRef();
    ASSERT_TRUE(date instanceof Date);
    ASSERT_EQ(date.toISOString(), "2025-05-27T12:34:56.789Z");
}

function main(): void {
    const etsVm = globalThis.gtest.etsVm;

    testPrimitiveTypes(etsVm);
    testArrayAndObjectTypes(etsVm);
    testMapAndSetTypes(etsVm);
    testFunctionCallback(etsVm);
    testBufferAndViewTypes(etsVm);
    testPromiseAndErrorTypes(etsVm);
    testDateType(etsVm);
}

(globalThis as any).nativeGetRef();
main();
