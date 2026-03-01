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

let callbackEtsFunctionJs: Any = 0;

function testCallBackEtsFunctionLambda(): number {
    return ESValue.wrap(callbackEtsFunctionJs).invoke((x: number) => x + 1).toNumber();
}

function testCallBackEtsFunctionLambdaCapture(): number {
    let bias = 1;
    return ESValue.wrap(callbackEtsFunctionJs).invoke((x: number) => x + bias).toNumber();
}

function plusOne(x: number): number {
    return x + 1;
}

function testCallBackEtsFunctionOuter(): number {
    return ESValue.wrap(callbackEtsFunctionJs).invoke(plusOne).toNumber();
}

function sumFunc(...items: number[]) : number {
    let sum: number = 0;
    for (let i: int = 0; i < items.length; i++) {
        sum += items[i];
    }
    return sum;
}

class Sum implements Function {
    unsafeCall(...items: FixedArray<Any>): Any {
        let sum: number = 0;
        for (let i: int = 0; i < items.length; i++) {
            sum += (items[i] as number);
        }
        return sum;
    }
    get name(): string {
        return 'Sum';
    }
}

let sum: Sum = new Sum();

let sumFunction: Function = (...items: number[]) => {
    let sum: number = 0;
    for (let i: int = 0; i < items.length; i++) {
        sum += items[i];
    }
    return sum;
};
