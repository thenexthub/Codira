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
import * as lib from "staticTest";

loadLibrary("ani_static");

function test_sumSync() {
    const result = lib.IBase.sumSync(10, 20);
    console.log("sumSync result:", result);
    arktest.assertEQ(result, 200);
}

function test_addSync() {
    const result = lib.IBase.addSync(10, 20);
    arktest.assertEQ(result, 30);
}

function test_new_class() {
    let test = new lib.IBase("hello");
    console.log("new IBase get:", test.get());
    arktest.assertEQ(test.get(), "hello");
}

// Frontend javalike constructor overloading bug
// function test_new_class_with_two_args() {
//     let test_1 = new lib.IBase("hello", "test");
//     arktest.assertEQ(test_1.get(), "test");
// }

function main() {
    console.log("##############start#############");
    const suite = new arktest.ArkTestsuite("IBase Tests");

    suite.addTest("test sumSync", test_sumSync);
    suite.addTest("test addSync", test_addSync);
    suite.addTest("test new class", test_new_class);
    // Frontend javalike constructor overloading bug
    // suite.addTest("test new class with two args",
    // test_new_class_with_two_args);

    exit(suite.run());
}
