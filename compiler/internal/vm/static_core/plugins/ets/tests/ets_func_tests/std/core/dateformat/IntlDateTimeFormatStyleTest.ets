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

function initFormatResults(): void {
    if (FORMAT_RESULTS.length > 0) {
        return
    }

    // en-US locale
    addFormatResult("en-US", {dateStyle: "full"}, "Wednesday, October 11, 2023")
    addFormatResult("en-US", {dateStyle: "full", timeStyle: "full", timeZone: "UTC"}, "Wednesday, October 11, 2023 at 12:20:00 AM Coordinated Universal Time")
    addFormatResult("en-US", {dateStyle: "full", timeStyle: "long", timeZone: "UTC"}, "Wednesday, October 11, 2023 at 12:20:00 AM UTC")
    addFormatResult("en-US", {dateStyle: "full", timeStyle: "medium", timeZone: "UTC"}, "Wednesday, October 11, 2023 at 12:20:00 AM")
    addFormatResult("en-US", {dateStyle: "full", timeStyle: "short", timeZone: "UTC"}, "Wednesday, October 11, 2023 at 12:20 AM")
    addFormatResult("en-US", {dateStyle: "long"}, "October 11, 2023")
    addFormatResult("en-US", {dateStyle: "long", timeStyle: "full", timeZone: "UTC"}, "October 11, 2023 at 12:20:00 AM Coordinated Universal Time")
    addFormatResult("en-US", {dateStyle: "long", timeStyle: "long", timeZone: "UTC"}, "October 11, 2023 at 12:20:00 AM UTC")
    addFormatResult("en-US", {dateStyle: "long", timeStyle: "medium", timeZone: "UTC"}, "October 11, 2023 at 12:20:00 AM")
    addFormatResult("en-US", {dateStyle: "long", timeStyle: "short", timeZone: "UTC"}, "October 11, 2023 at 12:20 AM")
    addFormatResult("en-US", {dateStyle: "medium"}, "Oct 11, 2023")
    addFormatResult("en-US", {dateStyle: "medium", timeStyle: "full", timeZone: "UTC"}, "Oct 11, 2023, 12:20:00 AM Coordinated Universal Time")
    addFormatResult("en-US", {dateStyle: "medium", timeStyle: "long", timeZone: "UTC"}, "Oct 11, 2023, 12:20:00 AM UTC")
    addFormatResult("en-US", {dateStyle: "medium", timeStyle: "medium", timeZone: "UTC"}, "Oct 11, 2023, 12:20:00 AM")
    addFormatResult("en-US", {dateStyle: "medium", timeStyle: "short", timeZone: "UTC"}, "Oct 11, 2023, 12:20 AM")
    addFormatResult("en-US", {dateStyle: "short"}, "10/11/23")
    addFormatResult("en-US", {dateStyle: "short", timeStyle: "full", timeZone: "UTC"}, "10/11/23, 12:20:00 AM Coordinated Universal Time")
    addFormatResult("en-US", {dateStyle: "short", timeStyle: "long", timeZone: "UTC"}, "10/11/23, 12:20:00 AM UTC")
    addFormatResult("en-US", {dateStyle: "short", timeStyle: "medium", timeZone: "UTC"}, "10/11/23, 12:20:00 AM")
    addFormatResult("en-US", {dateStyle: "short", timeStyle: "short", timeZone: "UTC"}, "10/11/23, 12:20 AM")

    // en-US locale + chinese calendar
    addFormatResult("en-US", {dateStyle: "long", calendar: "chinese"}, "Eighth Month 27, 2023(gui-mao)")

    // zh-CN locale
    addFormatResult("zh-CN", {dateStyle: "full"}, "2023年10月11日星期三")
    addFormatResult("zh-CN", {dateStyle: "full", timeStyle: "full", timeZone: "UTC"}, "2023年10月11日星期三 协调世界时 00:20:00")
    addFormatResult("zh-CN", {dateStyle: "full", timeStyle: "long", timeZone: "UTC"}, "2023年10月11日星期三 UTC 00:20:00")
    addFormatResult("zh-CN", {dateStyle: "full", timeStyle: "medium", timeZone: "UTC"}, "2023年10月11日星期三 00:20:00")
    addFormatResult("zh-CN", {dateStyle: "full", timeStyle: "short", timeZone: "UTC"}, "2023年10月11日星期三 00:20")
    addFormatResult("zh-CN", {dateStyle: "long"}, "2023年10月11日")
    addFormatResult("zh-CN", {dateStyle: "long", timeStyle: "full", timeZone: "UTC"}, "2023年10月11日 协调世界时 00:20:00")
    addFormatResult("zh-CN", {dateStyle: "long", timeStyle: "long", timeZone: "UTC"}, "2023年10月11日 UTC 00:20:00")
    addFormatResult("zh-CN", {dateStyle: "long", timeStyle: "medium", timeZone: "UTC"}, "2023年10月11日 00:20:00")
    addFormatResult("zh-CN", {dateStyle: "long", timeStyle: "short", timeZone: "UTC"}, "2023年10月11日 00:20")
    addFormatResult("zh-CN", {dateStyle: "medium"}, "2023年10月11日")
    addFormatResult("zh-CN", {dateStyle: "medium", timeStyle: "full", timeZone: "UTC"}, "2023年10月11日 协调世界时 00:20:00")
    addFormatResult("zh-CN", {dateStyle: "medium", timeStyle: "long", timeZone: "UTC"}, "2023年10月11日 UTC 00:20:00")
    addFormatResult("zh-CN", {dateStyle: "medium", timeStyle: "medium", timeZone: "UTC"}, "2023年10月11日 00:20:00")
    addFormatResult("zh-CN", {dateStyle: "medium", timeStyle: "short", timeZone: "UTC"}, "2023年10月11日 00:20")
    addFormatResult("zh-CN", {dateStyle: "short"}, "2023/10/11")
    addFormatResult("zh-CN", {dateStyle: "short", timeStyle: "full", timeZone: "UTC"}, "2023/10/11 协调世界时 00:20:00")
    addFormatResult("zh-CN", {dateStyle: "short", timeStyle: "long", timeZone: "UTC"}, "2023/10/11 UTC 00:20:00")
    addFormatResult("zh-CN", {dateStyle: "short", timeStyle: "medium", timeZone: "UTC"}, "2023/10/11 00:20:00")
    addFormatResult("zh-CN", {dateStyle: "short", timeStyle: "short", timeZone: "UTC"}, "2023/10/11 00:20")

    // zh-CN locale + chinese calendar
    addFormatResult("zh-CN", {dateStyle: "long", calendar: "chinese"}, "2023癸卯年八月廿七")
}

const TEST_DATE = new Date("2023-10-11T00:20:00Z")

function dateFormatWithFormattingStyle(): void {
    FORMAT_RESULTS.forEach((res: FormatResult) => {
        const locale = res[0]
        const options = res[1]
        const formatter = new Intl.DateTimeFormat(locale, options)
        
        const expectedResult = res[2]
        const actualResult = formatter.format(TEST_DATE)

        if (actualResult !== expectedResult) {
            arktest.assertEQ(actualResult, expectedResult, `unexpected date format result for locale: ${locale}, options: ${options}`)
        }
    })
}

function main(): int {
    initFormatResults()

    const suite = new arktest.ArkTestsuite("Intl.DateTimeFormat formatting style tests") 
    suite.addTest("date style", dateFormatWithFormattingStyle)
    return suite.run()
}

