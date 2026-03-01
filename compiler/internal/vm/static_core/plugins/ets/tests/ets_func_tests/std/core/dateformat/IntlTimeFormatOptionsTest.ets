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

const FORMAT_RESULTS = new Array<[Date, string, Intl.DateTimeFormatOptions, string]>()
function addFormatResult(date: Date, locale: string, options: Intl.DateTimeFormatOptions, result: string): void {
    const formatResult: [Date, string, Intl.DateTimeFormatOptions, string] = [date, locale, options, result]
    FORMAT_RESULTS.push(formatResult)
}

function initFormatResults(): void {
    if (FORMAT_RESULTS.length > 0) {
        return
    }

    const testDate = new Date("2023-10-05T15:10:20.345Z")

    addFormatResult(testDate, "zh-CN", { hour: "2-digit", minute: "2-digit", second: "2-digit", fractionalSecondDigits: 3, timeZone: "UTC" }, "15:10:20.345")
    addFormatResult(testDate, "zh-CN", { hour: "2-digit", minute: "2-digit", second: "2-digit", fractionalSecondDigits: 2, timeZone: "UTC" }, "15:10:20.34")
    addFormatResult(testDate, "zh-CN", { hour: "2-digit", minute: "2-digit", second: "2-digit", fractionalSecondDigits: 1, timeZone: "UTC" }, "15:10:20.3")

    addFormatResult(testDate, "en-US", { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "Europe/Berlin", timeZoneName: "short" }, "05:10:20 PM GMT+2")
    addFormatResult(testDate, "en-US", { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "Europe/Berlin", timeZoneName: "long" }, "05:10:20 PM Central European Summer Time")
    addFormatResult(testDate, "en-US", { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "Europe/Berlin", timeZoneName: "shortGeneric" }, "05:10:20 PM Germany Time")
    addFormatResult(testDate, "en-US", { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "Europe/Berlin", timeZoneName: "shortOffset" }, "05:10:20 PM GMT+2")
    addFormatResult(testDate, "en-US", { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "Europe/Berlin", timeZoneName: "longGeneric" }, "05:10:20 PM Central European Time")
    addFormatResult(testDate, "en-US", { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "Europe/Berlin", timeZoneName: "longOffset" }, "05:10:20 PM GMT+02:00")

    addFormatResult(testDate, "zh-CN", { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "Europe/Berlin", timeZoneName: "short" }, "GMT+2 17:10:20")
    addFormatResult(testDate, "zh-CN", { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "Europe/Berlin", timeZoneName: "long" }, "中欧夏令时间 17:10:20")
    addFormatResult(testDate, "zh-CN", { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "Europe/Berlin", timeZoneName: "shortGeneric" }, "德国时间 17:10:20")
    addFormatResult(testDate, "zh-CN", { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "Europe/Berlin", timeZoneName: "shortOffset" }, "GMT+2 17:10:20")
    addFormatResult(testDate, "zh-CN", { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "Europe/Berlin", timeZoneName: "longGeneric" }, "中欧时间 17:10:20")
    addFormatResult(testDate, "zh-CN", { hour: "2-digit", minute: "2-digit", second: "2-digit", timeZone: "Europe/Berlin", timeZoneName: "longOffset" }, "GMT+02:00 17:10:20")
}

function testTimeFormatWithLocaleAndOptions(): void {
    FORMAT_RESULTS.forEach((res: [Date, string, Intl.DateTimeFormatOptions, string]) => {
        const locale = res[1]
        const options = res[2]
        const formatter = new Intl.DateTimeFormat(locale, options)

        const testDate = res[0]
        const actual = formatter.format(testDate)
        const expected = res[3]
        if (actual !== expected) {
            arktest.assertEQ(actual, expected, `unexpected format result for locale: ${locale} and options: ${options}`)
        }
    })
}

function testTimeFormatWithInvalidFractionalSecondDigitsOption(): void {
    const options: Intl.DateTimeFormatOptions = { hour: "2-digit", minute: "2-digit", second: "2-digit", fractionalSecondDigits: 10 }
    arktest.expectError(() => {
        const formatter = new Intl.DateTimeFormat("en-US", options)
    }, "invalid digits value: 10")
}

function main(): int {
    initFormatResults()

    const suite = new arktest.ArkTestsuite("Intl.DateTimeFormat API time format tests")
    suite.addTest("time format with custom locale and options", testTimeFormatWithLocaleAndOptions)
    suite.addTest("time format with invalid fractionalSecondDigits", testTimeFormatWithInvalidFractionalSecondDigitsOption)
    return suite.run()
}
