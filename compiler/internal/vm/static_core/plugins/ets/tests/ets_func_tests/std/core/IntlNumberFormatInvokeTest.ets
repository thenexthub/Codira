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
    let suite = new arktest.ArkTestsuite("Intl. NumberFormat");
    suite.addTest("$_invoke", testInvoke);
    suite.addTest("$_invoke currency style error", testInvokeCurrencyStyleError);
    suite.addTest("$_invoke currency unit error", testInvokeCurrencyStyleError);
    suite.addTest("$_invoke undefined locale", testInvokeUndefinedLocale);
    suite.addTest("$_invoke undefined locale undefined options", testInvokeUndefinedLocaleUndefinedOptions);
    suite.addTest("$_invoke minMaxFracDigits", testInvokeMinMaxFracDigits);
    suite.addTest("$_invoke minMaxSignDigits", testInvokeMinMaxSignDigits);
    return suite.run();
}

function testInvoke() {
    let expected = new Intl.NumberFormat("en-EN");
    let result = Intl.NumberFormat.$_invoke("en-EN");
    let value = -123456.789;
    let isEqualed = equaledNumberFormats(result, expected, value);
    arktest.assertTrue(isEqualed, "Error. Got: " + result.format(value) + ". Expected: " + expected.format(value));
}

function testInvokeCurrencyStyleError() {
    let expectedError = new RangeError("Style is currency but currency is undefined");
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke("en-EN", { style: "currency" } as Intl.NumberFormatOptions) }, expectedError);
}

function testInvokeUnitStyleError() {
    let expectedError = new RangeError("Style is unit but unit is undefined");
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke("en-EN", { style: "unit" } as Intl.NumberFormatOptions) }, expectedError);
}

function testInvokeUndefinedLocale() {
    let numberFormat = new Intl.NumberFormat(undefined);
    arktest.assertTrue(numberFormat != undefined && numberFormat != null);
    let getOptions = numberFormat.resolvedOptions();
    let s: string = printResolvedOptions(getOptions);
    arktest.assertEQ(getOptions.locale, Intl.Locale.defaultTag(), "Error. Got: " + s);
    arktest.assertEQ(getOptions.numberingSystem, "latn", "Error. Got: " + s);
    arktest.assertEQ(getOptions.style, "decimal", "Error. Got: " + s);
    arktest.assertEQ(getOptions.minimumIntegerDigits, 1.0, "Error. Got: " + s);
    arktest.assertEQ(getOptions.minimumFractionDigits, 0.0, "Error. Got: " + s);
    arktest.assertEQ(getOptions.maximumFractionDigits, 3.0, "Error. Got: " + s);
    arktest.assertEQ(getOptions.useGrouping, true, "Error. Got: " + s);
    arktest.assertEQ(getOptions.notation, "standard", "Error. Got: " + s);
    arktest.assertEQ(getOptions.signDisplay, "auto", "Error. Got: " + s);
}

function testInvokeUndefinedLocaleUndefinedOptions() {
    let numberFormat = new Intl.NumberFormat(undefined, undefined);
    arktest.assertTrue(numberFormat != undefined && numberFormat != null);
    let getOptions = numberFormat.resolvedOptions();
    let s: string = printResolvedOptions(getOptions);
    arktest.assertEQ(getOptions.locale, Intl.Locale.defaultTag(), "Error. Got: " + s);
    arktest.assertEQ(getOptions.numberingSystem, "latn", "Error. Got: " + s);
    arktest.assertEQ(getOptions.style, "decimal", "Error. Got: " + s);
    arktest.assertEQ(getOptions.minimumIntegerDigits, 1.0, "Error. Got: " + s);
    arktest.assertEQ(getOptions.minimumFractionDigits, 0.0, "Error. Got: " + s);
    arktest.assertEQ(getOptions.maximumFractionDigits, 3.0, "Error. Got: " + s);
    arktest.assertEQ(getOptions.useGrouping, true, "Error. Got: " + s);
    arktest.assertEQ(getOptions.notation, "standard", "Error. Got: " + s);
    arktest.assertEQ(getOptions.signDisplay, "auto", "Error. Got: " + s);
}

function equaledNumberFormats(a: Intl.NumberFormat, b: Intl.NumberFormat, value: number | bigint): boolean {
    return a.format(value) == b.format(value);
}

function printResolvedOptions(options: Intl.ResolvedNumberFormatOptions): string {
    let parts = new Array<string>
    let addValue = (value: number | string | boolean | undefined, name: string) => {
        let s = name + " : " + (value === undefined ? "undefined" : String(value));
        parts.push(s);
    };
    addValue(options.compactDisplay, "compactDisplay");
    addValue(options.currencySign, "currencySign");
    addValue(options.currency, "currency");
    addValue(options.currencyDisplay, "currencyDisplay");
    addValue(options.locale, "locale");
    addValue(options.maximumFractionDigits, "maximumFractionDigits");
    addValue(options.minimumFractionDigits, "minimumFractionDigits");
    addValue(options.maximumSignificantDigits, "maximumSignificantDigits");
    addValue(options.minimumSignificantDigits, "minimumSignificantDigits");
    addValue(options.minimumIntegerDigits, "minimumIntegerDigits");
    addValue(options.notation, "notation");
    addValue(options.numberingSystem, "numberingSystem");
    addValue(options.signDisplay, "signDisplay");
    addValue(options.style, "style");
    addValue(options.unit, "unit");
    addValue(options.unitDisplay, "unitDisplay");
    addValue(options.useGrouping, "useGrouping");
    return parts.join(",");
}

function testInvokeMinMaxFracDigits() {

    let options3: Intl.NumberFormatOptions = {
        maximumFractionDigits: 5,
        minimumFractionDigits: 3
    }
    let getOptions = Intl.NumberFormat.$_invoke("zh", options3).resolvedOptions();
    arktest.assertEQ(getOptions.minimumFractionDigits, 3.0);
    arktest.assertEQ(getOptions.maximumFractionDigits, 5.0);
    arktest.assertTrue(getOptions.maximumFractionDigits > getOptions.minimumFractionDigits);
}

function testInvokeMinMaxSignDigits() {
    let options3: Intl.NumberFormatOptions = {
        maximumSignificantDigits: 18,
        minimumSignificantDigits: 17
    }
    let getOptions = Intl.NumberFormat.$_invoke("zh", options3).resolvedOptions();
    arktest.assertEQ(getOptions.maximumSignificantDigits!, 18.0);
    arktest.assertEQ(getOptions.minimumSignificantDigits!, 17.0);
    arktest.assertTrue(getOptions.maximumSignificantDigits! > getOptions.minimumSignificantDigits!);
}
