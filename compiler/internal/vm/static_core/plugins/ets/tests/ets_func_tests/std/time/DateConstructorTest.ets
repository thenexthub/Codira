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

function main(): int {
    const suite = new arktest.ArkTestsuite("Date")

    suite.addTest("Test MS constructor with Int", testIntMsConstructor)
    suite.addTest("Test year and month constructor with Int", testIntYearMonthConstructor)
    suite.addTest("Test year, month and day constructor with Int", testIntYearMonthDayConstructor)
    suite.addTest("Test YMDH constructor with Int", testIntYearMonthDayHoursConstructor)
    suite.addTest("Test YMDH and minutes constructor with Int", testIntYearMonthDayHoursMinutesConstructor)
    suite.addTest("Test YMDH minutes and seconds constructor with Int", testIntYearMonthDayHoursMinutesSecondsConstructor)
    suite.addTest("Test YMDH minutes seconds and millis constructor with Int", testIntYearMonthDayHoursMinutesSecondsMillisConstructor)

    suite.addTest("Test constructor with string", testStringConstructor)
    suite.addTest("Test constructor with Date object", testDateConstructor)
    suite.addTest("Test constructor with string (local TZ)", testStringConstructorLocalTZ)
    suite.addTest("Test constructor with string (invalid arguments)", testStringConstructorInvalidArgs)
    suite.addTest("Test constructor with string (expanded year)", testStringConstructorExpandedYear)
    suite.addTest("Test constructor with string and offset", testStringConstructorWithOffset)
    suite.addTest("Test constructor with RFC1123 string", testStringConstructorRFC1123)
    suite.addTest("Test Date.parse matches constructor", testDateParseConsistency)

    return suite.run()
}

function test(result: int, message: String): int {
    if (result == 0) {
        return 0;
    }
    console.println("FAILED: " + message);
    return 1;
}

const msPerMinute: int = 60000;
const intYears: int[] = [-300, -72, 1, 0, 99, 100, 1900, 1969, 1970, 1971, 1980, 2000, 2020, 2023, 2024, 2030, 2038, 2039];
const intMonths: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5];
const intHours: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 15, 17, 18, 19, 20, 21];
const intMinutes: int[] = [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 0, 4, 8];
const valueIntYears: int[] = [-300, -72, 1901, 1900, 1999, 100, 1900, 1969, 1970, 1971, 1980, 2000, 2020, 2023, 2024, 2030, 2038, 2039];

const dateString: string[] = ["2019-11-14T00:55:31.820Z", "2024-02-29T00:55:31.950Z", "2000-01-01T00:00:00.001Z"]

function testIntMsConstructor(): void {
    for (let i: int = 0; i < intYears.length; i++) {
        let cd = new Date(intYears[i]);
        arktest.assertEQ(cd.valueOf(), intYears[i])
    }
}

function testIntYearMonthConstructor(): void {
    for (let i: int = 0; i < intYears.length; i++) {
        let cd = new Date(intYears[i], intMonths[i]);
        let utc = Date.UTC(intYears[i], intMonths[i]);

        arktest.assertEQ(utc, cd.valueOf() - cd.getTimezoneOffset() * msPerMinute)
        checkDateWithIntYMD(cd, valueIntYears[i], intMonths[i], 1)
    }
}

function testIntYearMonthDayConstructor(): void {
    for (let i: int = 0; i < intYears.length; i++) {
        let day: int = intMonths[i] + 1;
        let cd = new Date(intYears[i], intMonths[i], day);
        let utc = Date.UTC(intYears[i], intMonths[i], day);

        arktest.assertEQ(utc, cd.valueOf() - cd.getTimezoneOffset() * msPerMinute)
        checkDateWithIntYMD(cd, valueIntYears[i], intMonths[i], day);
    }
}

function testIntYearMonthDayHoursConstructor(): void {
    for (let i: int = 0; i < intYears.length; i++) {
        let day: int = intMonths[i] + 1;
        let cd = new Date(intYears[i], intMonths[i], day, intHours[i]);
        let utc = Date.UTC(intYears[i], intMonths[i], day, intHours[i]);

        arktest.assertEQ(utc, cd.valueOf() - cd.getTimezoneOffset() * msPerMinute)
        checkDateWithIntYMDH(cd, valueIntYears[i], intMonths[i], day, intHours[i]);
    }
}

