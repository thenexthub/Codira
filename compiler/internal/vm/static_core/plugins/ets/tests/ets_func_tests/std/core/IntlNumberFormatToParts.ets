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

class NumberFormatOptionsBuilder
{
    // Default decimal style with default precision
    compactDisplay?: "short" | "long" = undefined;
    currency?: string = undefined;
    currencyDisplay?: "code" | "symbol" | "narrowSymbol" | "name" = undefined;
    currencySign?: "standard" | "accounting" = undefined;
    localeMatcher?: "lookup" | "best fit" = undefined;
    maximumFractionDigits?: number = 3;
    maximumSignificantDigits?: number = undefined;
    minimumFractionDigits?: number = 0;
    minimumIntegerDigits?: number = 1;
    minimumSignificantDigits?: number = undefined;
    notation?: "standard" | "scientific" | "engineering" | "compact" = "standard";
    signDisplay?: "auto" | "never" | "always" | "exceptZero" = "auto";
    style?: "decimal" | "percent" | "currency" | "unit" = "decimal";
    unit?: string = undefined;
    unitDisplay?: "short" | "long" | "narrow" = undefined;
    useGrouping?: boolean = true;

    static newDefault(): NumberFormatOptionsBuilder {
        return new NumberFormatOptionsBuilder();
    }

    static newClear(): NumberFormatOptionsBuilder {
        let options = NumberFormatOptionsBuilder.newDefault();
        options.compactDisplay = undefined;
        options.currency = undefined;
        options.currencyDisplay = undefined;
        options.currencySign = undefined;
        options.localeMatcher = undefined;
        options.maximumFractionDigits = undefined;
        options.maximumSignificantDigits = undefined;
        options.minimumFractionDigits = undefined;
        options.minimumIntegerDigits = undefined;
        options.minimumSignificantDigits = undefined;
        options.notation = undefined;
        options.signDisplay = undefined;
        options.style = undefined;
        options.unit = undefined;
        options.unitDisplay = undefined;
        options.useGrouping = undefined;
        return options;
    }

    build(): Intl.NumberFormatOptions {
        return {
            compactDisplay: this.compactDisplay,
            currency: this.currency,
            currencyDisplay: this.currencyDisplay,
            currencySign: this.currencySign,
            localeMatcher: this.localeMatcher,
            maximumFractionDigits: this.maximumFractionDigits,
            maximumSignificantDigits: this.maximumSignificantDigits,
            minimumFractionDigits: this.minimumFractionDigits,
            minimumIntegerDigits: this.minimumIntegerDigits,
            minimumSignificantDigits: this.minimumSignificantDigits,
            notation: this.notation,
            signDisplay: this.signDisplay,
            style: this.style,
            unit: this.unit,
            unitDisplay: this.unitDisplay,
            useGrouping: this.useGrouping,
        }
    }

    public withCompactDisplay(compactDisplay: "short" | "long"): NumberFormatOptionsBuilder {
        this.compactDisplay = compactDisplay;
        return this;
    }

    public withCurrency(currency: string): NumberFormatOptionsBuilder {
        this.currency = currency;
        return this;
    }

    public withCurrencyDisplay(currencyDisplay: "code" | "symbol" | "narrowSymbol" | "name"): NumberFormatOptionsBuilder {
        this.currencyDisplay = currencyDisplay;
        return this;
    }

    public withCurrencySign(currencySign: "standard" | "accounting"): NumberFormatOptionsBuilder {
        this.currencySign = currencySign;
        return this;
    }

    public withLocaleMatcher(localeMatcher: "lookup" | "best fit"): NumberFormatOptionsBuilder {
        this.localeMatcher = localeMatcher;
        return this;
    }

    public withMaximumFractionDigits(maximumFractionDigits: number): NumberFormatOptionsBuilder {
        this.maximumFractionDigits = maximumFractionDigits;
        return this;
    }

    public withMaximumSignificantDigits(maximumSignificantDigits: number): NumberFormatOptionsBuilder {
        this.maximumSignificantDigits = maximumSignificantDigits;
        return this;
    }

    public withMinimumFractionDigits(minimumFractionDigits: number): NumberFormatOptionsBuilder {
        this.minimumFractionDigits = minimumFractionDigits;
        return this;
    }

    public withMinimumIntegerDigits(minimumIntegerDigits: number): NumberFormatOptionsBuilder {
        this.minimumIntegerDigits = minimumIntegerDigits;
        return this;
    }

    public withMinimumSignificantDigits(minimumSignificantDigits: number): NumberFormatOptionsBuilder {
        this.minimumSignificantDigits = minimumSignificantDigits;
        return this;
    }

    public withNotation(notation: "standard" | "scientific" | "engineering" | "compact"): NumberFormatOptionsBuilder {
        this.notation = notation;
        return this;
    }

    public withSignDisplay(signDisplay: "auto" | "never" | "always" | "exceptZero"): NumberFormatOptionsBuilder {
        this.signDisplay = signDisplay;
        return this;
    }

