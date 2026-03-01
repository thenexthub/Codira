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

function testStringlocaleCompare1Param() {
    let str:string = "5";
    arktest.assertEQ(str.localeCompare('3'), 1);
    arktest.assertEQ(str.localeCompare('7'), -1);
    arktest.assertEQ(str.localeCompare('5'), 0);
}

function testStringlocaleCompare2Param() {
    let str:string = "5";
    arktest.assertEQ(str.localeCompare('3', "zh-CN"), 1);
    arktest.assertEQ(str.localeCompare('3', undefined), 1);
    arktest.assertEQ(str.localeCompare('7', "zh-CN"), -1);
    arktest.assertEQ(str.localeCompare('5', "zh-CN"), 0);
}

function testStringlocaleCompare3Param() {
    let su: Intl.CollatorOptions = {
        usage: 'sort',
        localeMatcher: "lookup"
    }

    let str:string = "5";
    arktest.assertEQ(str.localeCompare('3', "zh-CN", su), 1);
    arktest.assertEQ(str.localeCompare('7', "zh-CN", su), -1);
    arktest.assertEQ(str.localeCompare('5', "zh-CN", su), 0);
    arktest.assertEQ(str.localeCompare('3', ["zh-CN", "en-US"], su), 1);
    arktest.assertEQ(str.localeCompare('7', ["zh-CN", "en-US"], su), -1);
    arktest.assertEQ(str.localeCompare('5', ["zh-CN", "en-US"], su), 0);
    arktest.assertEQ(str.localeCompare('3', ["error-locale", "en-US"], su), 1);
    arktest.assertEQ(str.localeCompare('7', ["error-locale", "en-US"], su), -1);
    arktest.assertEQ(str.localeCompare('5', ["error-locale", "en-US"], su), 0);
}

function main(): int {
    const suite = new arktest.ArkTestsuite("String.localeCompare");
    suite.addTest("Test StringlocaleCompare1Param", testStringlocaleCompare1Param);
    suite.addTest("Test StringlocaleCompare2Param", testStringlocaleCompare2Param);
    suite.addTest("Test StringlocaleCompare3Param", testStringlocaleCompare3Param);
    return suite.run();
}
