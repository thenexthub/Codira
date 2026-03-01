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

type FormatResult = [string | undefined, Intl.DateTimeFormatOptions | undefined, string]

const FORMAT_RESULTS = new Array<FormatResult>()

function addFormatResult(locale: string | undefined, options: Intl.DateTimeFormatOptions | undefined, result: string): void {
    const res: [string | undefined, Intl.DateTimeFormatOptions | undefined, string] = [locale, options, result]
    FORMAT_RESULTS.push(res)
}

let FORMAT_RESULTS_UNDEF_OPTS = new Array<FormatResult>()
let FORMAT_RESULTS_CUSTOM_OPTS = new Array<FormatResult>()

const TEST_DATE = new Date("2023-10-11T00:20:00Z")

function initFormatResults(): void {
    if (FORMAT_RESULTS.length > 0) {
        return
    }

    addFormatResult("en-US", undefined, "10/11/2023")
    addFormatResult("zh-CN", undefined, "2023/10/11")
    addFormatResult("ru-RU", undefined, "11.10.2023")
    
    addFormatResult("en-US", {year: "numeric", month: "numeric"}, "10/2023")
    addFormatResult("en-US", {year: "numeric", month: "numeric", day: "numeric"}, "10/11/2023")
    addFormatResult("en-US", {year: "numeric", month: "numeric", day: "2-digit"}, "10/11/2023")
    addFormatResult("en-US", {year: "numeric", month: "2-digit"}, "10/2023")
    addFormatResult("en-US", {year: "numeric", month: "2-digit", day: "numeric"}, "10/11/2023")
    addFormatResult("en-US", {year: "numeric", month: "2-digit", day: "2-digit"}, "10/11/2023")
    addFormatResult("en-US", {year: "numeric", month: "long"}, "October 2023")
    addFormatResult("en-US", {year: "numeric", month: "long", day: "numeric"}, "October 11, 2023")
    addFormatResult("en-US", {year: "numeric", month: "long", day: "2-digit"}, "October 11, 2023")
    addFormatResult("en-US", {year: "numeric", month: "short"}, "Oct 2023")
    addFormatResult("en-US", {year: "numeric", month: "short", day: "numeric"}, "Oct 11, 2023")
    addFormatResult("en-US", {year: "numeric", month: "short", day: "2-digit"}, "Oct 11, 2023")
    addFormatResult("en-US", {year: "numeric", month: "narrow"}, "O 2023")
    addFormatResult("en-US", {year: "numeric", month: "narrow", day: "numeric"}, "O 11, 2023")
    addFormatResult("en-US", {year: "numeric", month: "narrow", day: "2-digit"}, "O 11, 2023")
    addFormatResult("en-US", {year: "2-digit", month: "numeric"}, "10/23")
    addFormatResult("en-US", {year: "2-digit", month: "numeric", day: "numeric"}, "10/11/23")
    addFormatResult("en-US", {year: "2-digit", month: "numeric", day: "2-digit"}, "10/11/23")
    addFormatResult("en-US", {year: "2-digit", month: "2-digit"}, "10/23")
    addFormatResult("en-US", {year: "2-digit", month: "2-digit", day: "numeric"}, "10/11/23")
    addFormatResult("en-US", {year: "2-digit", month: "2-digit", day: "2-digit"}, "10/11/23")
    addFormatResult("en-US", {year: "2-digit", month: "long"}, "October 23")
    addFormatResult("en-US", {year: "2-digit", month: "long", day: "numeric"}, "October 11, 23")
    addFormatResult("en-US", {year: "2-digit", month: "long", day: "2-digit"}, "October 11, 23")
    addFormatResult("en-US", {year: "2-digit", month: "short"}, "Oct 23")
    addFormatResult("en-US", {year: "2-digit", month: "short", day: "numeric"}, "Oct 11, 23")
    addFormatResult("en-US", {year: "2-digit", month: "short", day: "2-digit"}, "Oct 11, 23")
    addFormatResult("en-US", {year: "2-digit", month: "narrow"}, "O 23")
    addFormatResult("en-US", {year: "2-digit", month: "narrow", day: "numeric"}, "O 11, 23")
    addFormatResult("en-US", {year: "2-digit", month: "narrow", day: "2-digit"}, "O 11, 23")
    // FIXIT addFormatResult("zh-CN", {year: "numeric", month: "numeric"}, "2023/10")
    addFormatResult("zh-CN", {year: "numeric", month: "numeric", day: "numeric"}, "2023/10/11")
    addFormatResult("zh-CN", {year: "numeric", month: "numeric", day: "2-digit"}, "2023/10/11")
    addFormatResult("zh-CN", {year: "numeric", month: "2-digit"}, "2023年10月")
    addFormatResult("zh-CN", {year: "numeric", month: "2-digit", day: "numeric"}, "2023/10/11")
    addFormatResult("zh-CN", {year: "numeric", month: "2-digit", day: "2-digit"}, "2023/10/11")
    addFormatResult("zh-CN", {year: "numeric", month: "long"}, "2023年10月")
    addFormatResult("zh-CN", {year: "numeric", month: "long", day: "numeric"}, "2023年10月11日")
    addFormatResult("zh-CN", {year: "numeric", month: "long", day: "2-digit"}, "2023年10月11日")
    addFormatResult("zh-CN", {year: "numeric", month: "short"}, "2023年10月")
    addFormatResult("zh-CN", {year: "numeric", month: "short", day: "numeric"}, "2023年10月11日")
    addFormatResult("zh-CN", {year: "numeric", month: "short", day: "2-digit"}, "2023年10月11日")
    addFormatResult("zh-CN", {year: "numeric", month: "narrow"}, "2023年10月")
    addFormatResult("zh-CN", {year: "numeric", month: "narrow", day: "numeric"}, "2023年10月11日")
    addFormatResult("zh-CN", {year: "numeric", month: "narrow", day: "2-digit"}, "2023年10月11日")
    // FIXIT addFormatResult("zh-CN", {year: "2-digit", month: "numeric"}, "23/10")
    addFormatResult("zh-CN", {year: "2-digit", month: "numeric", day: "numeric"}, "23/10/11")
    addFormatResult("zh-CN", {year: "2-digit", month: "numeric", day: "2-digit"}, "23/10/11")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit"}, "23年10月")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", day: "numeric"}, "23/10/11")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", day: "2-digit"}, "23/10/11")
    addFormatResult("zh-CN", {year: "2-digit", month: "long"}, "23年10月")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", day: "numeric"}, "23年10月11日")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", day: "2-digit"}, "23年10月11日")
    addFormatResult("zh-CN", {year: "2-digit", month: "short"}, "23年10月")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", day: "numeric"}, "23年10月11日")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", day: "2-digit"}, "23年10月11日")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow"}, "23年10月")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", day: "numeric"}, "23年10月11日")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", day: "2-digit"}, "23年10月11日")

    // DateTimeFormat implementation phase test cases
    addFormatResult("en-US", {month: 'numeric', day: 'numeric'}, "10/11")
    addFormatResult("zh-CN", {month: 'numeric', day: 'numeric'}, "10/11")
    addFormatResult("en-US", {month: 'long', day: 'numeric'}, "October 11")
    addFormatResult("zh-CN", {month: 'long', day: 'numeric'}, "10月11日")
    addFormatResult("en-US", {hour: '2-digit', minute: '2-digit', hour12: true, timeZone: "UTC"}, "12:20 AM")
    addFormatResult("zh-CN", {hour: '2-digit', minute: '2-digit', hour12: true, timeZone: "UTC"}, "上午12:20")
    addFormatResult("en-US", {hour: '2-digit', minute: '2-digit', timeZone: "UTC"}, "12:20 AM")
    addFormatResult("zh-CN", {hour: '2-digit', minute: '2-digit', timeZone: "UTC"}, "00:20")
    addFormatResult("en-US", {hour: '2-digit', minute: '2-digit', hour12: false, timeZone: "UTC"}, "00:20")
    addFormatResult("zh-CN", {hour: '2-digit', minute: '2-digit', hour12: false, timeZone: "UTC"}, "00:20")
    addFormatResult("en-US", {month: 'numeric', day: 'numeric', weekday: 'long'}, "Wednesday, 10/11")
    addFormatResult("zh-CN", {month: 'numeric', day: 'numeric', weekday: 'long'}, "10/11星期三")
    addFormatResult("en-US", {month: 'numeric', day: 'numeric', weekday: 'short'}, "Wed, 10/11")
    addFormatResult("zh-CN", {month: 'numeric', day: 'numeric', weekday: 'short'}, "10/11周三")
    addFormatResult("en-US", {hour: '2-digit', minute: '2-digit', hourCycle: "h24", timeZone: "UTC"}, "24:20")
    addFormatResult("zh-CN", {hour: '2-digit', minute: '2-digit', hourCycle: "h24", timeZone: "UTC"}, "24:20")
    addFormatResult("en-US", {hour: '2-digit', minute: '2-digit', hourCycle: "h12", timeZone: "UTC"}, "12:20 AM")
    addFormatResult("zh-CN", {hour: '2-digit', minute: '2-digit', hourCycle: "h12", timeZone: "UTC"}, "上午12:20")

    addFormatResult(
            "en-US", 
            {year: '2-digit', month: '2-digit', day: '2-digit', hour: '2-digit', minute: '2-digit',  hour12: true, timeZone: "UTC"}, 
            "10/11/23, 12:20 AM")

    addFormatResult(
            "zh-CN", 
            {year: '2-digit', month: '2-digit', day: '2-digit', hour: '2-digit', minute: '2-digit',  hour12: true, timeZone: "UTC"}, 
            "23/10/11 上午12:20")

    addFormatResult(
            "en-US", 
            {year: '2-digit', month: '2-digit', day: '2-digit', hour: '2-digit', minute: '2-digit',  hour12: false, timeZone: "UTC"}, 
            "10/11/23, 00:20")

    addFormatResult(
            "zh-CN", 
            {year: '2-digit', month: '2-digit', day: '2-digit', hour: '2-digit', minute: '2-digit',  hour12: false, timeZone: "UTC"}, 
            "23/10/11 00:20")

    FORMAT_RESULTS_UNDEF_OPTS = FORMAT_RESULTS.filter((res: FormatResult) => res[1] === undefined)
    FORMAT_RESULTS_CUSTOM_OPTS = FORMAT_RESULTS.filter((res: FormatResult) => res[1] !== undefined)
}