    public withStyle(style: "decimal" | "percent" | "currency" | "unit"): NumberFormatOptionsBuilder {
        this.style = style;
        if (this.style == "percent") {
            this.maximumFractionDigits = 0
            this.minimumFractionDigits = 0
            this.useGrouping = true
        } else  if (this.style == "currency") {
            if (this.currencySign == undefined) {
                this.currencySign = "standard";
            }
            if (this.currencyDisplay == undefined) {
                this.currencyDisplay = "symbol";
            }
            this.minimumFractionDigits = 2;
            this.maximumFractionDigits = 2;
            this.useGrouping = true
        } else if (this.style == "unit") {
            if (this.unitDisplay == undefined) {
                this.unitDisplay = "short";
            }
        }
        return this;
    }

    public withUnit(unit: string): NumberFormatOptionsBuilder {
        this.unit = unit;
        return this;
    }

    public withUnitDisplay(unitDisplay: "short" | "long" | "narrow"): NumberFormatOptionsBuilder {
        this.unitDisplay = unitDisplay;
        return this;
    }

    public withUseGrouping(useGrouping: boolean): NumberFormatOptionsBuilder {
        this.useGrouping = useGrouping;
        return this;
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Intl. NumberFormatToParts");
    suite.addTest("testNumberFormatToPartsWithDifferentStyles", testNumberFormatToPartsWithDifferentStyles);
    suite.addTest("testNumberFormatToPartsWithDifferentNumbers", testNumberFormatToPartsWithDifferentNumbers);
    suite.addTest("testNumberFormatToPartsWithDifferentLocales", testNumberFormatToPartsWithDifferentLocales);
    suite.addTest("testNumberFormatToPartsWithDifferentOptions", testNumberFormatToPartsWithDifferentOptions);
    suite.addTest("testNumberFormatToPartsWithNotation", testNumberFormatToPartsWithNotation);
    suite.addTest("testNumberFormatToPartsWithSignDisplay", testNumberFormatToPartsWithSignDisplay);
    suite.addTest("testNumberFormatToPartsWithUnit", testNumberFormatToPartsWithUnit);
    suite.addTest("testNumberFormatToPartsWithGrouping", testNumberFormatToPartsWithGrouping);
    suite.addTest("testNumberFormatToPartsWithEdgeCases", testNumberFormatToPartsWithEdgeCases);
    suite.addTest("testNumberFormatToPartsWithBigInt", testNumberFormatToPartsWithBigInt);
    suite.addTest("testNumberFormatToPartsWithChineseLocale", testNumberFormatToPartsWithChineseLocale);

    // Range format tests
    suite.addTest("testNumberFormatRangeToPartsBasic", testNumberFormatRangeToPartsBasic);
    suite.addTest("testNumberFormatRangeToPartsWithDifferentStyles", testNumberFormatRangeToPartsWithDifferentStyles);
    suite.addTest("testNumberFormatRangeToPartsWithDifferentLocales", testNumberFormatRangeToPartsWithDifferentLocales);
    suite.addTest("testNumberFormatRangeToPartsWithOptions", testNumberFormatRangeToPartsWithOptions);
    suite.addTest("testNumberFormatRangeToPartsWithBigInt", testNumberFormatRangeToPartsWithBigInt);
    suite.addTest("testNumberFormatRangeToPartsWithMixedTypes", testNumberFormatRangeToPartsWithMixedTypes);
    suite.addTest("testNumberFormatRangeToPartsWithEdgeCases", testNumberFormatRangeToPartsWithEdgeCases);
    suite.addTest("testNumberFormatRangeToPartsWithChineseLocale", testNumberFormatRangeToPartsWithChineseLocale);

    return suite.run();
}

function assertPartEq(part: Intl.NumberFormatPart, eType: string, eValue: string) {
    arktest.assertEQ(part.type, eType, `Error! part.type is not equal to ${eType}, got ${part.type}`);
    arktest.assertEQ(part.value, eValue, `Error! part.value is not equal to ${eValue}, got ${part.value}`);
}
function assertRangePartEq(part: Intl.NumberRangeFormatPart, eType: string, eValue: string, eSource: string) {
    arktest.assertEQ(part.type, eType, `Error! part.type is not equal to ${eType}, got ${part.type}`);
    arktest.assertEQ(part.value, eValue, `Error! part.value is not equal to ${eValue}, got ${part.value}`);
    arktest.assertEQ(part.source, eSource, `Error! part.source is not equal to ${eSource}, got ${part.source}`);
}

function testNumberFormatToPartsWithDifferentStyles() {
    const numberFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withStyle("currency")
            .withCurrency("USD")
            .build());
    const parts = numberFormat.formatToParts(12.34);
    assertPartEq(parts[0], "currency", "$");
    assertPartEq(parts[1], "integer", "12");
    assertPartEq(parts[2], "decimal", ".");
    assertPartEq(parts[3], "fraction", "34");
    arktest.assertEQ(parts.length, 4, "Expected 4 parts for currency style");

