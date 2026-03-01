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

function callbackEtsFunctionJs(callbackHandler: (x: number) => number): number {
    return callbackHandler(0x55aa);
}

function callbackEtsFunction(func: Function): number {
    return func(0x55aa);
}

function callbackEtsFunctionTwoParam(func: Function): number {
    return func(1, 2);
}

function callbackEtsFunctionSum(sumFunc: Function): boolean {
    ASSERT_TRUE(sumFunc() === 0);
    ASSERT_TRUE(sumFunc(1) === 1);
    ASSERT_TRUE(sumFunc(1, 2) === 3);
    ASSERT_TRUE(sumFunc(1, 2, 3) === 6);
    return true;
}

function testErrorCall(sumFunc: ()=> void, erroString: string): void {
    try {
        sumFunc();
    } catch (e) {
        ASSERT_TRUE(e.toString() === erroString);
    }
}

function main(): void {
    let etsVm = globalThis.gtest.etsVm;

    // emulate import
    let ets_global = etsVm.getClass('Lets_functions/ETSGLOBAL;');
    ets_global.callbackEtsFunctionJs = callbackEtsFunctionJs;

    let testCallBackEtsFunctionLambda = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'testCallBackEtsFunctionLambda');
    let callBackResult = testCallBackEtsFunctionLambda();
    ASSERT_TRUE(callBackResult === 0x55ab);

    let testCallBackEtsFunctionLambdaCapture = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'testCallBackEtsFunctionLambdaCapture');
    callBackResult = testCallBackEtsFunctionLambdaCapture();
    ASSERT_TRUE(callBackResult === 0x55ab);

    let testCallBackEtsFunctionOuter = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'testCallBackEtsFunctionOuter');
    callBackResult = testCallBackEtsFunctionOuter();
    ASSERT_TRUE(callBackResult === 0x55ab);

    let plusOne = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'plusOne');
    callBackResult = callbackEtsFunction(plusOne);
    ASSERT_TRUE(callBackResult === 0x55ab);

    let sumFunc = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'sumFunc');
    ASSERT_TRUE(callbackEtsFunctionSum(sumFunc));

    let sumFunction = etsVm.getClass('Lets_functions/ETSGLOBAL;').sumFunction;
    ASSERT_TRUE(callbackEtsFunctionSum(sumFunction));

    testErrorCall(()=> {
        callbackEtsFunctionTwoParam(plusOne);
    }, 'TypeError: no suitable method found for this number of arguments');

    testErrorCall(()=> {
        let sumClass = etsVm.getClass('Lets_functions/Sum;');
        callbackEtsFunctionSum(new sumClass());
    }, 'TypeError: Object is not callable');

    testErrorCall(()=> {
        callbackEtsFunctionSum(ets_global.sum);
    }, 'TypeError: Object is not callable');
}

main();
