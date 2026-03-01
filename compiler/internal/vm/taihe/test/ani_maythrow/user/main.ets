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
import {BusinessError} from "@ohos.base";
import {getDataMaythrow, maythrow, noReturnBusinessError, noReturnMaythrow} from "maythrow";

loadLibrary("ani_maythrow");

function check_no_error() {
    let res = maythrow(1)
    arktest.assertEQ(res, 11)
}

function check_error() {
    let errorMsg: string = "";
    try {
        let res = maythrow(0)
    } catch (e) {
        errorMsg = e.toString();
    }
    arktest.assertEQ(errorMsg, "Error: some error happen");
}

function check_error_when_return_complex_object() {
    let errorMsg: string = "";
    try {
        let res = getDataMaythrow()
        console.log(res)
    } catch (e) {
        errorMsg = e.toString();
    }
    arktest.assertEQ(errorMsg, "Error: error in getDataMaythrow");
}

function check_error_when_no_return() {
    let errorMsg: string = "";
    try {
        noReturnMaythrow()
    } catch (e) {
        errorMsg = e.toString();
    }
    arktest.assertEQ(errorMsg, "Error: error in noReturnMaythrow");
}

function check_business_error() {
    let error_code: number = 0
    let errorMsg: string = "";
    try {
        noReturnBusinessError()
    } catch (e) {
        e = e as BusinessError
        error_code = e.code
        errorMsg = e.toString();
    }
    arktest.assertEQ(error_code, 5)
    arktest.assertEQ(errorMsg, "Error: error in noReturnBusinessError")
}

function main() {
    console.log("##############start#############")
    const suite = new arktest.ArkTestsuite("throw error ut")

    suite.addTest("check no error", check_no_error)
    suite.addTest("check error", check_error)
    suite.addTest(
        "check error when return complex object",
        check_error_when_return_complex_object)
    suite.addTest("check error when no return", check_error_when_no_return)
    suite.addTest("check business error", check_business_error)
    exit(suite.run())
}
