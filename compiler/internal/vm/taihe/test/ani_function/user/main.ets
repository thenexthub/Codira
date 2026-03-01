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
import * as FuncTest from "function_test";

loadLibrary("ani_function");

const NUMBER_TWENTY: int = 20;

function test_func_no_input_no_output() {
    let foo = FuncTest.makeFoo();
    foo.bar();
}

function test_func_int_input_no_output() {
    let foo = FuncTest.makeFoo();
    foo.bar(12);
}

function test_func_string_input_no_output() {
    let foo = FuncTest.makeFoo();
    foo.bar("zhangsan");
}

function test_func_union_str() {
    let foo = FuncTest.makeFoo();
    foo.bar("lisi", "myunionStr");
}

function test_func_union_number() {
    let foo = FuncTest.makeFoo();
    foo.bar("lisi", 18);
}

function test_func_optional_srting() {
    let foo = FuncTest.makeFoo();
    foo.bar("lisi", 18);
}

function test_void_input_void_output() {
    let foo = FuncTest.makeFoo();
    foo.test_cb_v(() => {
        console.log("void input void output callback!");
    });
}

function test_int_input_void_output() {
    let foo = FuncTest.makeFoo();
    foo.test_cb_i((a) => {
        arktest.assertEQ(a, 1);
    });
}

function test_string_boolean_input() {
    let foo = FuncTest.makeFoo();
    foo.test_cb_s((a, b) => {
        if (b) {
            arktest.assertEQ(a, "hello");
        }
    });
}

function test_string_return_value() {
    let foo = FuncTest.makeFoo();
    let result = foo.test_cb_rs((a) => {
        return a * 2;
    });
    arktest.assertEQ(result, "888");
}

function test_from_callback() {
    let foo = FuncTest.makeFoo();
    let p = new Promise<int>((resolve, reject) => {
        foo.add(10, NUMBER_TWENTY, (error: BusinessError|null, data?: int) => {
            if (error != null && error.message) {
                reject(error);
            } else {
                resolve(data!);
            }
        });
    });
    let res: int;
    let err: BusinessError = new BusinessError();
    try {
        res = await p;
    } catch (e) {
        err = e as BusinessError;
    }
    arktest.assertEQ(res, 30);
}

function test_static_func_add() {
    const result = FuncTest.FooCls.static_func_add(20, 10);
    arktest.assertEQ(result, 30);
}

function test_static_func_sub() {
    const result = FuncTest.FooCls.static_func_sub(20, 10);
    arktest.assertEQ(result, 10);
}

function test_static_property() {
    arktest.assertEQ(FuncTest.FooCls.static_property, 0);
    FuncTest.FooCls.static_property = 123;
    arktest.assertEQ(FuncTest.FooCls.static_property, 123);
}

function test_new_class() {
    let test = new FuncTest.FooCls("hello");
    console.log("test_new_class test is:", test.get());
    arktest.assertEQ(test.get(), "zhangsan");
}

function test_new_class_with_two_args() {
    let test = new FuncTest.FooCls("hello", "test");
    console.log("test_new_class_with_two_args test is:", test.get());
    arktest.assertEQ(test.get(), "zhangsan");
}

function main() {
    console.log("##############start#############");
    const suite = new arktest.ArkTestsuite("FuncTest Test");

    suite.addTest("test func no input no output", test_func_no_input_no_output);
    suite.addTest(
        "test func int input no output", test_func_int_input_no_output);
    suite.addTest(
        "test func string input no output", test_func_string_input_no_output);
    suite.addTest("test func union string", test_func_union_str);
    suite.addTest("test func union number", test_func_union_number);
    suite.addTest("test func optional srting", test_func_optional_srting);
    suite.addTest(
        "test callback no input no output", test_void_input_void_output);
    suite.addTest(
        "test callback int input no output", test_int_input_void_output);
    suite.addTest(
        "test callback string boolean input", test_string_boolean_input);
    suite.addTest("test callback", test_string_return_value);
    suite.addTest("test from callback", test_from_callback);
    suite.addTest("test static func add", test_static_func_add);
    suite.addTest("test static func sub", test_static_func_sub);
    suite.addTest("test new class", test_new_class);
    suite.addTest("test new class with two args", test_new_class_with_two_args);

    suite.addTest("test static property", test_static_property);
    exit(suite.run());
}
