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
    suite.addTest("testSupportedLocalesOf0007", testSupportedLocalesOf0007);
    suite.addTest("testSupportedLocalesOf0010", testSupportedLocalesOf0010);
    return suite.run();
}


function testSupportedLocalesOf0007() {
    let locales: string[] = ['test', 'id-u-co-pinyin', 'de-ID', 'en-US'];
    arktest.expectError((): void => { Intl.NumberFormat.supportedLocalesOf(locales) });
}

function testSupportedLocalesOf0010() {
    let locales: string[] = ['test', 'id-u-co-pinyin', 'de-ID', 'en-US'];
    let options: Intl.NumberFormatOptions = {
        localeMatcher: 'best fit'
    }
    arktest.expectError((): void => { Intl.NumberFormat.supportedLocalesOf(locales) });
}
