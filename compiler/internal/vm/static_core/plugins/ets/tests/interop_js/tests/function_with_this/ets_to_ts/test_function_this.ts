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

let baseFunc = globalThis.gtest.etsVm.getClass('Lfunction_this/ETSGLOBAL;').baseFunc;
let childFunc = globalThis.gtest.etsVm.getClass('Lfunction_this/ETSGLOBAL;').childFunc;
let fooBaseObj = globalThis.gtest.etsVm.getClass('Lfunction_this/ETSGLOBAL;').fooBaseObj;
let fooChildObj = globalThis.gtest.etsVm.getClass('Lfunction_this/ETSGLOBAL;').fooChildObj;
let etsDoCallbackFunction = globalThis.gtest.etsVm.getClass('Lfunction_this/ETSGLOBAL;').etsDoCallbackFunction;
let etsDoCallbackLambda = globalThis.gtest.etsVm.getClass('Lfunction_this/ETSGLOBAL;').etsDoCallbackLambda;


function doCallbackFunction(callback: Function) {
    return callback();
}

function doCallbackLambda(callback: ()=>object) {
    return callback();
}

function testBaseClassFunc() {
    let obj1 = doCallbackFunction(baseFunc);
    ASSERT_TRUE(typeof obj1 === 'object');
    ASSERT_TRUE(obj1.baseVal === 1);

    let obj2 = doCallbackLambda(baseFunc);
    ASSERT_TRUE(typeof obj2 === 'object');
    ASSERT_TRUE(obj2.baseVal === 1);
}

function testBaseClassFuncEts() {
    let obj1 = etsDoCallbackFunction(baseFunc);
    ASSERT_TRUE(typeof obj1 === 'object');
    ASSERT_TRUE(obj1.baseVal === 1);

    let obj2 = etsDoCallbackLambda(baseFunc)
    ASSERT_TRUE(typeof obj2 === 'object');
    ASSERT_TRUE(obj2.baseVal === 1);
}

function testChildClassFunc() {
    let obj1 = doCallbackFunction(childFunc);
    ASSERT_TRUE(typeof obj1 === 'object');
    ASSERT_TRUE(obj1.baseVal === 1);
    ASSERT_TRUE(obj1.childVal === 2);

    let obj2 = doCallbackLambda(childFunc)
    ASSERT_TRUE(typeof obj2 === 'object');
    ASSERT_TRUE(obj2.baseVal === 1);
    ASSERT_TRUE(obj2.childVal === 2);
}

function testChildClassFuncEts() {
    let obj1 = etsDoCallbackFunction(childFunc);
    ASSERT_TRUE(typeof obj1 === 'object');
    ASSERT_TRUE(obj1.baseVal === 1);
    ASSERT_TRUE(obj1.childVal === 2);

    let obj2 = etsDoCallbackLambda(childFunc)
    ASSERT_TRUE(typeof obj2 === 'object');
    ASSERT_TRUE(obj2.baseVal === 1);
    ASSERT_TRUE(obj2.childVal === 2);
}

function testFooBaseObj() {
    let obj1 = doCallbackFunction(fooBaseObj.foo)
    ASSERT_TRUE(typeof obj1 === 'object');
    ASSERT_TRUE(obj1.baseVal === 1);

    let obj2 = doCallbackLambda(fooBaseObj.foo)
    ASSERT_TRUE(typeof obj2 === 'object');
    ASSERT_TRUE(obj2.baseVal === 1);
}

function testFooBaseObjEts() {
    let obj1 = etsDoCallbackFunction(fooBaseObj.foo)
    ASSERT_TRUE(typeof obj1 === 'object');
    ASSERT_TRUE(obj1.baseVal === 1);

    let obj2 = etsDoCallbackLambda(fooBaseObj.foo)
    ASSERT_TRUE(typeof obj2 === 'object');
    ASSERT_TRUE(obj2.baseVal === 1);
}

function testFooChildObj() {
    let obj1 = doCallbackFunction(fooChildObj.foo)
    ASSERT_TRUE(typeof obj1 === 'object');
    ASSERT_TRUE(obj1.baseVal === 1);
    ASSERT_TRUE(obj1.childVal === 2);

    let obj2 = doCallbackLambda(fooChildObj.foo)
    ASSERT_TRUE(typeof obj2 === 'object');
    ASSERT_TRUE(obj2.baseVal === 1);
    ASSERT_TRUE(obj2.childVal === 2);
}

function testFooChildObjEts() {
    let obj1 = etsDoCallbackFunction(fooChildObj.foo)
    ASSERT_TRUE(typeof obj1 === 'object');
    ASSERT_TRUE(obj1.baseVal === 1);
    ASSERT_TRUE(obj1.childVal === 2);

    let obj2 = etsDoCallbackLambda(fooChildObj.foo)
    ASSERT_TRUE(typeof obj2 === 'object');
    ASSERT_TRUE(obj2.baseVal === 1);
    ASSERT_TRUE(obj2.childVal === 2);
}

function testFooInterfaceCall() {
    let obj1 = fooBaseObj.foo()
    ASSERT_TRUE(typeof obj1 === 'object');
    ASSERT_TRUE(obj1.baseVal === 1);

    let obj2 = fooChildObj.foo()
    ASSERT_TRUE(typeof obj2 === 'object');
    ASSERT_TRUE(obj2.baseVal === 1);
    ASSERT_TRUE(obj2.childVal === 2);
}

testBaseClassFunc();
testBaseClassFuncEts();
testChildClassFunc();
testChildClassFuncEts();
testFooBaseObj();
testFooBaseObjEts();
testFooChildObj();
testFooChildObjEts();
testFooInterfaceCall();