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

type FormatToPartsResult = [string | undefined, Intl.DateTimeFormatOptions | undefined, Intl.DateTimeFormatPart[]]

const FORMAT_RESULTS = new Array<FormatToPartsResult>()

class FmtPart implements Intl.DateTimeFormatPart {
    private _type: Intl.DateTimeFormatPartTypes
    private _value: String

    constructor() {
        this._type = "literal"
        this._value = ""
    }

    constructor(type: Intl.DateTimeFormatPartTypes, value: string) {
        this._type = type
        this._value = value
    }

    get type(): Intl.DateTimeFormatPartTypes {
        return this._type
    }

    set type(value: Intl.DateTimeFormatPartTypes) {
        this._type = value
    }

    get value(): string {
        return this._value
    }

    set value(value: string) {
        this._value = value
    }

    static $_invoke(type: Intl.DateTimeFormatPartTypes, value: string):FmtPart {
        return new FmtPart(type, value)
    }
}

function addFormatToPartsResult(locale: string | undefined, options: Intl.DateTimeFormatOptions | undefined, result: Intl.DateTimeFormatPart[]): void {
    const res: [string | undefined, Intl.DateTimeFormatOptions | undefined, Intl.DateTimeFormatPart[]] = [locale, options, result]
    FORMAT_RESULTS.push(res)
}

const TEST_DATE = new Date("2023-10-11T00:20:00Z")

function initFormatResults(): void {
    if (FORMAT_RESULTS.length > 0) {
        return
    }

    const result001:Intl.DateTimeFormatPart[] = [FmtPart("year", "2023"), FmtPart("literal", "/"), FmtPart("month", "10"), FmtPart("literal", "/"), FmtPart("day", "11")]
    addFormatToPartsResult("zh-CN", { year: "numeric", month: "2-digit", day: "2-digit" }, result001)
}

function verifyResults(actual: Intl.DateTimeFormatPart[], expected: Intl.DateTimeFormatPart[]): boolean {
    if (actual.length != expected.length) {
        return false
    }

    for (let i = 0; i < actual.length; i++) {
        if (actual[i].type != expected[i].type || actual[i].value != expected[i].value) {
            return false
        }
    }

    return true
}

function dateFormatToPartsWithOptions(): void {
    FORMAT_RESULTS.forEach((res: FormatToPartsResult) => {
        const locale = res[0]
        const options = res[1]

        const formatter = new Intl.DateTimeFormat(locale, options)
        const actualResult = formatter.formatToParts(TEST_DATE)
        const expectedResult = res[2]

        const failMsg = `Unexpected DateTimeFormat.formatToParts(${locale}, ${options}) result`
        arktest.assertTrue(verifyResults(actualResult, expectedResult), failMsg)
    })
}

function main(): int {
    initFormatResults()

    const suite = new arktest.ArkTestsuite("Intl.DateTimeFormat.formatToParts()")
    suite.addTest("with formatting options", dateFormatToPartsWithOptions)
    return suite.run()
}
