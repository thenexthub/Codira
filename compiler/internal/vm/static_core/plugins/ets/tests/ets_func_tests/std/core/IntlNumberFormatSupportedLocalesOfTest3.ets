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

function main(): int {
    let suite = new arktest.ArkTestsuite("Intl. NumberFormat supportedLocalesOf");
    // Tests are from https://gitee.com/openharmony/xts_acts/tree/master/commonlibrary/ets_utils/codeparsingtest/builtinstest/entry/src/ohosTest/ets/test/Intl/Intl.test.ets
    suite.addTest("testSupportedLocalesOf0001", testSupportedLocalesOf0001);
    suite.addTest("testSupportedLocalesOf0005", testSupportedLocalesOf0005);
    suite.addTest("testSupportedLocalesOf0006", testSupportedLocalesOf0006);
    suite.addTest("testSupportedLocalesOf0008", testSupportedLocalesOf0008);
    return suite.run();
}

function testSupportedLocalesOf0001() {
    let locales: string = 'de-ID';
    let result = Intl.NumberFormat.supportedLocalesOf(locales);
    arktest.assertEQ(result.toString(), ["de-ID"].toString());
    arktest.assertTrue(Array.isArray(result));
}

function testSupportedLocalesOf0005() {
    let locales: string[] = ['id-u-co-pinyin'];
    let result = Intl.NumberFormat.supportedLocalesOf(locales);
    arktest.assertEQ(result.toString(), 'id-u-co-pinyin');
    arktest.assertTrue(Array.isArray(result));
}

function testSupportedLocalesOf0006() {
    let locales: string[] = ['id-u-co-pinyin', 'de-ID', 'en-US'];
    let result = Intl.NumberFormat.supportedLocalesOf(locales);
    arktest.assertEQ(result.join(','), ['id-u-co-pinyin', 'de-ID', 'en-US'].toString());
    arktest.assertTrue(Array.isArray(result));
}

function testSupportedLocalesOf0008() {
    let locales: string[] = ['ban', 'id-u-co-pinyin', 'de-ID', 'en-US'];
    let result = Intl.NumberFormat.supportedLocalesOf(locales);
    arktest.assertEQ(result.join(','), ['id-u-co-pinyin', 'de-ID', 'en-US'].toString());
    arktest.assertTrue(Array.isArray(result));
}
