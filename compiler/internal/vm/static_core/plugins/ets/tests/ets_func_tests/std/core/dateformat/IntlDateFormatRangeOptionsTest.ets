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

const expectedResults: FormatResult[] = []

function addResult(locale: string | undefined, options: Intl.DateTimeFormatOptions | undefined, result: string): void {
    const res: [string | undefined, Intl.DateTimeFormatOptions | undefined, string] = [locale, options, result]
    expectedResults.push(res)
}

const TEST_RANGE_BEGIN = new Date('2024-10-11T10:20:30Z')
const TEST_RANGE_END = new Date('2024-10-12T07:35:50Z')

function initExpectedResults(): void {
    if (expectedResults.length > 0) {
        return
    }

    addResult('en-US', { year: 'numeric', month: '2-digit', day: '2-digit', timeZone: 'UTC' }, '10/11/2024 – 10/12/2024')
}

function dateFormatRangeUsingFormattingOptionsTest(): void {
    expectedResults.forEach((res: FormatResult) => {
        const locale = res[0]
        const options = res[1]

        const formatter = new Intl.DateTimeFormat(locale, options)
        const actualResult = formatter.formatRange(TEST_RANGE_BEGIN, TEST_RANGE_END)

        const expectedResult = res[2]
        if (actualResult !== expectedResult) {
         arktest.assertEQ(actualResult, expectedResult, `Unexpected date formatting result for locale: ${locale} and options: ${options}`)
        }
    })
}

function main(): int {
    initExpectedResults()

    const suite = new arktest.ArkTestsuite('Intl.DateTimeFormat.formatRange()')
    suite.addTest('using formatting options', dateFormatRangeUsingFormattingOptionsTest)
    return suite.run()
}
