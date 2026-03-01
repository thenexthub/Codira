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

let etsVm = globalThis.gtest.etsVm;
let testFunction = etsVm.getFunction('Ltest_sts_return_array/ETSGLOBAL;', 'testFunction');

export function functionReturnArray1(arg1?: Array<number>): Array<number> {
    if (arg1 === undefined) {
        return [1, 2];
    } else {
        return arg1;
    }
}

export function functionReturnArray2(arg1: Array<number>): Array<number> {
    return arg1;
}

export function functionReturnArray3(arg1: Array<number> = [5, 6]): Array<number> {
    return arg1;
}

export function functionReturnString1(arg1?: Array<string>): Array<string> {
    if (arg1 === undefined) {
        return ['a', 'b'];
    } else {
        return arg1;
    }
}

export function functionReturnString2(arg1: Array<string>): Array<string> {
    return arg1;
}

export function functionReturnString3(arg1: Array<string> = ['a', 'b']): Array<string> {
    return arg1;
}

ASSERT_TRUE(testFunction());
