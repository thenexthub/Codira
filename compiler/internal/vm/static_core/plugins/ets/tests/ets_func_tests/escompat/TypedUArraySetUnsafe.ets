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

function getTestName(arrayType: string): string {
    return arrayType + " meta $_set/$_get";
}

function testUint8ClampedArrayMetaSetGetClamp(): void {
    const arr = new Uint8ClampedArray(3);
    arr.$_set(0, -10);
    arr.$_set(1, 300);
    arr.$_set(2, 128);
    arktest.assertEQ(0, arr.$_get(0).toDouble());
    arktest.assertEQ(255, arr.$_get(1).toDouble());
    arktest.assertEQ(128, arr.$_get(2).toDouble());
}

function testUint8ArrayMetaSetGetWrap(): void {
    const arr = new Uint8Array(3);
    arr.$_set(0, 255);
    arr.$_set(1, -1);
    arr.$_set(2, 256);
    arktest.assertEQ(255, arr.$_get(0).toDouble());
    arktest.assertEQ(255, arr.$_get(1).toDouble());  // -1 mod 256
    arktest.assertEQ(0, arr.$_get(2).toDouble());    // 256 mod 256
}

function testUint16ArrayMetaSetGetWrap(): void {
    const arr = new Uint16Array(3);
    arr.$_set(0, 65535);
    arr.$_set(1, -1);
    arr.$_set(2, 65536);
    arktest.assertEQ(65535, arr.$_get(0).toDouble());
    arktest.assertEQ(65535, arr.$_get(1).toDouble()); // -1 mod 65536
    arktest.assertEQ(0, arr.$_get(2).toDouble());     // 65536 mod 65536
}

function testUint32ArrayMetaSetGetWrap(): void {
    const arr = new Uint32Array(3);
    arr.$_set(0, 4294967295);
    arr.$_set(1, -1);
    arr.$_set(2, 4294967296);
    arktest.assertEQ(4294967295, arr.$_get(0).toDouble());
    arktest.assertEQ(4294967295, arr.$_get(1).toDouble()); // -1 mod 2^32
    arktest.assertEQ(0, arr.$_get(2).toDouble());          // 2^32 mod 2^32
}

function testBigUint64ArrayMetaSetGetWrap(): void {
    const arr = new BigUint64Array(3);
    const max = 18446744073709551615n;
    const wrap = 18446744073709551616n;
    arr.$_set(0, max);
    arr.$_set(1, -1n);
    arr.$_set(2, wrap);
    arktest.assertEQ(max, arr.$_get(0));
    arktest.assertEQ(max, arr.$_get(1)); // -1 mod 2^64
    arktest.assertEQ(0n, arr.$_get(2));  // wrap mod 2^64
}

function main(): int {
    const suite = new arktest.ArkTestsuite("TypedUArrayMetaSetGet");
    suite.addTest(getTestName("Uint8ClampedArray"), testUint8ClampedArrayMetaSetGetClamp);
    suite.addTest(getTestName("Uint8Array"), testUint8ArrayMetaSetGetWrap);
    suite.addTest(getTestName("Uint16Array"), testUint16ArrayMetaSetGetWrap);
    suite.addTest(getTestName("Uint32Array"), testUint32ArrayMetaSetGetWrap);
    suite.addTest(getTestName("BigUint64Array"), testBigUint64ArrayMetaSetGetWrap);
    return suite.run();
}