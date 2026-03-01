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

const ErrMsg = 'This is a Error';
const ErrorMessage = 'Panda: throw new error in ets!';
const CustomErrorMessage = 'Panda: Something went wrong!';
const CustomErrorCode = 1001;

const ErrorFunc = etsVm.getFunction('Lerror/test/ETSGLOBAL;', 'ErrorFunc');
const CustomErrorFunc = etsVm.getFunction('Lerror/test/ETSGLOBAL;', 'CustomErrorFunc');
const err = etsVm.getClass('Lerror/test/ETSGLOBAL;').err;

function testError(): boolean {
    let res: boolean;
    res = (err instanceof Error) && (err.message === ErrMsg);
    return res;
}

function testThrowError(): boolean {
    let res: boolean;
    try {
        ErrorFunc(ErrorMessage);
        res = false;
    } catch (error) {
        let e: Error = error as Error;
        res = (error instanceof Error) && (e.message === ErrorMessage);
    }
    return res;
}

function testCustomThrowError(): boolean {
    let res: boolean;
    try {
        CustomErrorFunc(CustomErrorMessage, CustomErrorCode);
        res = false;
    } catch (error) {
        let e: Error = error as Error;
        res = (error instanceof Error) && (e.message === CustomErrorMessage) &&
              (e.code === CustomErrorCode);
    }
    return res;
}


ASSERT_TRUE(testError());
ASSERT_TRUE(testThrowError());
ASSERT_TRUE(testCustomThrowError());