    const numberFormat2 = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withStyle("decimal")
            .build());
    const parts2 = numberFormat2.formatToParts(12.34);
    assertPartEq(parts2[0], "integer", "12");
    assertPartEq(parts2[1], "decimal", ".");
    assertPartEq(parts2[2], "fraction", "34");
    arktest.assertEQ(parts2.length, 3, "Expected 3 parts for decimal style");

    const numberFormat3 = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withStyle("percent")
            .build());
    const parts3 = numberFormat3.formatToParts(12.34);
    assertPartEq(parts3[0], "integer", "1");
    assertPartEq(parts3[1], "group", ",");
    assertPartEq(parts3[2], "integer", "234");
    assertPartEq(parts3[3], "percentSign", "%");
    arktest.assertEQ(parts3.length, 4, "Expected 4 parts for percent style");
}

function testNumberFormatToPartsWithDifferentNumbers() {
    // Test integer
    const intFormat = new Intl.NumberFormat("en-US");
    const intParts = intFormat.formatToParts(1234);
    assertPartEq(intParts[0], "integer", "1");
    assertPartEq(intParts[1], "group", ",");
    assertPartEq(intParts[2], "integer", "234");
    arktest.assertEQ(intParts.length, 3, "Expected 3 parts for integer");

    // Test negative number
    const negFormat = new Intl.NumberFormat("en-US");
    const negParts = negFormat.formatToParts(-5678.9);
    assertPartEq(negParts[0], "minusSign", "-");
    assertPartEq(negParts[1], "integer", "5");
    assertPartEq(negParts[2], "group", ",");
    assertPartEq(negParts[3], "integer", "678");
    assertPartEq(negParts[4], "decimal", ".");
    assertPartEq(negParts[5], "fraction", "9");
    arktest.assertEQ(negParts.length, 6, "Expected 6 parts for negative number");

    // Test zero
    const zeroParts = intFormat.formatToParts(0);
    assertPartEq(zeroParts[0], "integer", "0");
    arktest.assertEQ(zeroParts.length, 1, "Expected 1 part for zero");

    // Test large number
    const largeParts = intFormat.formatToParts(1000000);
    assertPartEq(largeParts[0], "integer", "1");
    assertPartEq(largeParts[1], "group", ",");
    assertPartEq(largeParts[2], "integer", "000");
    assertPartEq(largeParts[3], "group", ",");
    assertPartEq(largeParts[4], "integer", "000");
    arktest.assertEQ(largeParts.length, 5, "Expected 5 parts for large number");
}

function testNumberFormatToPartsWithDifferentLocales() {
    // US English
    const usFormat = new Intl.NumberFormat("en-US");
    const usParts = usFormat.formatToParts(1234.56);
    assertPartEq(usParts[0], "integer", "1");
    assertPartEq(usParts[1], "group", ",");
    assertPartEq(usParts[2], "integer", "234");
    assertPartEq(usParts[3], "decimal", ".");
    assertPartEq(usParts[4], "fraction", "56");
    arktest.assertEQ(usParts.length, 5, "Expected 5 parts for US English");

    // German (uses comma as decimal separator)
    const deFormat = new Intl.NumberFormat("de-DE");
    const deParts = deFormat.formatToParts(1234.56);
    assertPartEq(deParts[0], "integer", "1");
    assertPartEq(deParts[1], "group", ".");
    assertPartEq(deParts[2], "integer", "234");
    assertPartEq(deParts[3], "decimal", ",");
    assertPartEq(deParts[4], "fraction", "56");
    arktest.assertEQ(deParts.length, 5, "Expected 5 parts for German");
    // Arabic (uses different digits and RTL)
    const arFormat = new Intl.NumberFormat("ar-EG");
    const arParts = arFormat.formatToParts(1234.56);
    // Note: Actual values would be in Arabic digits, adjusting the test as needed
    arktest.assertEQ(arParts[0].type, "integer", "Expected type 'integer'");
    arktest.assertEQ(arParts[1].type, "group", "Expected type 'group'");
    arktest.assertEQ(arParts[2].type, "integer", "Expected type 'integer'");
    arktest.assertEQ(arParts[3].type, "decimal", "Expected type 'decimal'");
    arktest.assertEQ(arParts[4].type, "fraction", "Expected type 'fraction'");
    arktest.assertEQ(arParts.length, 5, "Expected 5 parts for Arabic");
}

function testNumberFormatToPartsWithDifferentOptions() {
    // Test with minimum and maximum fraction digits
    const fracFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withMinimumFractionDigits(2)
            .withMaximumFractionDigits(4)
            .build());

    // Test value with fewer than minimum fraction digits
    const fracParts1 = fracFormat.formatToParts(123.4);
    assertPartEq(fracParts1[0], "integer", "123");
    assertPartEq(fracParts1[1], "decimal", ".");
    assertPartEq(fracParts1[2], "fraction", "40"); // Padded to reach minimum
    // Test with minimum integer digits
    const intFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withMinimumIntegerDigits(5)
            .build());

    const intParts = intFormat.formatToParts(123);
    assertPartEq(intParts[0], "integer", "00");
    assertPartEq(intParts[1], "group", ",");
    assertPartEq(intParts[2], "integer", "123");

    // Test value with more than maximum fraction digits
    const fracParts2 = fracFormat.formatToParts(123.4567880);
    assertPartEq(fracParts2[0], "integer", "123");
    assertPartEq(fracParts2[1], "decimal", ".");
    assertPartEq(fracParts2[2], "fraction", "4568"); // Truncated to maximum
}

