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

function compareArraysAreEqual<T>(arr1: T[], arr2: T[]): boolean {
    if (arr1.length !== arr2.length) {
        return false;
    }
    for (let i = 0; i < arr1.length; i++) {
        if (Array.isArray(arr1[i]) && Array.isArray(arr2[i])) {
            if (!compareArraysAreEqual(arr1[i], arr2[i])) {
                return false;
            }
        } else {
            if (arr1[i] !== arr2[i]) {
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

function testRestOfNumber(): void {
    let tmpFun: (obj: number[], ...nums: number[]) => boolean =
        (obj: number[], ...nums: number[]): boolean => compareArraysAreEqual<number>(obj, nums);
    ASSERT_TRUE(checkRestOfNumber(tmpFun));
}

function testRestOfString(): void {
    let tmpFun: (obj: string[], ...strs: string[]) => boolean =
        (obj: string[], ...strs: string[]): boolean => compareArraysAreEqual<string>(obj, strs);
    ASSERT_TRUE(checkRestOfString(tmpFun));
}

function testRestOfObject(): void {
    let tmpFun: (obj: object[], ...objs: object[]) => boolean =
        (obj: object[], ...objs: object[]): boolean => compareArraysAreEqual<object>(obj, objs);
    ASSERT_TRUE(checkRestOfObject(tmpFun));
}

function testRestOfTuple(): void {
    let tmpFun: (obj: [number, string][], ...tupleArr: [number, string][]) => boolean =
        (obj: [number, string][], ...tupleArr: [number, string][]): boolean =>
            compareArraysAreEqual<[number, string]>(obj, tupleArr);
    ASSERT_TRUE(checkRestOfTuple(tmpFun));
}

function testRestofUnion(): void {
    let tmpFun: (obj: (number | string | boolean)[], ...unionArr: (number | string | boolean)[]) => boolean =
        (obj: (number | string | boolean)[], ...unionArr: (number | string | boolean)[]): boolean =>
            compareArraysAreEqual<number | string | boolean>(obj, unionArr);
    ASSERT_TRUE(checkRestofUnion(tmpFun));
}

function testRestOfJSValue(): void {
    let tmpFun: (obj: object[], ...jsvalArr: object[]) => boolean =
        (obj: object[], ...jsvalArr: object[]) => compareArraysAreEqual(obj, jsvalArr);
    ASSERT_TRUE(checkRestOfJSValue(tmpFun));
}

testRestOfNumber();
testRestOfString();
testRestOfObject();
// NOTE (#24570): correct interop tests with tuples
testRestofUnion();
testRestOfJSValue();
