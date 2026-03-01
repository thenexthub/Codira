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

function plusOne(x: number): number {
    return x + 1;
}

function sum(...items: number[]): number {
    let sum = 0;
    for (let i = 0; i < items.length; i++) {
        sum += items[i];
    }
    return sum;
}

let sumFunc = (...items: number[]): number => {
    let sum = 0;
    for (let i = 0; i < items.length; i++) {
        sum += items[i];
    }
    return sum;
};

function main(): void {
    let etsVm = globalThis.gtest.etsVm;

    let callbackJsFunctionEts = etsVm.getFunction('Lets_function/ETSGLOBAL;', 'callbackJsFunctionEts');

    // test lambda function
    let callBackResult = callbackJsFunctionEts((val: number) => val + 1);
    ASSERT_TRUE(callBackResult === 0x55ab);

    // test lambda function with capture
    let bias = 1;
    callBackResult = callbackJsFunctionEts((val: number) => val + bias);
    ASSERT_TRUE(callBackResult === 0x55ab);

    // test normal function
    callBackResult = callbackJsFunctionEts(plusOne);
    ASSERT_TRUE(callBackResult === 0x55ab);

    let callbackFunctionEts = etsVm.getFunction('Lets_function/ETSGLOBAL;', 'callbackFunctionEts');
    callBackResult = callbackFunctionEts(plusOne);
    ASSERT_TRUE(callBackResult === 0x55ab);

    let callbackFunctionEtsTwoParam = etsVm.getFunction('Lets_function/ETSGLOBAL;', 'callbackFunctionEtsTwoParam');
    callBackResult = callbackFunctionEtsTwoParam(plusOne);
    ASSERT_TRUE(callBackResult === 0x55ab);

    let callbackFunctionEtsTestSum = etsVm.getFunction('Lets_function/ETSGLOBAL;', 'callbackFunctionEtsTestSum');
    ASSERT_TRUE(callbackFunctionEtsTestSum(sum));
    ASSERT_TRUE(callbackFunctionEtsTestSum(sumFunc));
}

main();
