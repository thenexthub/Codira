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

function compareArraysAreEqual<T>(arr1: FixedArray<T>, arr2: FixedArray<T>) {
    if(arr1.length !== arr2.length) {
        return false;
    }
    for(let i = 0; i < arr1.length; i++) {
        if (Array.isArray(arr1[i]) && Array.isArray(arr2[i])) {
            if (!compareArraysAreEqual(arr1[i], arr2[i])) {
                return false;
            }
        } else {
            if(arr1[i] !== arr2[i]) {
                return false;
            }
        }
    }
    return true;
}

let checkRestOfNumber = etsVm.getFunction('Lrest_spread/test/ETSGLOBAL;', 'checkRestOfNumber');
let checkRestOfString = etsVm.getFunction('Lrest_spread/test/ETSGLOBAL;', 'checkRestOfString');
let checkRestOfObject = etsVm.getFunction('Lrest_spread/test/ETSGLOBAL;', 'checkRestOfObject');
let checkRestOfTuple = etsVm.getFunction('Lrest_spread/test/ETSGLOBAL;', 'checkRestOfTuple');
let checkRestofUnion = etsVm.getFunction('Lrest_spread/test/ETSGLOBAL;', 'checkRestofUnion');
let checkRestOfJSValue = etsVm.getFunction('Lrest_spread/test/ETSGLOBAL;', 'checkRestOfJSValue');

function testRestOfNumber() {
    let tmpFun = (obj: FixedArray<number>, ...nums: FixedArray<number>) => compareArraysAreEqual<number>(obj, nums);
   ASSERT_TRUE(checkRestOfNumber(tmpFun));
}

function testRestOfString() {
    let tmpFun = (obj: FixedArray<string>, ...strs: FixedArray<string>) => compareArraysAreEqual(obj, strs);
    ASSERT_TRUE(checkRestOfString(tmpFun));
}

function testRestOfObject() {
    let tmpFun = (obj: FixedArray<object>, ...objs: FixedArray<object>) => compareArraysAreEqual(obj, objs);
    ASSERT_TRUE(checkRestOfObject(tmpFun));
}

function testRestOfTuple() {
    let tmpFun = (obj: [number, string][], ...tupleArr: [number, string][]) => compareArraysAreEqual(obj, tupleArr);
    ASSERT_TRUE(checkRestOfTuple(tmpFun));
}

function testRestofUnion() {
    let tmpFun = (obj: FixedArray<number | string | boolean>, ...unionArr: FixedArray<number | string | boolean>) => compareArraysAreEqual(obj, unionArr);
    ASSERT_TRUE(checkRestofUnion(tmpFun));
}

function testRestOfJSValue() {
    let tmpFun = (obj: FixedArray<object>, ...jsvalArr: FixedArray<object>) => compareArraysAreEqual(obj, jsvalArr);
    ASSERT_TRUE(checkRestOfJSValue(tmpFun));
}

testRestOfNumber();
testRestOfString();
testRestOfObject();
// NOTE (#24570): correct interop tests with tuples
// testRestOfTuple();
testRestofUnion();
testRestOfJSValue();