function dateFormatWithDefaultOptions(): void {
    FORMAT_RESULTS_UNDEF_OPTS.forEach((res: FormatResult) => {
        const locale = res[0]

        const undefOptsFmt = new Intl.DateTimeFormat(locale, undefined)
        const emptyOptsFmt = new Intl.DateTimeFormat(locale, {})

        const undefOptsFmtResult = undefOptsFmt.format(TEST_DATE)

        const expected = res[2]
        arktest.assertEQ(undefOptsFmtResult, expected, `unexpected date format result for locale: ${locale} and undefined format options`)

        const emptyOptsFmtResult = emptyOptsFmt.format(TEST_DATE)
        arktest.assertEQ(undefOptsFmtResult, emptyOptsFmtResult, `unexpected date format result for locale: ${locale} and empty format options`)
    })
}

function dateFormatwithCustomOptions(): void {
    FORMAT_RESULTS_CUSTOM_OPTS.forEach((res: FormatResult) => {
        const locale = res[0]
        const options = res[1]
        const formatter = new Intl.DateTimeFormat(locale, options)

        const expectedResult = res[2]
        const actualResult = formatter.format(TEST_DATE)

        if (actualResult !== expectedResult) {
            arktest.assertEQ(actualResult, expectedResult, `unexpected date format result for locale: ${locale} and options: ${options}`)
        }
    })
}

function main(): int {
    initFormatResults()

    const suite = new arktest.ArkTestsuite("Intl.DateTimeFormat date formatting tests") 
    suite.addTest("date format with default locale", dateFormatWithDefaultOptions)
    suite.addTest("date format with custom options", dateFormatwithCustomOptions)
    return suite.run()
}