function testNumberFormatToPartsWithNotation() {
    // Test scientific notation
    const scientificFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withNotation("scientific")
            .build());

    const scientificParts = scientificFormat.formatToParts(12345);
    assertPartEq(scientificParts[0], "integer", "1");
    assertPartEq(scientificParts[1], "decimal", ".");
    assertPartEq(scientificParts[2], "fraction", "235");
    assertPartEq(scientificParts[3], "exponentSeparator", "E");
    assertPartEq(scientificParts[4], "exponentInteger", "4");

    // Test engineering notation
    const engineeringFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withNotation("engineering")
            .build());

    const engineeringParts = engineeringFormat.formatToParts(12345);
    assertPartEq(engineeringParts[0], "integer", "12");
    assertPartEq(engineeringParts[1], "decimal", ".");
    assertPartEq(engineeringParts[2], "fraction", "345");
    assertPartEq(engineeringParts[3], "exponentSeparator", "E");
    assertPartEq(engineeringParts[4], "exponentInteger", "3");

    // Test compact notation with default options
    let compactFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withNotation("compact")
            .withCompactDisplay("short")
            .build());

    let compactParts = compactFormat.formatToParts(1234567);
    assertPartEq(compactParts[0], "integer", "1");
    assertPartEq(compactParts[1], "decimal", ".");
    assertPartEq(compactParts[2], "fraction", "235");
    assertPartEq(compactParts[3], "compact", "M");
    arktest.assertEQ(compactParts.length, 4, "Expected t parts for compact");

    // Test compact notation with undefined options
    compactFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newClear()
            .withNotation("compact")
            .withCompactDisplay("short")
            .build());
    compactParts = compactFormat.formatToParts(1234567);
    assertPartEq(compactParts[0], "integer", "1");
    assertPartEq(compactParts[1], "decimal", ".");
    assertPartEq(compactParts[2], "fraction", "2");
    assertPartEq(compactParts[3], "compact", "M");
    arktest.assertEQ(compactParts.length, 4, "Expected 4 parts for compact");
}

function testNumberFormatToPartsWithSignDisplay() {
    // Test with "always" sign display
    const alwaysSignFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withSignDisplay("always")
            .build());

    // Positive number should have + sign
    const positiveParts = alwaysSignFormat.formatToParts(123);
    assertPartEq(positiveParts[0], "plusSign", "+");
    assertPartEq(positiveParts[1], "integer", "123");
    arktest.assertEQ(positiveParts.length, 2, "Expected 2 parts for positive number");

    // Negative number should have - sign
    const negativeParts = alwaysSignFormat.formatToParts(-123);
    assertPartEq(negativeParts[0], "minusSign", "-");
    assertPartEq(negativeParts[1], "integer", "123");
    arktest.assertEQ(negativeParts.length, 2, "Expected 2 parts for negative number");

    // Test with "never" sign display
    const neverSignFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withSignDisplay("never")
            .build());

    // Negative number should not have sign
    const negParts = neverSignFormat.formatToParts(-123);
    assertPartEq(negParts[0], "integer", "123");
    arktest.assertEQ(negParts.length, 1, "Expected 1 part for negative number");

    // Test with "exceptZero" sign display
    const exceptZeroFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newClear()
            .withSignDisplay("exceptZero")
            .build());

    const zeroParts = exceptZeroFormat.formatToParts(0);
    assertPartEq(zeroParts[0], "integer", "0");
    arktest.assertEQ(zeroParts.length, 1, "Expected 1 part for zeroExcept");
}

function testNumberFormatToPartsWithUnit() {
    // Test with unit "percent" which is always supported
    const percentFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withStyle("percent")
            .build());

    const percentParts = percentFormat.formatToParts(0.5);
    assertPartEq(percentParts[0], "integer", "50");
    assertPartEq(percentParts[1], "percentSign", "%");
    arktest.assertEQ(percentParts.length, 2, "Expected 2 parts for percent");

    // Test with currency instead of unit
    const currencyFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withStyle("currency")
            .withCurrency("USD")
            .build());

    const currencyParts = currencyFormat.formatToParts(100);
    assertPartEq(currencyParts[0], "currency", "$");
    assertPartEq(currencyParts[1], "integer", "100");
    assertPartEq(currencyParts[2], "decimal", ".");
    assertPartEq(currencyParts[3], "fraction", "00");
    arktest.assertEQ(currencyParts.length, 4, "Expected 4 parts for currency");
}

function testNumberFormatToPartsWithGrouping() {
    // Test with grouping enabled
    const groupFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withUseGrouping(true)
            .build());

    const groupParts = groupFormat.formatToParts(1234567.89);
    // Should have group separators
    let hasGroupSeparator = false;
    for (const part of groupParts) {
        if (part.type == "group") {
            hasGroupSeparator = true;
            break;
        }
    }
    arktest.assertEQ(hasGroupSeparator, true, "Expected group separator in format");

    // Test with grouping disabled
    const noGroupFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withUseGrouping(false)
            .build());

    const noGroupParts = noGroupFormat.formatToParts(1234567.89);
    // Should not have group separators
    hasGroupSeparator = false;
    for (const part of noGroupParts) {
        if (part.type == "group") {
            hasGroupSeparator = true;
            break;
        }
    }
    arktest.assertEQ(hasGroupSeparator, false, "Expected no group separator in format");
}

