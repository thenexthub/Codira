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
    let failures = 0;

    failures += test(testDateChangeYear(), "Try to change date's year");
    failures += test(testDateChangeMonth(), "Try to change date's month");
    failures += test(testDateChangeMonthOverflow(), "Month overflow adjusts year forward");
    failures += test(testDateChangeMonthNegative(), "Negative month adjusts year backward");
    failures += test(testDateChangeDay(), "Try to set date's new day");
    failures += test(testDateChangeDayZero(), "Day zero rolls back to previous month");
    failures += test(testDateChangeDayOverflow(), "Day overflow rolls forward to next month");
    failures += test(testDateChangeHour(), "Try to set new hour");
    failures += test(testDateChangeHourOverflow(), "Hour overflow updates following fields");
    failures += test(testDateChangeMinutes(), "Try to set new minutes");
    failures += test(testDateChangeSeconds(), "Try to set new seconds");
    failures += test(testSetFromLeapYear(), "Set nonleap year from leap year");
    failures += test(testSetToLeapYear(), "Set leap year from nonleap year");
    failures += test(testSetFromLeapDayToNonLeapYear(), "Adjust leap day when moving to common year");
    failures += test(testSetUTCFullYear(), "Set UTC full year");


    // next test raises arktest.assertTrue( "Invalid year/month pair")
    //failures += test(testDateChangeMonthBad(), "Try to change month to bad value");
    return failures;
}

function test(result: int, message: String): int {
    if (result == 0) {
        return 0;
    }
    console.println("FAILED: " + message);
    return 1;
}

function testSetFromLeapYear(): int {
    let d = new Date(2024, 11, 31, 11, 59)
    d.setFullYear(2023)
    arktest.assertEQ(d.getFullYear(), 2023)
    return 0
}

function testSetToLeapYear(): int {
    let d = new Date(2023, 11, 31, 11, 59)
    d.setFullYear(2024)
    arktest.assertEQ(d.getFullYear(), 2024)
    return 0
}

function testSetFromLeapDayToNonLeapYear(): int {
    let d = new Date(2024, 1, 29, 12, 0, 0, 0)
    d.setFullYear(2023)
    arktest.assertEQ(d.getFullYear(), 2023)
    arktest.assertEQ(d.getMonth(), 2)
    arktest.assertEQ(d.getDate(), 1)
    return 0
}

function testDateChangeYear(): int {

    let date: Date = new Date();
    let year = date.getFullYear();
    date.setFullYear(year - 1);

    if (date.getFullYear() == (year - 1)) return 0;
    return 1;

}

function daysInMonth(year: int, month: int): number {
    let tmp = new Date(year, month + 1, 0);
    return tmp.getDate();
}

function testDateChangeMonthImpl(date: Date, change: int) {
    let month = date.getMonth();
    let dmo = daysInMonth(date.getFullYear(), month);
    let newMonth = month + change;
    let dmc = daysInMonth(date.getFullYear(), newMonth);
    if (dmc < dmo) newMonth++;
    date.setMonth(newMonth);
    arktest.assertEQ(date.getMonth(), newMonth)
}

function testDateChangeMonth(): int {
    try {
        let date = new Date();
        let month = date.getMonth();
        let change: int = 1;
        if (month == 11) change = -change;
        testDateChangeMonthImpl(date, change);
    } catch (ae) {
        if (ae instanceof AssertionError) {
            return 1;
        }
    }
    return 0;

}

function testDateChangeMonthOverflow(): int {
    let date: Date = new Date(2023, 10, 15);
    date.setMonth(13);
    arktest.assertEQ(date.getFullYear(), 2024);
    arktest.assertEQ(date.getMonth(), 1);
    arktest.assertEQ(date.getDate(), 15);
    return 0;
}

function testDateChangeMonthNegative(): int {
    let date: Date = new Date(2023, 0, 10);
    date.setMonth(-1);
    arktest.assertEQ(date.getFullYear(), 2022);
    arktest.assertEQ(date.getMonth(), 11);
    arktest.assertEQ(date.getDate(), 10);
    return 0;
}

function testDateChangeMonthBad(): int {
    let date: Date = new Date();
    date.setMonth(13);

    if (isNaN(date.valueOf())) {
        return 0
    }

    return 1;
}


function testDateChangeDay(): int {
    let date: Date = new Date();
    let day: byte = date.getDate().toByte();
    let newDay: byte = 15;
    if (newDay == day) newDay++;
    date.setDate(newDay);

    if (date.getDate() == newDay) return 0;
    return 1;
}

function testDateChangeDayZero(): int {
    let date: Date = new Date(2024, 0, 15);
    date.setDate(0);
    arktest.assertEQ(date.getFullYear(), 2023);
    arktest.assertEQ(date.getMonth(), 11);
    arktest.assertEQ(date.getDate(), 31);
    return 0;
}

function testDateChangeDayOverflow(): int {
    let date: Date = new Date(2024, 6, 1);
    date.setDate(32);
    arktest.assertEQ(date.getFullYear(), 2024);
    arktest.assertEQ(date.getMonth(), 7);
    arktest.assertEQ(date.getDate(), 1);
    return 0;
}

function testDateChangeHour(): int {
    let date: Date = new Date();
    let hour: byte = date.getHours().toByte();
    let newHour: byte = 12;
    if (newHour == hour) newHour = 11;
    date.setHours(newHour);

    if (date.getHours() == newHour) return 0;
    return 1;
}

function testDateChangeHourOverflow(): int {
    let date: Date = new Date(2024, 0, 1, 10, 0, 0, 0);
    date.setHours(25, 90, 0, 500);
    arktest.assertEQ(date.getFullYear(), 2024);
    arktest.assertEQ(date.getMonth(), 0);
    arktest.assertEQ(date.getDate(), 2);
    arktest.assertEQ(date.getHours(), 2);
    arktest.assertEQ(date.getMinutes(), 30);
    arktest.assertEQ(date.getSeconds(), 0);
    arktest.assertEQ(date.getMilliseconds(), 500);
    return 0;
}

function testDateChangeMinutes(): int {
    let date: Date = new Date();
    let minutes: byte = date.getMinutes().toByte();
    let newMinutes: byte = 30;
    if (newMinutes == minutes) newMinutes = 21;
    date.setMinutes(newMinutes);

    if (date.getMinutes() == newMinutes) return 0;
    return 1;
}

function testDateChangeSeconds(): int {
    let date: Date = new Date();
    let seconds: int = date.getSeconds();
    let newSeconds: int = 30;
    if (newSeconds == seconds) newSeconds = 21;
    date.setSeconds(newSeconds);

    if (date.getSeconds() == newSeconds) return 0;
    return 1;
}

function testSetUTCFullYear(): int {
    let utcValue = Date.UTC(96, 1, 2, 3, 4, 5);
    let date: Date = new Date(utcValue);
    arktest.assertEQ(date.getUTCFullYear(), 1996);
    arktest.assertEQ(date.valueOf(), 823230245000);
    arktest.assertEQ(date.toUTCString(), "Fri, 02 Feb 1996 03:04:05 GMT");
    date.setUTCFullYear(96);
    arktest.assertEQ(date.getUTCFullYear(), 96);
    arktest.assertEQ(date.valueOf(), -59134913755000);
    arktest.assertEQ(date.toUTCString(), "Thu, 02 Feb 0096 03:04:05 GMT");
    return 0;
}
