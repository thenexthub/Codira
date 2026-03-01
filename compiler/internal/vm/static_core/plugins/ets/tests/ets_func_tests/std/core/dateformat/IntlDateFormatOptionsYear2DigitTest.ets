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

	addFormatResult("en-US", {year: "2-digit"}, "23")
	addFormatResult("en-US", {year: "2-digit", month: "numeric"}, "10/23")
	addFormatResult("en-US", {year: "2-digit", month: "numeric", day: "numeric"}, "10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "numeric", day: "2-digit"}, "10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "numeric", weekday: "long"}, "10/23 Wednesday")
	addFormatResult("en-US", {year: "2-digit", month: "numeric", weekday: "long", day: "numeric"}, "Wednesday, 10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "numeric", weekday: "long", day: "2-digit"}, "Wednesday, 10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "numeric", weekday: "short"}, "10/23 Wed")
	addFormatResult("en-US", {year: "2-digit", month: "numeric", weekday: "short", day: "numeric"}, "Wed, 10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "numeric", weekday: "short", day: "2-digit"}, "Wed, 10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "numeric", weekday: "narrow"}, "10/23 W")
	addFormatResult("en-US", {year: "2-digit", month: "numeric", weekday: "narrow", day: "numeric"}, "W, 10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "numeric", weekday: "narrow", day: "2-digit"}, "W, 10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "2-digit"}, "10/23")
	addFormatResult("en-US", {year: "2-digit", month: "2-digit", day: "numeric"}, "10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "2-digit", day: "2-digit"}, "10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "2-digit", weekday: "long"}, "10/23 Wednesday")
	addFormatResult("en-US", {year: "2-digit", month: "2-digit", weekday: "long", day: "numeric"}, "Wednesday, 10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "2-digit", weekday: "long", day: "2-digit"}, "Wednesday, 10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "2-digit", weekday: "short"}, "10/23 Wed")
	addFormatResult("en-US", {year: "2-digit", month: "2-digit", weekday: "short", day: "numeric"}, "Wed, 10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "2-digit", weekday: "short", day: "2-digit"}, "Wed, 10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "2-digit", weekday: "narrow"}, "10/23 W")
	addFormatResult("en-US", {year: "2-digit", month: "2-digit", weekday: "narrow", day: "numeric"}, "W, 10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "2-digit", weekday: "narrow", day: "2-digit"}, "W, 10/11/23")
	addFormatResult("en-US", {year: "2-digit", month: "long"}, "October 23")
	addFormatResult("en-US", {year: "2-digit", month: "long", day: "numeric"}, "October 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "long", day: "2-digit"}, "October 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "long", weekday: "long"}, "October 23 Wednesday")
	addFormatResult("en-US", {year: "2-digit", month: "long", weekday: "long", day: "numeric"}, "Wednesday, October 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "long", weekday: "long", day: "2-digit"}, "Wednesday, October 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "long", weekday: "short"}, "October 23 Wed")
	addFormatResult("en-US", {year: "2-digit", month: "long", weekday: "short", day: "numeric"}, "Wed, October 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "long", weekday: "short", day: "2-digit"}, "Wed, October 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "long", weekday: "narrow"}, "October 23 W")
	addFormatResult("en-US", {year: "2-digit", month: "long", weekday: "narrow", day: "numeric"}, "W, October 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "long", weekday: "narrow", day: "2-digit"}, "W, October 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "short"}, "Oct 23")
	addFormatResult("en-US", {year: "2-digit", month: "short", day: "numeric"}, "Oct 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "short", day: "2-digit"}, "Oct 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "short", weekday: "long"}, "Oct 23 Wednesday")
	addFormatResult("en-US", {year: "2-digit", month: "short", weekday: "long", day: "numeric"}, "Wednesday, Oct 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "short", weekday: "long", day: "2-digit"}, "Wednesday, Oct 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "short", weekday: "short"}, "Oct 23 Wed")
	addFormatResult("en-US", {year: "2-digit", month: "short", weekday: "short", day: "numeric"}, "Wed, Oct 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "short", weekday: "short", day: "2-digit"}, "Wed, Oct 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "short", weekday: "narrow"}, "Oct 23 W")
	addFormatResult("en-US", {year: "2-digit", month: "short", weekday: "narrow", day: "numeric"}, "W, Oct 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "short", weekday: "narrow", day: "2-digit"}, "W, Oct 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "narrow"}, "O 23")
	addFormatResult("en-US", {year: "2-digit", month: "narrow", day: "numeric"}, "O 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "narrow", day: "2-digit"}, "O 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "narrow", weekday: "long"}, "O 23 Wednesday")
	addFormatResult("en-US", {year: "2-digit", month: "narrow", weekday: "long", day: "numeric"}, "Wednesday, O 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "narrow", weekday: "long", day: "2-digit"}, "Wednesday, O 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "narrow", weekday: "short"}, "O 23 Wed")
	addFormatResult("en-US", {year: "2-digit", month: "narrow", weekday: "short", day: "numeric"}, "Wed, O 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "narrow", weekday: "short", day: "2-digit"}, "Wed, O 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "narrow", weekday: "narrow"}, "O 23 W")
	addFormatResult("en-US", {year: "2-digit", month: "narrow", weekday: "narrow", day: "numeric"}, "W, O 11, 23")
	addFormatResult("en-US", {year: "2-digit", month: "narrow", weekday: "narrow", day: "2-digit"}, "W, O 11, 23")
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

	const suite = new arktest.ArkTestsuite("Intl.DateTimeFormat(en-US)")
	suite.addTest("format() using year format '2-digit'", testDateFormatUsingOptionsYear2Digit)
	return suite.run()
}
