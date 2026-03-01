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
    let suite = new arktest.ArkTestsuite("Intl. NumberFormat resolvedOptions");
    suite.addTest("testResolvedOptionsDefault", testResolvedOptionsDefault);
    suite.addTest("testResolvedOptionsNotDefault", testResolvedOptionsNotDefault);
    suite.addTest("testResolvedOptionsDecimal", testResolvedOptionsDecimal);
    suite.addTest("testResolvedOptionsPercent", testResolvedOptionsPercent);
    suite.addTest("testResolvedOptionsCurrency", testResolvedOptionsCurrency);
    //Tests are from https://gitee.com/openharmony/xts_acts/tree/master/commonlibrary/ets_utils/codeparsingtest/builtinstest/entry/src/ohosTest/ets/test/Intl/Intl.test.ets
    suite.addTest("testResolvedOptions0001", testResolvedOptions0001);
    suite.addTest("testResolvedOptions0002", testResolvedOptions0002);
    suite.addTest("testResolvedOptions0003", testResolvedOptions0003);
    suite.addTest("testResolvedOptions0004", testResolvedOptions0004);
    suite.addTest("testResolvedOptions0005", testResolvedOptions0005);
    suite.addTest("testResolvedOptions0006", testResolvedOptions0006);
    suite.addTest("testResolvedOptions0007", testResolvedOptions0007);
    return suite.run();
}

function testResolvedOptions0001() {
    let options = Intl.NumberFormat.$_invoke().resolvedOptions();
    arktest.assertTrue(options.locale.includes('en-US'));
    arktest.assertEQ(options.numberingSystem, 'latn');
    arktest.assertEQ(options.style, 'decimal');
    arktest.assertEQ(options.minimumIntegerDigits, 1);
    arktest.assertEQ(options.minimumFractionDigits, 0);
    arktest.assertEQ(options.maximumFractionDigits, 3);
    arktest.assertEQ(options.useGrouping, true);
    arktest.assertEQ(options.notation, "standard");
    arktest.assertEQ(options.signDisplay, "auto");
}

function testResolvedOptions0002() {
    let options = Intl.NumberFormat.$_invoke("de").resolvedOptions();
    arktest.assertEQ(options.locale, "de");
}

