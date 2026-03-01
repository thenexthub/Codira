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

const etsVm = globalThis.gtest.etsVm;

let getProxyA = etsVm.getFunction('Lreflect_proxy/test/ETSGLOBAL;', 'getProxyA');
let getProxyAB = etsVm.getFunction('Lreflect_proxy/test/ETSGLOBAL;', 'getProxyAB');
let testProxyA = etsVm.getFunction('Lreflect_proxy/test/ETSGLOBAL;', 'testProxyA');
let testProxyAB = etsVm.getFunction('Lreflect_proxy/test/ETSGLOBAL;', 'testProxyAB');
let SomeObject = etsVm.getClass('Lreflect_proxy/test/SomeObject;');

function testReflectProxyA(): void {
    let val = getProxyA();
    let val1 = getProxyA();

    ASSERT_TRUE(val !== val1);

    ASSERT_TRUE(val.messageA === 'some default string A');
    ASSERT_TRUE(val.foo() === 111);

    let testStr = 'some default string from ts';
    val.messageA = testStr;
    ASSERT_TRUE(val.messageA === testStr);

    let sObjec = new SomeObject(1);
    let rObject = val.boo(111, sObjec);
    ASSERT_TRUE(rObject.point === 112);

    testProxyA(val, testStr);
}

function testReflectProxyAB(): void {
    let val = getProxyAB();
    let val1 = getProxyAB();

    ASSERT_TRUE(val !== val1);

    ASSERT_TRUE(val.messageA === 'some default string A');
    ASSERT_TRUE(val.foo() === 111);
    ASSERT_TRUE(val.messageB === 'some default string B');
    ASSERT_TRUE(val.goo() === 123);

    let testStrA = 'some default string from ts A';
    val.messageA = testStrA;
    ASSERT_TRUE(val.messageA === testStrA);

    let sObjec = new SomeObject(1);
    let rObject = val.boo(111, sObjec);
    ASSERT_TRUE(rObject.point === 112);

    let testStrB = 'some default string from ts B';
    val.messageB = testStrB;
    ASSERT_TRUE(val.messageB === testStrB);

    testProxyAB(val, testStrA, testStrB);
}

testReflectProxyA();
testReflectProxyAB();