function testNumberFormatToPartsWithEdgeCases() {
    const format = new Intl.NumberFormat("en-US");

    // Test with Infinity
    const infParts = format.formatToParts(Infinity);
    assertPartEq(infParts[0], "integer", "∞");
    arktest.assertEQ(infParts.length, 1, "Expected 1 part for infinity");

    // Test with negative Infinity
    const negInfParts = format.formatToParts(-Infinity);
    assertPartEq(negInfParts[0], "minusSign", "-");
    assertPartEq(negInfParts[1], "integer", "∞");
    arktest.assertEQ(negInfParts.length, 2, "Expected 2 parts for infinity");

    // Test with very small and very large numbers
    const smallParts = format.formatToParts(0.0000001);
    assertPartEq(smallParts[0], "integer", "0");
    arktest.assertEQ(smallParts.length, 1, "Expected 1 parts for small number");

    const largeParts = format.formatToParts(1e10);
    assertPartEq(largeParts[0], "integer", "10");
    assertPartEq(largeParts[1], "group", ",");
    assertPartEq(largeParts[2], "integer", "000");
    assertPartEq(largeParts[3], "group", ",");
    assertPartEq(largeParts[4], "integer", "000");
    assertPartEq(largeParts[5], "group", ",");
    assertPartEq(largeParts[6], "integer", "000");
    arktest.assertEQ(largeParts.length, 7, "Expected 7 parts for large number");
}

function testNumberFormatToPartsWithBigInt() {
    const format = new Intl.NumberFormat("en-US");

    // Test with small BigInt
    const smallBigInt = 123n;
    const smallParts = format.formatToParts(smallBigInt);

    assertPartEq(smallParts[0], "integer", "123");
    arktest.assertEQ(smallParts.length, 1, "Expected 1 part for small BigInt");

    // Test with large BigInt
    const largeBigInt = 9007199254740991n; // 2^53 - 1
    const largeParts = format.formatToParts(largeBigInt);
    assertPartEq(largeParts[0], "integer", "9");
    assertPartEq(largeParts[1], "group", ",");
    assertPartEq(largeParts[2], "integer", "007");
    assertPartEq(largeParts[3], "group", ",");
    assertPartEq(largeParts[4], "integer", "199");
    assertPartEq(largeParts[5], "group", ",");
    assertPartEq(largeParts[6], "integer", "254");
    assertPartEq(largeParts[7], "group", ",");
    assertPartEq(largeParts[8], "integer", "740");
    assertPartEq(largeParts[9], "group", ",");
    assertPartEq(largeParts[10], "integer", "991");
    arktest.assertEQ(largeParts.length, 11, "Expected 11 parts for large BigInt");

    // Test with negative BigInt
    const negativeBigInt = -42n;
    const negativeParts = format.formatToParts(negativeBigInt);
    assertPartEq(negativeParts[0], "minusSign", "-");
    assertPartEq(negativeParts[1], "integer", "42");
    arktest.assertEQ(negativeParts.length, 2, "Expected 2 parts for negative BigInt");

    // Test with zero BigInt
    const zeroBigInt = 0n;
    const zeroParts = format.formatToParts(zeroBigInt);
    assertPartEq(zeroParts[0], "integer", "0");
    arktest.assertEQ(zeroParts.length, 1, "Expected 1 part for zero BigInt");
}

function testNumberFormatRangeToPartsBasic() {
    const format = new Intl.NumberFormat("en-US");

    // Test basic range formatting
    const parts = format.formatRangeToParts(1, 5);

    assertRangePartEq(parts[0], "integer", "1", "startRange");
    assertRangePartEq(parts[1], "literal", "–", "shared");
    assertRangePartEq(parts[2], "integer", "5", "endRange");
}

function testNumberFormatRangeToPartsWithDifferentStyles() {
    // Test with currency style
    const currencyFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withStyle("currency")
            .withCurrency("USD")
            .build());

    const currencyParts = currencyFormat.formatRangeToParts(10, 20);
    assertRangePartEq(currencyParts[0], "currency", "$", "startRange");
    assertRangePartEq(currencyParts[1], "integer", "10", "startRange");
    assertRangePartEq(currencyParts[2], "decimal", ".", "startRange");
    assertRangePartEq(currencyParts[3], "fraction", "00", "startRange");
    assertRangePartEq(currencyParts[4], "literal", " – ", "shared");
    assertRangePartEq(currencyParts[5], "currency", "$", "endRange");
    assertRangePartEq(currencyParts[6], "integer", "20", "endRange");
    assertRangePartEq(currencyParts[7], "decimal", ".", "endRange");
    assertRangePartEq(currencyParts[8], "fraction", "00", "endRange");

    // Test with percent style
    const percentFormat = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withStyle("percent")
            .build());

    const percentParts = percentFormat.formatRangeToParts(0.1, 0.2);
    assertRangePartEq(percentParts[0], "integer", "10", "startRange");
    assertRangePartEq(percentParts[1], "percentSign", "%", "startRange");
    assertRangePartEq(percentParts[2], "literal", " – ", "shared");
    assertRangePartEq(percentParts[3], "integer", "20", "endRange");
    assertRangePartEq(percentParts[4], "percentSign", "%", "endRange");
}

