/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function testMessageValidValue() {
    let errObj1 = new Error("ErrorMessage");
    errObj1.name = "Tag";
    arktest.assertEQ(errObj1.toString(), "Tag: ErrorMessage", 'errObj1.toString()');

    let errObj2 = new Error("ErrorMessage");
    errObj2.name = "";
    arktest.assertEQ(errObj2.toString(), "ErrorMessage", 'errObj2.toString()');

    let errObj3 = new Error("ErrorMessage");
    arktest.assertEQ(errObj3.toString(), "Error: ErrorMessage", 'errObj3.toString()');
}

function testMessageEmptyValue() {
    let errObj4 = new Error("");
    errObj4.name = "Tag";
    arktest.assertEQ(errObj4.toString(), "Tag", 'errObj4.toString()');

    let errObj5 = new Error("");
    errObj5.name = "";
    arktest.assertEQ(errObj5.toString(), "", 'errObj5.toString()');

    let errObj6 = new Error("");
    arktest.assertEQ(errObj6.toString(), "Error", 'errObj6.toString()');
}

function testMessageNullValue() {
    let errObj7 = new Error();
    errObj7.name = "Tag";
    arktest.assertEQ(errObj7.toString(), "Tag", 'errObj7.toString()');

    let errObj8 = new Error();
    errObj8.name = "";
    arktest.assertEQ(errObj8.toString(), "", 'errObj8.toString()');

    let errObj9 = new Error();
    arktest.assertEQ(errObj9.toString(), "Error", 'errObj9.toString()');
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Error.toString");
    suite.addTest("Test MessageValidValue", testMessageValidValue);
    suite.addTest("Test MessageEmptyValue", testMessageEmptyValue);
    suite.addTest("Test MessageNullValue", testMessageNullValue);
    return suite.run();
}