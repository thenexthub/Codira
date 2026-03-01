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

function testDateToLocaleString(): void {
    const date = new Date(2024, 1, 19, 10, 0, 0)

    const langTag = "zh-CN"
    const locale = new Intl.Locale(langTag)

    arktest.assertEQ(date.toLocaleString(langTag), "2024/2/19 10:00:00")
    arktest.assertEQ(date.toLocaleString(locale), "2024/2/19 10:00:00")
    arktest.assertEQ(date.toLocaleString(), date.toLocaleString(undefined))

    const options: Intl.DateTimeFormatOptions = { hour: "2-digit", minute: "2-digit", second: "2-digit" }
    arktest.assertEQ(date.toLocaleString(locale, options), "10:00:00")
}

function testDateToLocaleTimeString(): void {
    const date = new Date(2024, 1, 19, 10, 0, 0)

    const langTag = "zh-CN"
    const locale = new Intl.Locale(langTag)

    arktest.assertEQ(date.toLocaleTimeString(langTag), "10:00:00")
    arktest.assertEQ(date.toLocaleTimeString(locale), "10:00:00")

    const options: Intl.DateTimeFormatOptions = {
        year: "2-digit",
        month: "2-digit",
        day: "2-digit",
        hour: "2-digit",
        minute: "2-digit"
    }

    arktest.assertEQ(date.toLocaleTimeString(langTag, options), "24/02/19 10:00")
    arktest.assertEQ(date.toLocaleTimeString(locale, options), "24/02/19 10:00")
    arktest.assertEQ(date.toLocaleTimeString(), date.toLocaleTimeString(undefined))

    arktest.assertEQ(date.toLocaleTimeString(langTag, { hour: "2-digit", minute: "2-digit" }), "10:00")
    arktest.assertEQ(date.toLocaleTimeString(locale, { hour: "2-digit", minute: "2-digit" }), "10:00")

    arktest.assertEQ(date.toLocaleTimeString(langTag, { year: "numeric", month: "2-digit", day: "2-digit" }), "2024/02/19 10:00:00")
    arktest.assertEQ(date.toLocaleTimeString(locale, { year: "numeric", month: "2-digit", day: "2-digit" }), "2024/02/19 10:00:00")

    const styleOptions: Intl.DateTimeFormatOptions = { dateStyle: "medium" }
    arktest.expectError(() => {
        const result = date.toLocaleTimeString(langTag, styleOptions)
    }, new TypeError("Invalid option : dateStyle"))
}

function testDateToLocaleDateString(): void {
    const date = new Date(2024, 1, 19, 10, 0, 0)

    const langTag = "zh-CN"
    const locale = new Intl.Locale(langTag)

    arktest.assertEQ(date.toLocaleDateString(langTag), "2024/2/19")
    arktest.assertEQ(date.toLocaleDateString(locale), "2024/2/19")
    arktest.assertEQ(date.toLocaleDateString(), date.toLocaleDateString(undefined))

    const timeOptions: Intl.DateTimeFormatOptions = { hour: "2-digit", minute: "2-digit", second: "2-digit" }
    arktest.assertEQ(date.toLocaleDateString(langTag, timeOptions), "2024/2/19 10:00:00")
    arktest.assertEQ(date.toLocaleDateString(locale, timeOptions), "2024/2/19 10:00:00")

    const dateOptions: Intl.DateTimeFormatOptions = { year: "numeric", month: "short", day: "2-digit" }
    arktest.assertEQ(date.toLocaleDateString(langTag, dateOptions), "2024年2月19日")
    arktest.assertEQ(date.toLocaleDateString(locale, dateOptions), "2024年2月19日")

    const customOptions: Intl.DateTimeFormatOptions = {
        year: "numeric",
        month: "short",
        day: "2-digit",
        hour: "2-digit",
        minute: "2-digit"
    }

    arktest.assertEQ(date.toLocaleDateString(langTag, customOptions), "2024年2月19日 10:00")
    arktest.assertEQ(date.toLocaleDateString(locale, customOptions), "2024年2月19日 10:00")

    const timeStyleOptions: Intl.DateTimeFormatOptions = { timeStyle: "medium" }
    arktest.expectError(() => {
        const result = date.toLocaleDateString(langTag, timeStyleOptions)
    }, new TypeError("Invalid option : timeStyle"))
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Date")
    suite.addTest("toLocaleString", testDateToLocaleString)
    suite.addTest("toLocaleTimeString", testDateToLocaleTimeString)
    suite.addTest("toLocaleDateString", testDateToLocaleDateString)
    return suite.run()
}