function testNumberFormatRangeToPartsWithDifferentLocales() {
    // Test with English locale
    const enFormat = new Intl.NumberFormat("en-US");
    const enParts = enFormat.formatRangeToParts(1.5, 2.5);

    assertRangePartEq(enParts[0], "integer", "1", "startRange");
    assertRangePartEq(enParts[1], "decimal", ".", "startRange");
    assertRangePartEq(enParts[2], "fraction", "5", "startRange");
    assertRangePartEq(enParts[3], "literal", "–", "shared");
    assertRangePartEq(enParts[4], "integer", "2", "endRange");
    assertRangePartEq(enParts[5], "decimal", ".", "endRange");
    assertRangePartEq(enParts[6], "fraction", "5", "endRange");

    // Test with German locale
    const deFormat = new Intl.NumberFormat("de-DE");
    const deParts = deFormat.formatRangeToParts(1.5, 2.5);

    assertRangePartEq(deParts[0], "integer", "1", "startRange");
    assertRangePartEq(deParts[1], "decimal", ",", "startRange");
    assertRangePartEq(deParts[2], "fraction", "5", "startRange");
    assertRangePartEq(deParts[3], "literal", "–", "shared");
    assertRangePartEq(deParts[4], "integer", "2", "endRange");
    assertRangePartEq(deParts[5], "decimal", ",", "endRange");
    assertRangePartEq(deParts[6], "fraction", "5", "endRange");
}

function testNumberFormatRangeToPartsWithOptions() {
    /// FIXME (templin.konstantin): Fix this case after NumberFormat options fixes (internal issue: #24833)
    const format = new Intl.NumberFormat(
        "en-US",
        NumberFormatOptionsBuilder
            .newDefault()
            .withUseGrouping(true)
            .withMinimumFractionDigits(2)
            .withMaximumFractionDigits(2)
            .build());

    const parts = format.formatRangeToParts(1234.5, 5678.9);
    assertRangePartEq(parts[0], "integer", "1", "startRange");
    assertRangePartEq(parts[1], "group", ",", "startRange");
    assertRangePartEq(parts[2], "integer", "234", "startRange");
    assertRangePartEq(parts[3], "decimal", ".", "startRange");
    assertRangePartEq(parts[4], "fraction", "50", "startRange");
    assertRangePartEq(parts[5], "literal", "–", "shared");
    assertRangePartEq(parts[6], "integer", "5", "endRange");
    assertRangePartEq(parts[7], "group", ",", "endRange");
    assertRangePartEq(parts[8], "integer", "678", "endRange");
    assertRangePartEq(parts[9], "decimal", ".", "endRange");
    assertRangePartEq(parts[10], "fraction", "90", "endRange");
}

function testNumberFormatRangeToPartsWithBigInt() {
    const format = new Intl.NumberFormat("en-US");

    // Test with BigInt values
    const parts = format.formatRangeToParts(10n, 20n);
    assertRangePartEq(parts[0], "integer", "10", "startRange");
    assertRangePartEq(parts[1], "literal", "–", "shared");
    assertRangePartEq(parts[2], "integer", "20", "endRange");

    // Test with large BigInt values
    const largeParts = format.formatRangeToParts(9007199254740991n, 9007199254740992n);
    assertRangePartEq(largeParts[0], "integer", "9", "startRange");
    assertRangePartEq(largeParts[1], "group", ",", "startRange");
    assertRangePartEq(largeParts[2], "integer", "007", "startRange");
    assertRangePartEq(largeParts[3], "group", ",", "startRange");
    assertRangePartEq(largeParts[4], "integer", "199", "startRange");
    assertRangePartEq(largeParts[5], "group", ",", "startRange");
    assertRangePartEq(largeParts[6], "integer", "254", "startRange");
    assertRangePartEq(largeParts[7], "group", ",", "startRange");
    assertRangePartEq(largeParts[8], "integer", "740", "startRange");
    assertRangePartEq(largeParts[9], "group", ",", "startRange");
    assertRangePartEq(largeParts[10], "integer", "991", "startRange");
    assertRangePartEq(largeParts[11], "literal", "–", "shared");
    assertRangePartEq(largeParts[12], "integer", "9", "endRange");
    assertRangePartEq(largeParts[13], "group", ",", "endRange");
    assertRangePartEq(largeParts[14], "integer", "007", "endRange");
    assertRangePartEq(largeParts[15], "group", ",", "endRange");
    assertRangePartEq(largeParts[16], "integer", "199", "endRange");
    assertRangePartEq(largeParts[17], "group", ",", "endRange");
    assertRangePartEq(largeParts[18], "integer", "254", "endRange");
    assertRangePartEq(largeParts[19], "group", ",", "endRange");
    assertRangePartEq(largeParts[20], "integer", "740", "endRange");
    assertRangePartEq(largeParts[21], "group", ",", "endRange");
    assertRangePartEq(largeParts[22], "integer", "992", "endRange");
}

