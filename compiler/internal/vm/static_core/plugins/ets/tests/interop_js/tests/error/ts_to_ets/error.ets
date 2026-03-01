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

let module = ESObject.load('../../plugins/ets/tests/interop_js/tests/error/ts_to_ets/test_error');
let err = module.getProperty('err');
let etsErrorFunc = module.getProperty('ErrorFunc');
let etsCustomErrorFunc = module.getProperty('CustomErrorFunc');
let errRangeErrorModule = module.getProperty('errRangeError');

const ErrMsg: string = 'This is a Error';
const ErrorMessage: string = 'Panda: throw new error in ets!';
const CustomErrorMessage: string = 'Panda: Something went wrong!';
const CustomErrorCode: number = 1001;

function testError(): boolean {
    let res: boolean;
    res = (err.getProperty('message').toString() === ErrMsg);
    return res;
}

function testThrowError(): boolean {
    let res: boolean;
    try {
        etsErrorFunc.invoke(ErrorMessage);
        res = false;
    } catch (error) {
        res = error.message === ErrorMessage;
    }
    return res;
}

function testCustomThrowError(): boolean {
    let error_catched = false;
    try {
        etsCustomErrorFunc.invoke(CustomErrorMessage, CustomErrorCode);
    } catch (e) {
        error_catched = true;

        arktest.assertEQ(e instanceof Error, true);
        arktest.assertEQ(e.message, CustomErrorMessage);
        arktest.assertEQ(e.code, CustomErrorCode);
    }
    return error_catched;
}

function testRangeError(): boolean {
    let res: boolean;
    try {
        let error = new RangeError();
        res = errRangeErrorModule.instanceOf(Type.of(error));
    } catch (e) {
        res = false;
    }
    return res;
}