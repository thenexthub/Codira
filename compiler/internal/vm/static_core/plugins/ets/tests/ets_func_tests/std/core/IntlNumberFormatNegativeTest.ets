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

/*---
desc: Expects Compile Time Error when invalid value is assigned into property of OptionsNumberFormat
assert: only valid values of OptionsNumberFormat properties are acceptable
tags: [negative, compile-only]
---*/

function main(): int {
    const suite = new arktest.ArkTestsuite("Intl. NumberFormat with Compile Time Errors on fault options");
    // Tests from https://gitee.com/openharmony/xts_acts/tree/master/commonlibrary/ets_utils/codeparsingtest/builtinstest/entry/src/ohosTest/ets/test/Intl/Intl.test.ets
    suite.addTest("testNumberFormatConstructorNew0012", testNumberFormatConstructorNew0012);
    suite.addTest("testNumberFormatConstructorNew0025", testNumberFormatConstructorNew0025);
    return suite.run();
}

function testNumberFormatConstructorNew0012() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        localeMatcher: "test"
    }
    let numberFormat = new Intl.NumberFormat.$_invoke(locales, options);
}

function testNumberFormatConstructorNew0025() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        currencySign: "test"
    }
    let numberFormat = new Intl.NumberFormat(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
}
