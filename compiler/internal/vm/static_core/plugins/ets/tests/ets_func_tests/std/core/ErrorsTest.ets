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

function testStringIndexOutOfBoundsErrorExtendsRangeError() {
    let str: string = "123";
    try {
        str[-1];
    } catch (e) {
        arktest.assertTrue(e instanceof StringIndexOutOfBoundsError);
        arktest.assertEQ(e.name, "StringIndexOutOfBoundsError");
        arktest.assertTrue(e instanceof RangeError);
    }

    try {
        str[5];
    } catch (e) {
        arktest.assertTrue(e instanceof StringIndexOutOfBoundsError);
        arktest.assertEQ(e.name, "StringIndexOutOfBoundsError");
        arktest.assertTrue(e instanceof RangeError);
    }
}

function testArrayIndexOutOfBoundsErrorExtendsRangeError() {
    try {
        throw new ArrayIndexOutOfBoundsError("ArrayIndexOutOfBoundsError");
    } catch (e) {
        arktest.assertEQ(e.name, "ArrayIndexOutOfBoundsError");
        arktest.assertTrue(e instanceof RangeError);
    }
}

function testIndexOutOfBoundsErrorExtendsRangeError() {
    try {
        throw new IndexOutOfBoundsError("IndexOutOfBoundsError");
    } catch (e) {
        arktest.assertEQ(e.name, "IndexOutOfBoundsError");
        arktest.assertTrue(e instanceof RangeError);
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Errors.testErrors");
    suite.addTest("Test StringIndexOutOfBoundsErrorExtendsRangeError", testStringIndexOutOfBoundsErrorExtendsRangeError);
    suite.addTest("Test ArrayIndexOutOfBoundsErrorExtendsRangeError", testArrayIndexOutOfBoundsErrorExtendsRangeError);
    suite.addTest("Test IndexOutOfBoundsErrorExtendsRangeError", testIndexOutOfBoundsErrorExtendsRangeError);
    return suite.run();
}