function testResolvedOptions0003() {
    let options: Intl.NumberFormatOptions = {
        style: "currency", currency: "EUR"
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    arktest.assertEQ(getOptions.locale, 'de');
    arktest.assertEQ(getOptions.numberingSystem, 'latn');
    arktest.assertEQ(getOptions.style, 'currency');
    arktest.assertEQ(getOptions.currency, 'EUR');
    arktest.assertEQ(getOptions.currencyDisplay, 'symbol');
    arktest.assertEQ(getOptions.currencySign, 'standard');
    arktest.assertEQ(getOptions.minimumIntegerDigits, 1);
    arktest.assertEQ(getOptions.minimumFractionDigits, 2);
    arktest.assertEQ(getOptions.maximumFractionDigits, 2);
    arktest.assertEQ(getOptions.useGrouping, true);
    arktest.assertEQ(getOptions.notation, 'standard');
    arktest.assertEQ(getOptions.signDisplay, 'auto');
}

function testResolvedOptions0004() {
    let options: Intl.NumberFormatOptions = {
        style: "percent"
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    arktest.assertEQ(getOptions.locale, 'de');
    arktest.assertEQ(getOptions.numberingSystem, 'latn');
    arktest.assertEQ(getOptions.style, 'percent');
    arktest.assertEQ(getOptions.minimumIntegerDigits, 1);
    arktest.assertEQ(getOptions.minimumFractionDigits, 0);
    arktest.assertEQ(getOptions.maximumFractionDigits, 0);
    arktest.assertEQ(getOptions.useGrouping, true);
    arktest.assertEQ(getOptions.notation, 'standard');
    arktest.assertEQ(getOptions.signDisplay, 'auto');
}

function testResolvedOptions0005() {
    let options: Intl.NumberFormatOptions = {
        style: "currency", currency: "EUR", currencySign: "accounting"
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    arktest.assertEQ(getOptions.locale, 'de');
    arktest.assertEQ(getOptions.numberingSystem, 'latn');
    arktest.assertEQ(getOptions.style, 'currency');
    arktest.assertEQ(getOptions.currency, 'EUR');
    arktest.assertEQ(getOptions.currencyDisplay, 'symbol');
    arktest.assertEQ(getOptions.currencySign, 'accounting');
    arktest.assertEQ(getOptions.minimumIntegerDigits, 1);
    arktest.assertEQ(getOptions.minimumFractionDigits, 2);
    arktest.assertEQ(getOptions.maximumFractionDigits, 2);
    arktest.assertEQ(getOptions.useGrouping, true);
    arktest.assertEQ(getOptions.notation, 'standard');
    arktest.assertEQ(getOptions.signDisplay, 'auto');
}

function testResolvedOptions0006() {
    let options: Intl.NumberFormatOptions = {
        useGrouping: false
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    arktest.assertEQ(getOptions.locale, 'de');
    arktest.assertEQ(getOptions.numberingSystem, 'latn');
    arktest.assertEQ(getOptions.style, 'decimal');
    arktest.assertEQ(getOptions.minimumIntegerDigits, 1);
    arktest.assertEQ(getOptions.minimumFractionDigits, 0);
    arktest.assertEQ(getOptions.maximumFractionDigits, 3);
    arktest.assertEQ(getOptions.useGrouping, false);
    arktest.assertEQ(getOptions.notation, 'standard');
    arktest.assertEQ(getOptions.signDisplay, 'auto');
}

function testResolvedOptions0007() {
    let options: Intl.NumberFormatOptions = {
        minimumIntegerDigits: 10
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    arktest.assertEQ(getOptions.minimumIntegerDigits, 10);
}

function testResolvedOptionsDefault() {
    let expected: Intl.ResolvedNumberFormatOptions = {
            locale: "en",
            numberingSystem: "latn",
            style: "decimal",
            useGrouping: true,
            notation: "standard",
            signDisplay: "auto",
            maximumFractionDigits: 3,
            minimumFractionDigits: 0,
            minimumIntegerDigits: 1
        }
    let result = new Intl.NumberFormat("en", undefined).resolvedOptions();
    let isEqualed = equaledResolvedNumberFormatOptions(result, expected);
    arktest.assertTrue(isEqualed);
    result = new Intl.NumberFormat("en").resolvedOptions();
    isEqualed = equaledResolvedNumberFormatOptions(result, expected);
    arktest.assertTrue(isEqualed);
}

function testResolvedOptionsNotDefault() {
    let options: Intl.NumberFormatOptions = {
        style: "decimal",
        useGrouping: false,
        notation: "compact",
        compactDisplay: "long",
        signDisplay: "auto",
        minimumFractionDigits: 19,
        maximumFractionDigits: 21,
        minimumSignificantDigits: 17,
        maximumSignificantDigits: 20,
        minimumIntegerDigits: 18
    };

    let x = new Intl.NumberFormat("zh", options).resolvedOptions();
    arktest.assertEQ(x.compactDisplay, "long");
    arktest.assertEQ(x.currencySign, undefined);
    arktest.assertEQ(x.currency, undefined);
    arktest.assertEQ(x.currencyDisplay, undefined);
    arktest.assertEQ(x.locale, "zh");
    arktest.assertEQ(x.minimumFractionDigits, 19);
    arktest.assertEQ(x.maximumFractionDigits, 21);
    arktest.assertEQ(x.minimumSignificantDigits, undefined);
    arktest.assertEQ(x.maximumSignificantDigits, undefined);
    arktest.assertEQ(x.minimumIntegerDigits, 18);
    arktest.assertEQ(x.notation, "compact");
    arktest.assertEQ(x.numberingSystem, "latn");
    arktest.assertEQ(x.signDisplay, "auto");
    arktest.assertEQ(x.style, "decimal");
    arktest.assertEQ(x.unit, undefined);
    arktest.assertEQ(x.unitDisplay, undefined);
    arktest.assertEQ(x.useGrouping, false);
}


function testResolvedOptionsDecimal() {
    let options: Intl.NumberFormatOptions = {
        style: "decimal"
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    arktest.assertEQ(getOptions.locale, "de");
    arktest.assertEQ(getOptions.numberingSystem, "latn");
    arktest.assertEQ(getOptions.style, "decimal");
    arktest.assertEQ(getOptions.minimumIntegerDigits, 1.0);
    arktest.assertEQ(getOptions.minimumFractionDigits, 0.0);
    arktest.assertEQ(getOptions.maximumFractionDigits, 3.0);
    arktest.assertEQ(getOptions.useGrouping, true);
    arktest.assertEQ(getOptions.notation, "standard");
    arktest.assertEQ(getOptions.signDisplay, "auto");
}

function testResolvedOptionsPercent() {
    let options: Intl.NumberFormatOptions = {
        style: "percent"
    }
    let getOptions = new Intl.NumberFormat("de", options).resolvedOptions();
    arktest.assertEQ(getOptions.locale, "de");
    arktest.assertEQ(getOptions.numberingSystem, "latn");
    arktest.assertEQ(getOptions.style, "percent");
    arktest.assertEQ(getOptions.minimumIntegerDigits, 1.0);
    arktest.assertEQ(getOptions.minimumFractionDigits, 0.0);
    arktest.assertEQ(getOptions.maximumFractionDigits, 0.0);
    arktest.assertEQ(getOptions.useGrouping, true)
    arktest.assertEQ(getOptions.notation, "standard");
    arktest.assertEQ(getOptions.signDisplay, "auto");
}

function testResolvedOptionsCurrency() {
    let options: Intl.NumberFormatOptions = {
        style: "currency", currency: "CNY"
    }
    let getOptions = new Intl.NumberFormat("zh", options).resolvedOptions();
    arktest.assertEQ(getOptions.locale, "zh");
    arktest.assertEQ(getOptions.numberingSystem, "latn");
    arktest.assertEQ(getOptions.style, "currency");
    arktest.assertEQ(getOptions.currency, "CNY");
    arktest.assertEQ(getOptions.currencyDisplay, "symbol");
    arktest.assertEQ(getOptions.currencySign, "standard");
    arktest.assertEQ(getOptions.minimumIntegerDigits, 1.0);
    arktest.assertEQ(getOptions.minimumFractionDigits, 2.0);
    arktest.assertEQ(getOptions.maximumFractionDigits, 2.0);
    arktest.assertEQ(getOptions.useGrouping, true);
    arktest.assertEQ(getOptions.notation, "standard");
    arktest.assertEQ(getOptions.signDisplay, "auto");
}

function equaledResolvedNumberFormatOptions(a: Intl.ResolvedNumberFormatOptions, b: Intl.ResolvedNumberFormatOptions): boolean {
    arktest.assertEQ(a.compactDisplay,b.compactDisplay);
    arktest.assertEQ(a.currencySign, b.currencySign);
    arktest.assertEQ(a.currency, b.currency);
    arktest.assertEQ(a.currencyDisplay, b.currencyDisplay);
    arktest.assertEQ(a.locale, b.locale);
    arktest.assertEQ(a.maximumFractionDigits, b.maximumFractionDigits);
    arktest.assertEQ(a.maximumSignificantDigits, b.maximumSignificantDigits);
    arktest.assertEQ(a.minimumFractionDigits, b.minimumFractionDigits);
    arktest.assertEQ(a.minimumIntegerDigits, b.minimumIntegerDigits);
    arktest.assertEQ(a.minimumSignificantDigits, b.minimumSignificantDigits);
    arktest.assertEQ(a.notation, b.notation);
    arktest.assertEQ(a.numberingSystem, b.numberingSystem);
    arktest.assertEQ(a.signDisplay, b.signDisplay);
    arktest.assertEQ(a.style, b.style);
    arktest.assertEQ(a.unit, b.unit);
    arktest.assertEQ(a.unitDisplay, b.unitDisplay);
    arktest.assertEQ(a.useGrouping, b.useGrouping);
    return true;
}
