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

type FormatToPartsResult = [string | undefined, Intl.DateTimeFormatOptions | undefined, Intl.DateTimeRangeFormatPart[]]

const FORMAT_RESULTS = new Array<FormatToPartsResult>()

function addFormatRangeToPartsResult(locale: string | undefined, options: Intl.DateTimeFormatOptions | undefined, result: Intl.DateTimeRangeFormatPart[]): void {
    const res: [string | undefined, Intl.DateTimeFormatOptions | undefined, Intl.DateTimeRangeFormatPart[]] = [locale, options, result]
    FORMAT_RESULTS.push(res)
}

const RANGE_START = new Date("2024-10-11T10:20:30.444Z")
const RANGE_END = new Date("2024-10-12T20:30:50.444Z")

class RPart implements Intl.DateTimeRangeFormatPart {
    private _type: Intl.DateTimeFormatPartTypes
    private _value: string
    private _source: Intl.DateTimeRangeFormatPartSource

    constructor() {
        this._type = "literal"
        this._value = ""
        this._source = "shared"
    }

    constructor(type: Intl.DateTimeFormatPartTypes, value: string, source: Intl.DateTimeRangeFormatPartSource) {
        this._type = type
        this._value = value
        this._source = source
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

    get source(): Intl.DateTimeRangeFormatPartSource {
        return this._source
    }

    set source(source: Intl.DateTimeRangeFormatPartSource) {
        this._source = source
    }

    static $_invoke(type: Intl.DateTimeFormatPartTypes, value: string, source: Intl.DateTimeRangeFormatPartSource): RPart {
        return new RPart(type, value, source)
    }
}

function initFormatResults(): void {
    if (FORMAT_RESULTS.length > 0) {
        return
    }

    const resultUS:Intl.DateTimeRangeFormatPart[] = [
        RPart("month", "10", "startRange"), RPart("literal", "/", "startRange"), 
        RPart("day", "11", "startRange"), RPart("literal", "/", "startRange"), 
        RPart("year", "2024", "startRange"), RPart("literal", " – ", "shared"), 
        RPart("month", "10", "endRange"), RPart("literal", "/", "endRange"), 
        RPart("day", "12", "endRange"), RPart("literal", "/", "endRange"), 
        RPart("year", "2024", "endRange")
    ]

    addFormatRangeToPartsResult("en-US", { year: "numeric", month: "2-digit", day: "2-digit", timeZone: "UTC" }, resultUS)

    const resultCN:Intl.DateTimeRangeFormatPart[] = [
        RPart("year", "2024", "startRange"), RPart("literal", "/", "startRange"), 
        RPart("month", "10", "startRange"), RPart("literal", "/", "startRange"), 
        RPart("day", "11", "startRange"), RPart("literal", " – ", "shared"), 
        RPart("year", "2024", "endRange"), RPart("literal", "/", "endRange"),
        RPart("month", "10", "endRange"), RPart("literal", "/", "endRange"), 
        RPart("day", "12", "endRange")
    ]

    addFormatRangeToPartsResult("zh-CN", { year: "numeric", month: "2-digit", day: "2-digit", timeZone: "UTC" }, resultCN)
}

function verifyResults(actual: Intl.DateTimeRangeFormatPart[], expected: Intl.DateTimeRangeFormatPart[]): boolean {
    if (actual.length != expected.length) {
        return false
    }

    for (let i = 0; i < actual.length; i++) {
        const valid = actual[i].type == expected[i].type 
                && actual[i].value == expected[i].value 
                && actual[i].source == expected[i].source

        if (!valid) {
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
        const actualResult = formatter.formatRangeToParts(RANGE_START, RANGE_END)
        const expectedResult = res[2]

        const resultIsValid = verifyResults(actualResult, expectedResult);
        if (!resultIsValid) {
            const range = `${RANGE_START}, ${RANGE_END}`
            const failMsg = `Unexpected DateTimeFormat(${locale}, ${options}).formatRangeToParts(${range}) result`
            arktest.assertTrue(false, failMsg)
        }
    })
}

function main(): int {
    initFormatResults()

    const suite = new arktest.ArkTestsuite("Intl.DateTimeFormat.formatToParts()")
    suite.addTest("with formatting options", dateFormatToPartsWithOptions)
    return suite.run()
}
