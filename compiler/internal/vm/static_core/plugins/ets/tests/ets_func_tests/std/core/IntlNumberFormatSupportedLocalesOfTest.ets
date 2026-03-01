/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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
    let suite = new arktest.ArkTestsuite('Intl. NumberFormat supportedLocalesOf');
    suite.addTest('supportedLocalesOf', testSupportedLocalesOf);
    suite.addTest('supportedLocalesOfEmpty', testSupportedLocalesOfEmpty);
    suite.addTest('supportedLocalesOfUndefined', testSupportedLocalesOfUndefined);
    // Tests are from https://gitee.com/openharmony/xts_acts/tree/master/commonlibrary/ets_utils/codeparsingtest/builtinstest/entry/src/ohosTest/ets/test/Intl/Intl.test.ets
    suite.addTest('testSupportedLocalesOf0002', testSupportedLocalesOf0002);
    suite.addTest('testSupportedLocalesOf0003', testSupportedLocalesOf0003);
    suite.addTest('testSupportedLocalesOf0003', testSupportedLocalesOf0004);
    return suite.run();
}

function testSupportedLocalesOf() {
    let en: string = Intl.intlBestFitLocale('en-US');
    let cn: string = Intl.intlBestFitLocale('zh-CN');
    let ru: string = Intl.intlBestFitLocale('ru-RU');
    let localeList: string[] = [ru, cn, en];
    let expected: string[] = [ru, cn, en];
    let result: string[] = Intl.NumberFormat.supportedLocalesOf(localeList);
    arktest.assertEQ(expected[0], result[0], 'Error. Got: ' + result[0] + '. Expected: ' + expected[0]);
    arktest.assertEQ(expected[1], result[1], 'Error. Got: ' + result[1] + '. Expected: ' + expected[1]);
    arktest.assertEQ(expected[2], result[2], 'Error. Got: ' + result[2] + '. Expected: ' + expected[2]);
}

function testSupportedLocalesOfEmpty() {
    let locales: string = ''
    let expectedError = new Error('Incorrect locale information provided');
    arktest.expectError((): void => { Intl.NumberFormat.supportedLocalesOf(locales); }, expectedError);
}

function testSupportedLocalesOfUndefined() {
    let locales: string = ''
    let expectedError = new Error('Incorrect locale information provided');
    arktest.expectError((): void => { Intl.NumberFormat.supportedLocalesOf(locales, undefined); }, expectedError);
}

function testSupportedLocalesOf0002() {
    let locales: string = '';
    arktest.expectError((): void => { Intl.NumberFormat.supportedLocalesOf(locales) });
}

function testSupportedLocalesOf0003() {
    let locales: string = 'test';
    arktest.expectError((): void => { Intl.NumberFormat.supportedLocalesOf(locales) });
}

function testSupportedLocalesOf0004() {
    let locales: string[] = [];
    let result = Intl.NumberFormat.supportedLocalesOf(locales);
    arktest.assertEQ(result.toString(), [''].toString());
    arktest.assertTrue(Array.isArray(result));
}
