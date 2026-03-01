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

const TEST_DATE = new Date("2023-10-11T00:20:00Z")

function initFormatResults(): void {
    if (FORMAT_RESULTS.length > 0) {
        return
    }

    addFormatResult("zh-CN", {year: "2-digit"}, "23年")
    // NOTE(cheezzario): #24816 implement special handling for the following case
    // addFormatResult("zh-CN", {year: "2-digit", month: "numeric"}, "23/10")
    addFormatResult("zh-CN", {year: "2-digit", month: "numeric", day: "numeric"}, "23/10/11")
    addFormatResult("zh-CN", {year: "2-digit", month: "numeric", day: "2-digit"}, "23/10/11")
    // NOTE(cheezzario): #24816 implement special handling for the following case
    // addFormatResult("zh-CN", {year: "2-digit", month: "numeric", weekday: "long"}, "23/10 星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "numeric", weekday: "long", day: "numeric"}, "23年10月11日星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "numeric", weekday: "long", day: "2-digit"}, "23年10月11日星期三")
    // NOTE(cheezzario): #24816 implement special handling for the following case
    // addFormatResult("zh-CN", {year: "2-digit", month: "numeric", weekday: "short"}, "23/10 周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "numeric", weekday: "short", day: "numeric"}, "23/10/11周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "numeric", weekday: "short", day: "2-digit"}, "23/10/11周三")
    // NOTE(cheezzario): #24816 implement special handling for the following case
    // addFormatResult("zh-CN", {year: "2-digit", month: "numeric", weekday: "narrow"}, "23/10 三")
    addFormatResult("zh-CN", {year: "2-digit", month: "numeric", weekday: "narrow", day: "numeric"}, "23/10/11三")
    addFormatResult("zh-CN", {year: "2-digit", month: "numeric", weekday: "narrow", day: "2-digit"}, "23/10/11三")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit"}, "23年10月")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", day: "numeric"}, "23/10/11")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", day: "2-digit"}, "23/10/11")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", weekday: "long"}, "23年10月 星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", weekday: "long", day: "numeric"}, "23年10月11日星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", weekday: "long", day: "2-digit"}, "23年10月11日星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", weekday: "short"}, "23年10月 周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", weekday: "short", day: "numeric"}, "23/10/11周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", weekday: "short", day: "2-digit"}, "23/10/11周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", weekday: "narrow"}, "23年10月 三")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", weekday: "narrow", day: "numeric"}, "23/10/11三")
    addFormatResult("zh-CN", {year: "2-digit", month: "2-digit", weekday: "narrow", day: "2-digit"}, "23/10/11三")
    addFormatResult("zh-CN", {year: "2-digit", month: "long"}, "23年10月")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", day: "numeric"}, "23年10月11日")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", day: "2-digit"}, "23年10月11日")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", weekday: "long"}, "23年10月 星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", weekday: "long", day: "numeric"}, "23年10月11日星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", weekday: "long", day: "2-digit"}, "23年10月11日星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", weekday: "short"}, "23年10月 周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", weekday: "short", day: "numeric"}, "23年10月11日周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", weekday: "short", day: "2-digit"}, "23年10月11日周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", weekday: "narrow"}, "23年10月 三")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", weekday: "narrow", day: "numeric"}, "23年10月11日三")
    addFormatResult("zh-CN", {year: "2-digit", month: "long", weekday: "narrow", day: "2-digit"}, "23年10月11日三")
    addFormatResult("zh-CN", {year: "2-digit", month: "short"}, "23年10月")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", day: "numeric"}, "23年10月11日")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", day: "2-digit"}, "23年10月11日")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", weekday: "long"}, "23年10月 星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", weekday: "long", day: "numeric"}, "23年10月11日星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", weekday: "long", day: "2-digit"}, "23年10月11日星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", weekday: "short"}, "23年10月 周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", weekday: "short", day: "numeric"}, "23年10月11日周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", weekday: "short", day: "2-digit"}, "23年10月11日周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", weekday: "narrow"}, "23年10月 三")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", weekday: "narrow", day: "numeric"}, "23年10月11日三")
    addFormatResult("zh-CN", {year: "2-digit", month: "short", weekday: "narrow", day: "2-digit"}, "23年10月11日三")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow"}, "23年10月")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", day: "numeric"}, "23年10月11日")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", day: "2-digit"}, "23年10月11日")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", weekday: "long"}, "23年10月 星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", weekday: "long", day: "numeric"}, "23年10月11日星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", weekday: "long", day: "2-digit"}, "23年10月11日星期三")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", weekday: "short"}, "23年10月 周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", weekday: "short", day: "numeric"}, "23年10月11日周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", weekday: "short", day: "2-digit"}, "23年10月11日周三")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", weekday: "narrow"}, "23年10月 三")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", weekday: "narrow", day: "numeric"}, "23年10月11日三")
    addFormatResult("zh-CN", {year: "2-digit", month: "narrow", weekday: "narrow", day: "2-digit"}, "23年10月11日三")
}

function testDateFormatUsingOptionsYear2Digit(): void {
	FORMAT_RESULTS.forEach((res: FormatResult) => {
		const locale = res[0]
		const options = res[1]

		const formatter = new Intl.DateTimeFormat(locale, options)
		const actualResult = formatter.format(TEST_DATE)

		const expectedResult = res[2]
        if (actualResult !== expectedResult) {
            arktest.assertEQ(actualResult, expectedResult, `Unexpected date formatting result for locale: ${locale} and options: ${options}`)
        }
	})
}

function main(): int {
	initFormatResults()

	const suite = new arktest.ArkTestsuite("Intl.DateTimeFormat(zh-CN)")
	suite.addTest("format() using year format '2-digit'", testDateFormatUsingOptionsYear2Digit)
	return suite.run()
}
