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

function testCallRange(a, b) {
    return a + b;
}

function testCall0() {
    return 1;
}

function testCallShort(a) {
    return a + 1;
}

class C {
    f = 1;
}

class TestNew0 {
    field1 = 0;
    constructor() {
    }
}

class TestNewRange {
    field1;
    field2;

    constructor(field1, field2) {
        this.field1 = field1;
        this.field2 = field2;
    }

    baz() {
        return 1;
    }

    foo(a, b) {
        return a + b;
    }

    bar(a) {
        return a + 1;
    }
}

class TestNewShort {
    field1;

    constructor(field1) {
        this.field1 = field1;
    }
}

let testInstanceOf = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'testIsInstance');

let doTestCallRange = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestCallRange');
let doTestCall0 = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestCall0');
let doTestNew0 = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestNew0');
let doTestNewRange = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestNewRange');
let doTestCallThis0 = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestCallThis0');
let doTestCallThisRange = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestCallThisRange');
let doTestCallShort = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestCallShort');
let doTestCallThisShort = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestCallThisShort');
let doTestNewShort = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestNewShort');

let doTestVal = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestVal');
let doTestValName = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestValName');
let doTestValNameV = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestValNameV');
let doTestValIdx = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestValIdx');
let doTestStVal = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestStVal');
let doTestStValName = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestStValName');
let doTestStValNameV = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestStValNameV');
let doTestStValIdx = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestStValIdx');

let doTestLdbyvalFooStatic = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestLdbyvalFooStatic');
let doTestLdbyvalArrayStatic = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestLdbyvalArrayStatic');
let doTestLdByIdxArrayStatic = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestLdByIdxArrayStatic');
let doTestStByValArrayStatic = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestStByValArrayStatic');
let doTestStByValFooStatic = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestStByValFooStatic');
let doTestStByIdxArrayStatic = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestStByIdxArrayStatic');

let doTestIsTrue = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestIsTrue');
let doTestTypeOf = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestTypeOf');
let doTestEquals = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestEquals');
let doTestStrictEquals = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestStrictEquals');

let doTestCallStatic = etsVm.getFunction('Ltest-static/ETSGLOBAL;', 'doTestCallStatic');

function anyInstanceOfTest() {
    ASSERT_TRUE(testInstanceOf(new C(), C));
    ASSERT_TRUE(!testInstanceOf({f: 1}, C));
}

function anyCallRangeTest() {
    ASSERT_EQ(doTestCallRange(testCallRange, 0, 1), 1);
}

function anyCall0Test() {
    ASSERT_EQ(doTestCall0(testCall0), 1);
}

function anyCallShortTest() {
    ASSERT_EQ(doTestCallShort(testCallShort, 0), 1);
}

function anyCallThisRangeTest() {
    let obj = new TestNewRange(0, 1);
    ASSERT_EQ(doTestCallThisRange(obj, 0, 1), 1);
}

function anyCallThis0Test() {
    let obj = new TestNewRange(0, 1);
    ASSERT_EQ(doTestCallThis0(obj), 1);
}

function anyCallThisShortTest() {
    let obj = new TestNewRange(0, 1);
    ASSERT_EQ(doTestCallThisShort(obj, 0), 1);
}

function anyCallNewRangeTest() {
    let obj = doTestNewRange(TestNewRange, 0, 1);
    ASSERT_EQ(obj.field1, 0);
    ASSERT_EQ(obj.field2, 1);
}

function anyCallNew0Test() {
    let obj = doTestNew0(TestNew0);
    ASSERT_EQ(obj.field1, 0);
}

function anyCallNewShortTest() {
    let obj = doTestNewShort(TestNewShort, 1);
    ASSERT_EQ(obj.field1, 1);
}

function anyLdByValTest() {
    ASSERT_EQ(doTestVal({a: 1}, 'a'), 1);
}

function anyLdByNameTest() {
    ASSERT_EQ(doTestValName({a: 1}), 1);
}

function anyLdByNameVTest() {
    ASSERT_EQ(doTestValNameV({a: 1}), 1);
}