function testIntYearMonthDayHoursMinutesConstructor(): void {
    for (let i: int = 0; i < intYears.length; i++) {
        let day: int = intMonths[i] + 1;
        let cd = new Date(intYears[i], intMonths[i], day, intHours[i], intMinutes[i]);
        let utc = Date.UTC(intYears[i], intMonths[i], day, intHours[i], intMinutes[i]);

        arktest.assertEQ(utc, cd.valueOf() - cd.getTimezoneOffset() * msPerMinute)
        checkDateWithIntYMDHM(cd, valueIntYears[i], intMonths[i], day, intHours[i], intMinutes[i]);
    }
}

function testIntYearMonthDayHoursMinutesSecondsConstructor(): void {
    for (let i: int = 0; i < intYears.length; i++) {
        let day: int = intMonths[i] + 1;
        let cd = new Date(intYears[i], intMonths[i], day, intHours[i], intMinutes[i], intMinutes[i]);
        let utc = Date.UTC(intYears[i], intMonths[i], day, intHours[i], intMinutes[i], intMinutes[i]);

        arktest.assertEQ(utc, cd.valueOf() - cd.getTimezoneOffset() * msPerMinute)
        checkDateWithIntYMDHMS(cd, valueIntYears[i], intMonths[i], day, intHours[i], intMinutes[i], intMinutes[i]);
    }
}

function testIntYearMonthDayHoursMinutesSecondsMillisConstructor(): void {
    for (let i: int = 0; i < intYears.length; i++) {
        let day: int = intMonths[i] + 1;
        let millis: int = intMinutes[i] * 10;

        let cd = new Date(
            intYears[i],
            intMonths[i],
            day,
            intHours[i],
            intMinutes[i],
            intMinutes[i],
            millis);
        let utc = Date.UTC(intYears[i], intMonths[i], day, intHours[i], intMinutes[i], intMinutes[i], millis);

        arktest.assertEQ(utc, cd.valueOf() - cd.getTimezoneOffset() * msPerMinute)
        checkDateWithIntYMDHMSM(cd, valueIntYears[i], intMonths[i], day, intHours[i], intMinutes[i], intMinutes[i], millis);
    }
}

function testStringConstructor(): void {
    for (let i: int = 0; i < dateString.length; i++) {
        let cd: Date = new Date(dateString[i]);
        arktest.assertEQ(dateString[i], cd.toISOString())
    }
}

function testDateConstructor(): void {
    for (let i: int = 0; i < intYears.length; i++) {
        let day: int = intMonths[i] + 1;
        let tmp = new Date(
            intYears[i],
            intMonths[i],
            day,
            intHours[i],
            intMinutes[i],
            intMinutes[i]);
        let cd = new Date(tmp);

        arktest.assertEQ(tmp.valueOf(), cd.valueOf())
        checkDateWithIntYMDHMS(tmp, valueIntYears[i], intMonths[i], day, intHours[i], intMinutes[i], intMinutes[i]);
    }
}

function testStringConstructorLocalTZ(): void {
    const localDateTime = new Date(2024, 1, 19, 8, 0, 0)
    const isoLocalDateTime = new Date("2024-02-19T08:00:00")
    arktest.assertEQ(localDateTime.getTime(), isoLocalDateTime.getTime())

    const isoLocalDate = new Date("1970-01-01")
    arktest.assertEQ(isoLocalDate.getTime(), 0)
}

