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
import * as ani_test from "ani_test";

loadLibrary("ani_test");

function test_data_struct() {
    // Test struct creation and display
    const option: ani_test.Data = {
        src: "path/to/source",
        dest: 123,
        files: ["file1.txt", "file2.txt", "file3.txt"]
    };
    ani_test.showData(option);

    // Test makeData returns expected structure
    const result = ani_test.makeData();
    arktest.assertEQ(result.src, "C++ Object");
    arktest.assertEQ(result.dest, 1);
    arktest.assertEQ(result.files.length, 2);
    arktest.assertEQ(result.files[0], "file.txt");
    arktest.assertEQ(result.files[1], "file.txt");
}

function test_union() {
    // Test showUnion with different types
    ani_test.showUnion(null);
    ani_test.showUnion((1).toInt());
    ani_test.showUnion(2.0f);
    ani_test.showUnion("Hello from STS!");

    // Test makeUnion returns expected values
    const union0 = ani_test.makeUnion(0);
    arktest.assertEQ(union0, null);

    const union1 = ani_test.makeUnion(1);
    arktest.assertEQ(union1, 100);

    const union2 = ani_test.makeUnion(2);
    arktest.assertEQ(union2, 0.5);

    const union3 = ani_test.makeUnion(3);
    arktest.assertEQ(union3, "Hello from C++!");
}

function test_optional() {
    // Test showOptionalInt with value and undefined
    ani_test.showOptionalInt((100).toInt());
    ani_test.showOptionalInt(undefined);

    // Test makeOptionalInt returns expected values
    const optTrue = ani_test.makeOptionalInt(true);
    arktest.assertEQ(optTrue, 10);

    const optFalse = ani_test.makeOptionalInt(false);
    arktest.assertEQ(optFalse, undefined);
}

function test_array() {
    // Test showArrayInt with array
    ani_test.showArrayInt([1, 2, 3]);

    // Test makeArrayInt returns expected array
    const arr = ani_test.makeArrayInt(10, 123);
    arktest.assertEQ(arr.length, 10);
    for (let i = 0; i < arr.length; i++) {
        arktest.assertEQ(arr[i], 123);
    }
}

// Test UserFoo implementation
class UserFoo implements ani_test.Foo {
    name: string;
    constructor(name: string) {
        this.name = name;
    }
    bar(): void {
        console.log("UserFoo(" + this.name + ") is calling bar()");
    }
}

function test_interface() {
    // Test makeFoo implementation
    let impl_foo_list = ani_test.makeFoo(["A", "B", "C"]);
    arktest.assertEQ(impl_foo_list.length, 3);
    for (let i = 0; i < impl_foo_list.length; i++) {
        impl_foo_list[i].bar();
    }

    const X: ani_test.Foo = new UserFoo("X")
    const Y: ani_test.Foo = new UserFoo("Y")
    const Z: ani_test.Foo = new UserFoo("Z")
    const userFoos: Array<ani_test.Foo> = [X, Y, Z];
    arktest.assertEQ(userFoos.length, 3);
    ani_test.callBar(userFoos);
}

function main() {
    const suite = new arktest.ArkTestsuite("Ani Test Suite");
    suite.addTest("Test Data Struct", test_data_struct);
    suite.addTest("Test Union", test_union);
    suite.addTest("Test Optional", test_optional);
    suite.addTest("Test Array", test_array);
    suite.addTest("Test Interface", test_interface);
    exit(suite.run());
}
