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

function testNumberToLocaleString(): void {
    const langTag = "zh-CN"
    const options: Intl.NumberFormatOptions = { useGrouping: false }

    const locale = new Intl.Locale(langTag)
    const num = new Number(45678)

    arktest.assertEQ(num.toLocaleString(langTag, options), "45678")
    arktest.assertEQ(num.toLocaleString(locale, options), "45678")

    arktest.assertEQ(num.toLocaleString(langTag), "45,678")
    arktest.assertEQ(num.toLocaleString(locale), "45,678")
    arktest.assertEQ(num.toLocaleString(), num.toLocaleString(undefined))
}

function testNaNToLocaleString(): void {
    const nan: Number = Number.NaN
    const negNan: Number = -Number.NaN

    arktest.assertEQ(nan.toLocaleString("en-US"), "NaN")
    arktest.assertEQ(negNan.toLocaleString("en-US"), "NaN")

    arktest.assertEQ(nan.toLocaleString("ru-RU"), "не число")
    arktest.assertEQ(nan.toLocaleString("fa-IR"), "ناعدد")
}

function testCurrencyToLocaleString(): void {
    const num: Number = 123456.789;

    const usdResult = num.toLocaleString("en-US", { style: "currency", currency: "USD" });
    arktest.assertEQ(usdResult, "$123,456.79", "USD format");

    const jpyResult = num.toLocaleString("ja-JP", { style: "currency", currency: "JPY" });
    arktest.assertEQ(jpyResult, "￥123,457", "JPY format");

    const cnyResult = num.toLocaleString("zh-CN", { style: "currency", currency: "CNY" });
    arktest.assertEQ(cnyResult, "¥123,456.79", "CNY format");

    const gbpResult = num.toLocaleString("en-GB", { style: "currency", currency: "GBP" });
    arktest.assertEQ(gbpResult, "£123,456.79", "GBP format");

    const inrResult = num.toLocaleString("en-IN", { style: "currency", currency: "INR" });
    arktest.assertEQ(inrResult, "₹1,23,456.79", "INR format");
}
function testMinimumFractionDigits1(): void {
    const num: Number = 0.756;

    const result = num.toLocaleString(undefined, { style: "percent", minimumFractionDigits: 2 });
    arktest.assertEQ(result, "75.60%");
}

function testMinimumFractionDigits2(): void {
    const num: Number = -0.111;

    const result = num.toLocaleString(undefined, { style: "percent", minimumFractionDigits: 2 });
    arktest.assertEQ(result, "-11.10%");
}

function testMinimumFractionDigits3(): void {
    const num: Number = 1.0;

    const result = num.toLocaleString(undefined, { style: "percent", minimumFractionDigits: 2 });
    arktest.assertEQ(result, "100.00%");
}

function testMinimumFractionDigits4(): void {
    const num: Number = 0;

    const result = num.toLocaleString(undefined, { style: "percent", minimumFractionDigits: 2 });
    arktest.assertEQ(result, "0.00%");
}

function testMinimumFractionDigits5(): void {
    const num: Number = 0.55;

    const result = num.toLocaleString(undefined, { style: "percent", maximumFractionDigits: 3, minimumFractionDigits: 2 });
    arktest.assertEQ(result, "55.00%");
}

function testMinimumFractionDigits6(): void {
    const num: Number = 0.123456;

    const result = num.toLocaleString(undefined, { style: "percent", maximumFractionDigits: 5, minimumFractionDigits: 3 });
    arktest.assertEQ(result, "12.3456%");
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Number")
    suite.addTest("toLocaleString", testNumberToLocaleString)
    suite.addTest("NaN.toLocaleString", testNaNToLocaleString)
    suite.addTest("num.toLocaleString", testCurrencyToLocaleString)
    suite.addTest("num.toLocaleString", testMinimumFractionDigits1)
    suite.addTest("num.toLocaleString", testMinimumFractionDigits2)
    suite.addTest("num.toLocaleString", testMinimumFractionDigits3)
    suite.addTest("num.toLocaleString", testMinimumFractionDigits4)
    suite.addTest("num.toLocaleString", testMinimumFractionDigits5)
    suite.addTest("num.toLocaleString", testMinimumFractionDigits6)
    return suite.run()
}
