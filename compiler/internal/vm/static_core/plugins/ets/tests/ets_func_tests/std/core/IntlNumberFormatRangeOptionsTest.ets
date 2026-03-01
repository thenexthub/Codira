/**
 * Copyright (c);2025 Huawei Device Co., Ltd.
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

type FormatResult = [number, string | undefined, Intl.NumberFormatOptions | undefined, string]
const FORMAT_RESULTS = new Array<FormatResult>()

function addFormatResult(i: number, locale: string | undefined, options: Intl.NumberFormatOptions | undefined, result: string): void {
    const res: [number, string | undefined, Intl.NumberFormatOptions | undefined, string] = [i, locale, options, result]
    FORMAT_RESULTS.push(res)
}

const TEST_RANGE_BEGIN = -1234567.8901
const TEST_RANGE_END = 1234567.8901

function initFormatResults(): void {
    if (FORMAT_RESULTS.length > 0) {
        return
    }
    let i: int;
    addFormatResult(i = 0, undefined, undefined, FORMAT_EXPECTED[i])
    addFormatResult(i = 1, "zh", undefined, FORMAT_EXPECTED[i])
    addFormatResult(i = 2, "zh", { compactDisplay: "short" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 3, "zh", { compactDisplay: "long" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 4, "zh", { currencySign: "standard" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 5, "zh", { currencySign: "accounting" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 6, "zh", { currency: "CNY" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 7, "zh", { currency: "USD" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 8, "zh", { currencyDisplay: "code" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 9, "zh", { currencyDisplay: "symbol" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 10, "zh", { currencyDisplay: "narrowSymbol" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 11, "zh", { currencyDisplay: "name" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 12, "zh", { maximumFractionDigits: 1 }, FORMAT_EXPECTED[i])
    addFormatResult(i = 13, "zh", { minimumFractionDigits: 5 }, FORMAT_EXPECTED[i])
    addFormatResult(i = 14, "zh", { maximumSignificantDigits: 1 }, FORMAT_EXPECTED[i])
    addFormatResult(i = 15, "zh", { minimumSignificantDigits: 5 }, FORMAT_EXPECTED[i])
    addFormatResult(i = 16, "zh", { minimumIntegerDigits: 10 }, FORMAT_EXPECTED[i])
    addFormatResult(i = 17, "zh", { notation: "standard" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 18, "zh", { notation: "scientific" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 19, "zh", { notation: "engineering" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 20, "en", { notation: "compact", compactDisplay: "short" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 21, "en", { notation: "compact", compactDisplay: "long" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 22, "zh", { signDisplay: "auto" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 23, "zh", { signDisplay: "never" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 24, "zh", { signDisplay: "always" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 25, "zh", { signDisplay: "exceptZero" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 26, "zh", { style: "decimal"}, FORMAT_EXPECTED[i])
    addFormatResult(i = 27, "zh", { style: "percent"}, FORMAT_EXPECTED[i])
    addFormatResult(i = 28, "zh", { style: "currency", currency: "CNY"}, FORMAT_EXPECTED[i])
    addFormatResult(i = 29, "zh", { style: "unit", unit: "kilogram" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 30, "zh", { unitDisplay: "short" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 31, "zh", { unitDisplay: "long" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 32, "zh", { unitDisplay: "narrow" }, FORMAT_EXPECTED[i])
    addFormatResult(i = 33, "zh", { useGrouping: false }, FORMAT_EXPECTED[i])
    addFormatResult(i = 34, "zh", { useGrouping: true }, FORMAT_EXPECTED[i])
}

function testNumberFormatFormatRangeWithOptions(): void {
    FORMAT_RESULTS.forEach((res: FormatResult) => {
        const i = res[0]
        const locale = res[1]
        const options = res[2]
        const expected = res[3]

        const formatter = new Intl.NumberFormat(locale, options)
        const result = formatter.formatRange(TEST_RANGE_BEGIN, TEST_RANGE_END)

        if (result !== expected) {
            const str = JSON.stringify(formatter.resolvedOptions())
            arktest.assertEQ(result, expected, `unexpected format result in #${i} test-case for locale: ${locale} and options: ${str}`)
        }
    })
}

function main(): int {
    initFormatResults()
    const suite = new arktest.ArkTestsuite("Intl.NumberFormat formatRage NumberFormatOptions")
    suite.addTest("testNumberFormatFormatRangeWithOptions", testNumberFormatFormatRangeWithOptions)
    return suite.run()
}

const FORMAT_EXPECTED: string[] = [
/*0*/ "-1,234,567.89 – 1,234,567.89",
/*1*/ "-1,234,567.89 - 1,234,567.89",
/*2*/ "-1,234,567.89 - 1,234,567.89",
/*3*/ "-1,234,567.89 - 1,234,567.89",
/*4*/ "-1,234,567.89 - 1,234,567.89",
/*5*/ "-1,234,567.89 - 1,234,567.89",
/*6*/ "-1,234,567.89 - 1,234,567.89",
/*7*/ "-1,234,567.89 - 1,234,567.89",
/*8*/ "-1,234,567.89 - 1,234,567.89",
/*9*/ "-1,234,567.89 - 1,234,567.89",
/*10*/ "-1,234,567.89 - 1,234,567.89",
/*11*/ "-1,234,567.89 - 1,234,567.89",
/*12*/ "-1,234,567.9 - 1,234,567.9",
/*13*/ "-1,234,567.89010 - 1,234,567.89010",
/*14*/ "-1,000,000 - 1,000,000",
/*15*/ "-1,234,567.8901 - 1,234,567.8901",
/*16*/ "-0,001,234,567.89 - 0,001,234,567.89",
/*17*/ "-1,234,567.89 - 1,234,567.89",
/*18*/ "-1.235E6 - 1.235E6",
/*19*/ "-1.235E6 - 1.235E6",
/*20*/ "-1.2M – 1.2M",
/*21*/ "-1.2 million – 1.2 million",
/*22*/ "-1,234,567.89 - 1,234,567.89",
/*23*/ "1,234,567.89-1,234,567.89",
/*24*/ "-1,234,567.89 - +1,234,567.89",
/*25*/ "-1,234,567.89 - +1,234,567.89",
/*26*/ "-1,234,567.89 - 1,234,567.89",
/*27*/ "-123,456,789% - 123,456,789%",
/*28*/ "-¥1,234,567.89 - ¥1,234,567.89",
/*29*/ "-1,234,567.89 - 1,234,567.89 kg",
/*30*/ "-1,234,567.89 - 1,234,567.89",
/*31*/ "-1,234,567.89 - 1,234,567.89",
/*32*/ "-1,234,567.89 - 1,234,567.89",
/*33*/ "-1234567.89 - 1234567.89",
/*34*/ "-1,234,567.89 - 1,234,567.89"
]