function testStringConstructorInvalidArgs(): void {
    arktest.expectError(() => {
        const startingSpaceDate = new Date(" 2024-02-19T08:00:00")
    }, "Invalid time value: ' 2024-02-19T08:00:00'")

    arktest.expectError(() => {
        const trailingSpaceDate = new Date("2024-02-19T08:00:00 ")
    }, "Invalid time value: '2024-02-19T08:00:00 '")

    const ms4Date = new Date("2024-02-28T00:55:31.1234Z")
    arktest.assertEQ(Date.UTC(2024, 1, 28, 0, 55, 31, 123), ms4Date.getTime())

    const ms2Date = new Date("2024-02-28T00:55:31.12Z")
    arktest.assertEQ(Date.UTC(2024, 1, 28, 0, 55, 31, 120), ms2Date.getTime())

    arktest.expectError(() => {
        const missingMsDate = new Date("2024-02-28T00:55:31.")
    }, "Invalid time value: '2024-02-28T00:55:31.'")

    arktest.expectError(() => {
        const invalidMonth = new Date("2000-99-01T00:00:00.001Z")
    }, "Invalid time value: '2000-99-01T00:00:00.001Z'")

    arktest.expectError(() => {
        const invalidDay = new Date("2000-01-99T00:00:00.001Z")
    }, "Invalid time value: '2000-01-99T00:00:00.001Z'")

    arktest.expectError(() => {
        const invalidHour = new Date("2000-01-01T99:00:00.001Z")
    }, "Invalid time value: '2000-01-01T99:00:00.001Z'")

    arktest.expectError(() => {
        const invalidMinute = new Date("2000-01-01T00:99:00.001Z")
    }, "Invalid time value: '2000-01-01T00:99:00.001Z'")

    arktest.expectError(() => {
        const invalidSecond = new Date("2000-01-01T00:00:99.001Z")
    }, "Invalid time value: '2000-01-01T00:00:99.001Z'")

    arktest.expectError(
        () => { new Date("275761") },
        new FormatError("Invalid time value: '275761'")
    )
    arktest.expectError(
        () => { new Date("1622505600000") },
        new FormatError("Invalid time value: '1622505600000'")
    )
}

function testStringConstructorExpandedYear(): void {
    const date = new Date("-000001-07-01T00:00Z")
    arktest.assertEQ(date.toUTCString(), "Thu, 01 Jul -0001 00:00:00 GMT")
}

function testStringConstructorWithOffset(): void {
    const plusOffset = new Date("2024-02-19T08:00:00+05:30")
    arktest.assertEQ(plusOffset.getTime(), Date.UTC(2024, 1, 19, 2, 30, 0, 0))

    const minusOffset = new Date("2024-02-19T08:00:00-04:00")
    arktest.assertEQ(minusOffset.getTime(), Date.UTC(2024, 1, 19, 12, 0, 0, 0))
}

function testStringConstructorRFC1123(): void {
    const rfcDate = new Date("Tue, 19 Feb 2024 08:00:00 GMT")
    arktest.assertEQ(rfcDate.toISOString(), "2024-02-19T08:00:00.000Z")
}

function testDateParseConsistency(): void {
    const samples: string[] = [
        "2019-11-14T00:55:31.820Z",
        "2024-02-29T00:55:31.950Z",
        "Tue, 19 Feb 2024 08:00:00 GMT",
        "2024-02-19T08:00:00-04:00"
    ]
    for (let i: int = 0; i < samples.length; i++) {
        const parsed = Date.parse(samples[i])
        const constructed = new Date(samples[i]).getTime()
        arktest.assertEQ(parsed, constructed, "Date.parse result mismatch")
    }
}


function checkDateWithIntYMD(date: Date, year: int, month: int, day: int): void {
    checkDateWithIntYMDH(date, year, month, day, 0);
}

function checkDateWithIntYMDH(date: Date, year: int, month: int, day: int, hour: int): void {
    checkDateWithIntYMDHM(date, year, month, day, hour, 0);
}

function checkDateWithIntYMDHM(date: Date, year: int, month: int, day: int, hour: int, minutes: int): void {
    checkDateWithIntYMDHMS(date, year, month, day, hour, minutes, 0);
}

function checkDateWithIntYMDHMS(date: Date, year: int, month: int, day: int, hour: int, minutes: int, seconds: int): void {
    checkDateWithIntYMDHMSM(date, year, month, day, hour, minutes, seconds, 0);
}

function checkDateWithIntYMDHMSM(cd: Date, year: int, month: int, day: int, hour: int, minutes: int, seconds: int, millis: int): void {
    arktest.assertEQ(cd.getFullYear(), year, "year mismatch")
    arktest.assertEQ(cd.getMonth(), month, "month mismatch")
    arktest.assertEQ(cd.getDate(), day, "day mismatch")
    arktest.assertEQ(cd.getHours(), hour, "hour mismatch")
    arktest.assertEQ(cd.getMinutes(), minutes, "minute mismatch")
    arktest.assertEQ(cd.getSeconds(), seconds, "second mismatch")
    arktest.assertEQ(cd.getMilliseconds(), millis, "ms mismatch")
}