function testNumberFormatRangeToPartsWithMixedTypes() {
    const format = new Intl.NumberFormat("en-US");

    // Test with number and BigInt
    const parts1 = format.formatRangeToParts(10, 20n);
    assertRangePartEq(parts1[0], "integer", "10", "startRange");
    assertRangePartEq(parts1[1], "literal", "–", "shared");
    assertRangePartEq(parts1[2], "integer", "20", "endRange");

    // Test with BigInt and number
    const parts2 = format.formatRangeToParts(30n, 40);
    assertRangePartEq(parts2[0], "integer", "30", "startRange");
    assertRangePartEq(parts2[1], "literal", "–", "shared");
    assertRangePartEq(parts2[2], "integer", "40", "endRange");
}

function testNumberFormatRangeToPartsWithEdgeCases() {
    const format = new Intl.NumberFormat("en-US");

    const negParts = format.formatRangeToParts(-10, -5);
    assertRangePartEq(negParts[0], "minusSign", "-", "startRange");
    assertRangePartEq(negParts[1], "integer", "10", "startRange");
    assertRangePartEq(negParts[2], "literal", " – ", "shared");
    assertRangePartEq(negParts[3], "minusSign", "-", "endRange");
    assertRangePartEq(negParts[4], "integer", "5", "endRange");

    // Test with mixed sign range
    const mixedSignParts = format.formatRangeToParts(-10, 5);
    assertRangePartEq(mixedSignParts[0], "minusSign", "-", "startRange");
    assertRangePartEq(mixedSignParts[1], "integer", "10", "startRange");
    assertRangePartEq(mixedSignParts[2], "literal", " – ", "shared");
    assertRangePartEq(mixedSignParts[3], "integer", "5", "endRange");

    // Test with same value for start and end
    const sameParts = format.formatRangeToParts(10, 10);
    assertRangePartEq(sameParts[0], "approximatelySign", "~", "shared");
    assertRangePartEq(sameParts[1], "integer", "10", "shared");

    // Test with zero
    const zeroParts = format.formatRangeToParts(0, 1);
    assertRangePartEq(zeroParts[0], "integer", "0", "startRange");
    assertRangePartEq(zeroParts[1], "literal", "–", "shared");
    assertRangePartEq(zeroParts[2], "integer", "1", "endRange");
}

function testNumberFormatToPartsWithChineseLocale() {
     // Test basic number formatting with Chinese locale (zh-CN)
    const cnFormat = new Intl.NumberFormat("zh-CN");
    const cnParts = cnFormat.formatToParts(12345.67);
    assertPartEq(cnParts[0], "integer", "12");
    assertPartEq(cnParts[1], "group", ",");
    assertPartEq(cnParts[2], "integer", "345");
    assertPartEq(cnParts[3], "decimal", ".");
    assertPartEq(cnParts[4], "fraction", "67");

    // Test with currency in Chinese Yuan (CNY)
    const cnCurrencyFormat = new Intl.NumberFormat(
        "zh-CN",
        {
            style: "currency",
            currency: "CNY",
            useGrouping: true
        }
    );

    const cnCurrencyParts = cnCurrencyFormat.formatToParts(12345.67);
    assertPartEq(cnCurrencyParts[0], "currency", "¥");
    assertPartEq(cnCurrencyParts[1], "integer", "12");
    assertPartEq(cnCurrencyParts[2], "group", ",");
    assertPartEq(cnCurrencyParts[3], "integer", "345");
    assertPartEq(cnCurrencyParts[4], "decimal", ".");
    assertPartEq(cnCurrencyParts[5], "fraction", "67");

    // Test with Hong Kong locale (zh-HK) and Hong Kong Dollar (HKD)
    const hkFormat = new Intl.NumberFormat(
        "zh-HK",
        {
            style: "currency",
            currency: "HKD",
            useGrouping: true
        }
    );

    const hkParts = hkFormat.formatToParts(12345.67);
    assertPartEq(hkParts[0], "currency", "HK$");
    assertPartEq(hkParts[1], "integer", "12");
    assertPartEq(hkParts[2], "group", ",");
    assertPartEq(hkParts[3], "integer", "345");
    assertPartEq(hkParts[4], "decimal", ".");
    assertPartEq(hkParts[5], "fraction", "67");

    const twFormat = new Intl.NumberFormat(
        "zh-TW",
        {
            style: "currency",
            currency: "TWD",
            useGrouping: true
        }
    );

    const twParts = twFormat.formatToParts(12345.67);
    assertPartEq(twParts[0], "currency", "$");
    assertPartEq(twParts[1], "integer", "12");
    assertPartEq(twParts[2], "group", ",");
    assertPartEq(twParts[3], "integer", "345");
    assertPartEq(twParts[4], "decimal", ".");
    assertPartEq(twParts[5], "fraction", "67");

    // Test with percent style in Chinese
    const cnPercentFormat = new Intl.NumberFormat(
        "zh-CN",
        {
            style: "percent",
        }
    );
    const cnPercentParts = cnPercentFormat.formatToParts(0.3456);
    assertPartEq(cnPercentParts[0], "integer", "35");
    assertPartEq(cnPercentParts[1], "percentSign", "%");

    // Test with large numbers in Chinese which might use special grouping
    const cnLargeFormat = new Intl.NumberFormat("zh-CN",
        {
            useGrouping: true
        }
    );
    const cnLargeParts = cnLargeFormat.formatToParts(1234567890);
    assertPartEq(cnLargeParts[0], "integer", "1");
    assertPartEq(cnLargeParts[1], "group", ",");
    assertPartEq(cnLargeParts[2], "integer", "234");
    assertPartEq(cnLargeParts[3], "group", ",");
    assertPartEq(cnLargeParts[4], "integer", "567");
    assertPartEq(cnLargeParts[5], "group", ",");
    assertPartEq(cnLargeParts[6], "integer", "890");
}

