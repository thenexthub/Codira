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
    const suite = new arktest.ArkTestsuite("Intl. NumberFormatOptions ResolvedNumberFormatOptions check possibility to read fields");
    suite.addTest("testNumberFormatOptions", testNumberFormatOptions);
    suite.addTest("testResolvedNumberFormatOptions", testResolvedNumberFormatOptions);
    return suite.run();
}

function testNumberFormatOptions() {
    let x: Intl.NumberFormatOptions = {};
    arktest.assertEQ(x.compactDisplay, undefined);
    arktest.assertEQ(x.currency, undefined);
    arktest.assertEQ(x.currencyDisplay, undefined);
    arktest.assertEQ(x.currencySign, undefined);
    arktest.assertEQ(x.localeMatcher, undefined);
    arktest.assertEQ(x.maximumFractionDigits, undefined);
    arktest.assertEQ(x.maximumSignificantDigits, undefined);
    arktest.assertEQ(x.minimumFractionDigits, undefined);
    arktest.assertEQ(x.minimumIntegerDigits, undefined);
    arktest.assertEQ(x.minimumSignificantDigits, undefined);
    arktest.assertEQ(x.notation, undefined);
    arktest.assertEQ(x.signDisplay, undefined);
    arktest.assertEQ(x.style, undefined);
    arktest.assertEQ(x.unit, undefined);
    arktest.assertEQ(x.unitDisplay, undefined);
    arktest.assertEQ(x.useGrouping, undefined);
}

function testResolvedNumberFormatOptions() {
    let x: Intl.ResolvedNumberFormatOptions = {
        // TS requires to define the following properties
        locale: "",
        style: "decimal",
        numberingSystem: "latn",
        minimumIntegerDigits: 1,
        useGrouping: false,
        notation: "compact",
        signDisplay: "auto",
        // The following properties couldn't be "undefined", so define them by example:
        maximumFractionDigits: 0,
        minimumFractionDigits: 0
    };
    arktest.assertEQ(x.compactDisplay, undefined);
    arktest.assertEQ(x.currencySign, undefined);
    arktest.assertEQ(x.currency, undefined);
    arktest.assertEQ(x.currencyDisplay, undefined);
    arktest.assertEQ(x.locale, "");
    arktest.assertEQ(x.maximumFractionDigits, 0);
    arktest.assertEQ(x.maximumSignificantDigits, undefined);
    arktest.assertEQ(x.minimumFractionDigits, 0);
    arktest.assertEQ(x.minimumIntegerDigits, 1);
    arktest.assertEQ(x.minimumSignificantDigits, undefined);
    arktest.assertEQ(x.notation, "compact");
    arktest.assertEQ(x.numberingSystem, "latn");
    arktest.assertEQ(x.signDisplay, "auto");
    arktest.assertEQ(x.style, "decimal");
    arktest.assertEQ(x.unit, undefined);
    arktest.assertEQ(x.unitDisplay, undefined);
    arktest.assertEQ(x.useGrouping, false);
}