function anyLdByIdxTest() {
    ASSERT_EQ(doTestValIdx({1: 1}), 1);
}

function anyStByValTest() {
    let o = {a: 0};
    doTestStVal(o, 'a', 1);
    ASSERT_EQ(o.a, 1);
}

function anyStByNameTest() {
    let o = {a: 0};
    doTestStValName(o, 1);
    ASSERT_EQ(o.a, 1);
}

function anyStByNameVTest() {
    let o = {a: 0};
    doTestStValNameV(o, 1);
    ASSERT_EQ(o.a, 1);
}

function anyStByIdxTest() {
    let o = {1:0};
    doTestStValIdx(o, 1);
    ASSERT_EQ(o[1], 1);
}

// Enable once FE create any bytecode for pure static code
function doTestLdbyvalFooStaticTest() {
    let o = doTestLdbyvalFooStatic();
    ASSERT_EQ(o, 0x55aa);
}

// Enable once FE create any bytecode for pure static code
function doTestLdbyvalArrayStaticTest() {
    let o = doTestLdbyvalArrayStatic();
    ASSERT_EQ(o, 0x7c00);
}

// Enable once FE create any bytecode for pure static code
function doTestLdByIdxArrayStaticTest() {
    let o = doTestLdByIdxArrayStatic();
    ASSERT_EQ(o, 0x7c00);
}

// Enable once FE create any bytecode for pure static code
function doTestStByValArrayStaticTest() {
    let o = doTestStByValArrayStatic();
    ASSERT_EQ(o, 0xcafe);
}

// Enable once FE create any bytecode for pure static code
function doTestStByValFooStaticTest() {
    let o = doTestStByValFooStatic();
    ASSERT_EQ(o, 0xcafe);
}

// Enable once FE create any bytecode for pure static code
function doTestStByIdxArrayStaticTest() {
    let o = doTestStByIdxArrayStatic();
    ASSERT_EQ(o, 0xcafe);
}

function doTestAnyCallStaticTest() {
    ASSERT_EQ(doTestCallStatic(), 1);
}

function etsIsTrueTest() {
    let o = new C();
    ASSERT_TRUE(doTestIsTrue(o));
    ASSERT_TRUE(!doTestIsTrue(''));
    ASSERT_TRUE(!doTestIsTrue(0));
}

function etsTypeOfTest() {
    let o = new C();
    ASSERT_EQ(doTestTypeOf(o), 'object');
}

function etsEqualsTest() {
    let o1 = new C();
    let o2 = new C();
    ASSERT_TRUE(!doTestEquals(o1, o2));
    ASSERT_TRUE(!doTestEquals(o1, 'aaa'));
    ASSERT_TRUE(doTestEquals(o1, o1));
}

function etsStrictEqualsTest() {
    let o1 = new C();
    let o2 = new C();
    ASSERT_TRUE(!doTestStrictEquals(o1, o2));
    ASSERT_TRUE(!doTestEquals(o1, 'aaa'));
    ASSERT_TRUE(doTestStrictEquals(o1, o1));
}

anyInstanceOfTest();

anyCallRangeTest();
anyCall0Test();
anyCallShortTest();
anyCallThisRangeTest();
anyCallThis0Test();
anyCallThisShortTest();
anyCallNewRangeTest();
anyCallNew0Test();
anyCallNewShortTest();

anyLdByValTest();
anyLdByNameTest();
anyLdByNameVTest();
anyLdByIdxTest();
anyStByValTest();
anyStByNameTest();
anyStByNameVTest();
anyStByIdxTest();

etsIsTrueTest();
etsTypeOfTest();
etsEqualsTest();
etsStrictEqualsTest();

doTestAnyCallStaticTest();
doTestLdbyvalFooStaticTest();
doTestLdbyvalArrayStaticTest();
doTestLdByIdxArrayStaticTest();
doTestStByValArrayStaticTest();
doTestStByValFooStaticTest();
doTestStByIdxArrayStaticTest();