/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

function main(): int {
    const suite = new arktest.ArkTestsuite('IntlLocalesToLanguageTags Tests');
    suite.addTest('testEmptyStringInvalid', testEmptyStringInvalid);
    suite.addTest('testNumericInvalid', testNumericInvalid);
    suite.addTest('testDoubleHyphenInvalid', testDoubleHyphenInvalid);
    suite.addTest('testValidSimpleTag', testValidSimpleTag);
    suite.addTest('testValidWithRegion', testValidWithRegion);
    suite.addTest('testGrandfatheredInvalidTag1', testGrandfatheredInvalidTag1);
    suite.addTest('testGrandfatheredInvalidTag2', testGrandfatheredInvalidTag2);
    return suite.run();
}

function testEmptyStringInvalid() {
    const loc = '';
    arktest.expectError((): void => { Intl.intlLocalesToLanguageTags(loc) });
}

function testNumericInvalid() {
    const loc = '123';
    arktest.expectError((): void => { Intl.intlLocalesToLanguageTags(loc) });
}

function testDoubleHyphenInvalid() {
    const loc = 'en--US';
    arktest.expectError((): void => { Intl.intlLocalesToLanguageTags(loc) });
}

function testValidSimpleTag() {
    const loc = 'en';
    const result = Intl.intlLocalesToLanguageTags(loc);
    arktest.assertEQ(result.toString(), 'en');
}

function testValidWithRegion() {
    const loc = 'zh-CN';
    const result = Intl.intlLocalesToLanguageTags(loc);
    arktest.assertEQ(result.toString(), 'zh-CN');
}

function testGrandfatheredInvalidTag1() {
    const loc = 'i-enochian';
    arktest.expectError((): void => { Intl.intlLocalesToLanguageTags(loc) });
}

function testGrandfatheredInvalidTag2() {
    const loc = 'sgn-BE-FR';
    arktest.expectError((): void => { Intl.intlLocalesToLanguageTags(loc) });
}