function testNumberFormatRangeToPartsWithChineseLocale() {
    // Test basic range formatting with Chinese locale (zh-CN)
    const cnRangeFormat = new Intl.NumberFormat("zh-CN");
    const cnRangeParts = cnRangeFormat.formatRangeToParts(1000, 2000);
    assertRangePartEq(cnRangeParts[0], "integer", "1", "startRange");
    assertRangePartEq(cnRangeParts[1], "group", ",", "startRange");
    assertRangePartEq(cnRangeParts[2], "integer", "000", "startRange");
    assertRangePartEq(cnRangeParts[3], "literal", "-", "shared");
    assertRangePartEq(cnRangeParts[4], "integer", "2", "endRange");
    assertRangePartEq(cnRangeParts[5], "group", ",", "endRange");
    assertRangePartEq(cnRangeParts[6], "integer", "000", "endRange");

    // Test with negative and positive range in Chinese
    const cnMixedRangeParts = cnRangeFormat.formatRangeToParts(-100, 100);
    assertRangePartEq(cnMixedRangeParts[0], "minusSign", "-", "startRange");
    assertRangePartEq(cnMixedRangeParts[1], "integer", "100", "startRange");
    assertRangePartEq(cnMixedRangeParts[2], "literal", " - ", "shared");
    assertRangePartEq(cnMixedRangeParts[3], "integer", "100", "endRange");

    // Test range formatting with currency in Chinese Yuan (CNY)
    const cnCurrencyRangeFormat = new Intl.NumberFormat(
        "zh-CN",
        NumberFormatOptionsBuilder
            .newDefault()
            .withStyle("currency")
            .withCurrency("CNY")
            .withUseGrouping(true)
            .build());

    const cnCurrencyRangeParts = cnCurrencyRangeFormat.formatRangeToParts(1234.56, 7890.12);
    assertRangePartEq(cnCurrencyRangeParts[0], "currency", "¥", "startRange");
    assertRangePartEq(cnCurrencyRangeParts[1], "integer", "1", "startRange");
    assertRangePartEq(cnCurrencyRangeParts[2], "group", ",", "startRange");
    assertRangePartEq(cnCurrencyRangeParts[3], "integer", "234", "startRange");
    assertRangePartEq(cnCurrencyRangeParts[4], "decimal", ".", "startRange");
    assertRangePartEq(cnCurrencyRangeParts[5], "fraction", "56", "startRange");
    assertRangePartEq(cnCurrencyRangeParts[6], "literal", " - ", "shared");
    assertRangePartEq(cnCurrencyRangeParts[7], "currency", "¥", "endRange");
    assertRangePartEq(cnCurrencyRangeParts[8], "integer", "7", "endRange");
    assertRangePartEq(cnCurrencyRangeParts[9], "group", ",", "endRange");
    assertRangePartEq(cnCurrencyRangeParts[10], "integer", "890", "endRange");
    assertRangePartEq(cnCurrencyRangeParts[11], "decimal", ".", "endRange");
    assertRangePartEq(cnCurrencyRangeParts[12], "fraction", "12", "endRange");

    const cnPercentRangeFormat = new Intl.NumberFormat(
        "zh-CN",
        NumberFormatOptionsBuilder
            .newDefault()
            .withStyle("percent")
            .build());

    const cnPercentRangeParts = cnPercentRangeFormat.formatRangeToParts(0.25, 0.75);
    arktest.assertEQ(cnPercentRangeParts.length, 5, "Expected parts in Chinese percent range format");
    assertRangePartEq(cnPercentRangeParts[0], "integer", "25", "startRange");
    assertRangePartEq(cnPercentRangeParts[1], "percentSign", "%", "startRange");
    assertRangePartEq(cnPercentRangeParts[2], "literal", " - ", "shared");
    assertRangePartEq(cnPercentRangeParts[3], "integer", "75", "endRange");
    assertRangePartEq(cnPercentRangeParts[4], "percentSign", "%", "endRange");
}
