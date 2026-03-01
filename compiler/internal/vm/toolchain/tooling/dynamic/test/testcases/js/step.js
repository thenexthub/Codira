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

print("testStart");

async function testAwait() {
    return new Promise(resolve => {
        resolve("test");
    });
}
function testFunction() {
    var a = 1;
    a = 2;
    a = 3;
    a = 4;
    nextFunction();
    a = 5;
    a = 6;
    a = 7;
    a = 8;
    a = 9;
    a = 10;
    a = 12;
    async function asyncTest() {
        var c = 1;
        c = 2;
        return 'test';
    }
    a = 13;
    a = 14;
    asyncTest();
    a = 15;
    a = 16;
    a = 17;
    a = 18;
    a = 19;
    a = 20;
    async function asyncTest2() {
        var d = 1;
        d = 2;
        var result = await testAwait();
        d = 3;
        return result;
    }
    asyncTest2();
    a = 21;
    a = 22;
    a = 23;
    if (a == 23) {
        a = 100;
    }
    a += 1;
    a += 1;
    a += 1;
    a += 1;
    a += 1;
    a += 1;
    a += 1;
    print("->testFunction end, a:" + a);
}
function nextFunction() {
    print("->nextFunction()");
    var c = 1;
    c = 2;
    c = 3;
    c = 4;
    c = 5;
    c = 6;
    c = 7;
}
print("->testFunction()");
var b = 1;
b = 2;
b = 3;
b = 4;
testFunction();
b = 5;
b = 6;
b = 7;
b = 8;
b = 9;
b = 10;
b = 11;
b = 12;
b = 13;
print("->testEnd");
