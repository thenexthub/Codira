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

function testDateToUTCString() {
    let date = new Date('August 19, 1975 23:15:30 UTC');
    arktest.assertEQ(date.toUTCString(), 'Tue, 19 Aug 1975 23:15:30 GMT');
    date.toTimeString();
    date.getDate();
    date.toString();
    arktest.assertEQ(date.valueOf(), 177722130000);

    let timeStamp = new Date(628021900000);
    arktest.assertEQ(timeStamp.toUTCString(), 'Sat, 25 Nov 1989 18:31:40 GMT');
    timeStamp.toTimeString();
    timeStamp.getDate();
    timeStamp.toString()
    arktest.assertEQ(timeStamp.valueOf(), 628021900000);

    let dateTime = new Date(Date.UTC(1995, 11, 17, 3, 24, 0));
    let utc = Date.UTC(1995, 11, 17, 3, 24, 0);
    arktest.assertEQ(dateTime.valueOf(), utc);
    arktest.assertEQ(dateTime.toUTCString(), 'Sun, 17 Dec 1995 03:24:00 GMT');

    dateTime.toTimeString();
    dateTime.getDate();
    dateTime.toString()
    date = new Date('Thu 03 Jun 2093 04:18 GMT');
    arktest.assertEQ(date.toUTCString(), 'Wed, 03 Jun 2093 04:18:00 GMT');
    date.toTimeString();
    date.getDate();
    date.toString()
    arktest.assertEQ(date.valueOf(), 3894841080000);
}

function testGetDate0002() {
    const birthday = new Date('August 1, 1975 23:15:30');
    let expected = new Date(1975, 7, 1, 23, 15, 30);
    arktest.assertEQ(birthday.valueOf(), expected.valueOf());
    arktest.assertEQ(birthday.getDate(), expected.getDate());
}

function testGetDay0002() {
    const birthday = new Date('August 17, 1975 23:15:30');
    let expected = new Date(1975, 7, 17, 23, 15, 30);
    arktest.assertEQ(birthday.valueOf(), expected.valueOf());
    arktest.assertEQ(birthday.getDay(), expected.getDay());
}

function testGetFullYear0002() {
    const moonLanding = new Date('July 20, 50 00:20:18');
    let expected = new Date(1950, 6, 20, 0, 20, 18);
    arktest.assertEQ(moonLanding.valueOf(), expected.valueOf());
    arktest.assertEQ(moonLanding.getFullYear(), expected.getFullYear());
}

function testGetHours0003() {
    const birthday = new Date('March 13, 08 04:20');
    let expected = new Date(2008, 2, 13, 4, 20, 0);
    arktest.assertEQ(birthday.valueOf(), expected.valueOf());
    arktest.assertEQ(birthday.getHours(), expected.getHours());
}

function testGetMonth0002() {
    const moonLanding = new Date('January 20, 69 00:20:18');
    let expected = new Date(1969, 0, 20, 0, 20, 18);
    arktest.assertEQ(moonLanding.valueOf(), expected.valueOf());
    arktest.assertEQ(moonLanding.getMonth(), expected.getMonth());
}

function testGetSeconds0003() {
    const moonLanding = new Date('July 20, 69 00:20:18');
    let expected = new Date(1969, 6, 20, 0, 20, 18);
    arktest.assertEQ(moonLanding.valueOf(), expected.valueOf());
    arktest.assertEQ(moonLanding.getSeconds(), expected.getSeconds());
}

function testGetTime0002() {
    const moonLanding = new Date('January 20, 69 00:20:00');
    const copy = new Date();
	const copyTimeStamp =copy.setTime(moonLanding.getTime())
    let expected = new Date(1969, 0, 20, 0, 20, 0);
	const expectedCopy = new Date();
	const expectedCopyTimeStamp = expectedCopy.setTime(expected.getTime())
    arktest.assertEQ(moonLanding.valueOf(), expected.valueOf());
    arktest.assertEQ(copyTimeStamp, expectedCopyTimeStamp);
}

function testGetTimezoneOffset0004() {
    const date1 = new Date('');
    const result = date1.getTimezoneOffset();
    arktest.assertTrue(isNaN(result));
}

function testGetUTCDate0002() {
    const date1 = new Date('August 1, 1975 23:15:30 GMT+11:00');
    const result = date1.getUTCDate();
    arktest.assertEQ(result, 1);
}

function testGetUTCDay0002() {
    const birthday = new Date('August 17, 1975 23:15:30 GMT+11:00');
    const result = birthday.getUTCDay();
    arktest.assertEQ(result, 0);
}

function testGetUTCFullYear0002() {
    const moonLanding = new Date('July 20, 50 00:20:18 GMT+11:00');
    const result = moonLanding.getUTCFullYear();
    arktest.assertEQ(result, 1950);
}

function testGetUTCHours0003() {
    const birthday = new Date('March 13, 08 04:20 GMT-00:00');
    const result = birthday.getUTCHours();
    arktest.assertEQ(result, 4);
}

function testGetUTCMinutes0003() {
    const birthday = new Date('March 13, 08 04:20 GMT+11:00');
    const result = birthday.getUTCMinutes();
    arktest.assertEQ(result, 20);
}

function testGetUTCMonth0002() {
    const moonLanding = new Date('January 31, 1975, 23:15:30 GMT+11:00');
    const result = moonLanding.getUTCMonth();
    arktest.assertEQ(result, 0);
}

function testGetUTCSeconds0003() {
    const moonLanding = new Date('July 20, 1969, 20:18:18 UTC');
    const result = moonLanding.getSeconds();
    arktest.assertEQ(result, 18);
}

function testSetDate0004() {
    const event = new Date('August 19, 1975 23:15:30');
    const targetDate = 31;
    event.setDate(targetDate);
    let expected = new Date(1975, 7, 19, 23, 15, 30);
    expected.setDate(targetDate);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.getDate(), expected.getDate());
}

function testSetHours0003() {
    const event = new Date('August 19, 1975 23:15:30');
    event.setHours(23);
    let expected = new Date(1975, 7, 19, 23, 15, 30);
    expected.setHours(23);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
}

function testSetMinutes0002() {
    const event = new Date('August 19, 1975 23:15:30');
    event.setMinutes(0);
    let expected = new Date(1975, 7, 19, 23, 15, 30);
    expected.setMinutes(0);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
}

function testSetMonth0006() {
    const event = new Date('August 19, 1975 23:15:30');
    event.setMonth(8);
    let expected = new Date(1975, 7, 19, 23, 15, 30);
    expected.setMonth(8);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
}

function testSetSeconds0001() {
    const event = new Date('August 19, 1975 23:15:30');
    let expected = new Date(1975, 7, 19, 23, 15, 30);
    event.setSeconds(-1);
    expected.setSeconds(-1);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
    event.setSeconds(-10);
    expected.setSeconds(-10);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
    event.setSeconds(60);
    expected.setSeconds(60);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
    event.setSeconds(70);
    expected.setSeconds(70);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
}

function testSetTime0001() {
    const launchDate = new Date('July 1, 1999, 12:00:00');
    const futureDate = new Date();
    futureDate.setTime(launchDate.getTime());
    let expected = new Date(1999, 6, 1, 12, 0, 0)
    arktest.assertEQ(futureDate.valueOf(), expected.valueOf());
    arktest.assertEQ(futureDate.toString(), expected.toString());
}

function testSetUTCDate0004() {
    const event = new Date('August 19, 1975 23:15:30');
    event.setUTCDate(31);
    const result = event.getUTCDate();
    arktest.assertEQ(result, 31);
}

function testSetUTCHours0002() {
    const event = new Date('August 19, 1975 23:15:30');
    let target = 0;
    event.setUTCHours(target);
    let expected = new Date(1975, 7, 19, 23, 15, 30);
    expected.setUTCHours(target);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
}

function testSetUTCMinutes0002() {
    const event = new Date('August 19, 1975 23:15:30');
    const target = 0;
    event.setUTCMinutes(target);
    let expected = new Date(1975, 7, 19, 23, 15, 30)
    expected.setUTCMinutes(target);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
}

function testSetUTCMonth0006() {
    const event = new Date('August 19, 1975 23:15:30');
    const targetDate = 0;
    event.setUTCMonth(8, targetDate);
    let expected = new Date(1975, 7, 19, 23, 15, 30)
    expected.setUTCMonth(8, targetDate);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
}

function testSetUTCSeconds0002() {
    const event = new Date('August 19, 1975 23:15:30');
    const target = 0;
    event.setUTCSeconds(target);
    let expected = new Date(1975, 7, 19, 23, 15, 30);
    expected.setUTCSeconds(target);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
}

function testDateToDateString0004() {
    const event = new Date('');
    arktest.assertEQ(event.toString(), 'Invalid Date');
}

function testDateToISOString0001() {
    const event = new Date('05 October 2011 14:48 UTC');
    let expected = new Date(Date.UTC(2011, 9, 5, 14, 48, 0));
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
    arktest.assertEQ(event.toISOString(), expected.toISOString());
}

function testDateToString0001() {
    const event: Date = new Date('August 19, 1975 23:15:30');
    let expected = new Date(1975, 7, 19, 23, 15, 30);
    arktest.assertEQ(event.valueOf(), expected.valueOf());
    arktest.assertEQ(event.toString(), expected.toString());
}

function testSpecialDateString() {
    let date = new Date('0');
    let expected = new Date(2000, 0, 1, 0, 0, 0);
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.toString(), expected.toString());

    date = new Date('1');
    expected = new Date(2001, 0, 1, 0, 0, 0);
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.toString(), expected.toString());

    date = new Date('50');
    expected = new Date(1950, 0, 1, 0, 0, 0);
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.toString(), expected.toString());

    date = new Date('49');
    expected = new Date(2049, 0, 1, 0, 0, 0);
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.toString(), expected.toString());

    date = new Date('foobar 2000');
    expected = new Date(2000, 0, 1, 0, 0, 0);
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.toString(), expected.toString());

    date = new Date('May 16 UTC');
    expected = new Date(Date.UTC(2001, 4, 16, 0, 0, 0));
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.toString(), expected.toString());

    date = new Date('May 16 UTC+2');
    expected = new Date(Date.UTC(2001, 4, 15, 22, 0, 0));
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.toString(), expected.toString());
    
    date = new Date('2');
    expected = new Date(2002, 0, 1, 0, 0, 0);
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.toString(), expected.toString());

    date = new Date('14');
    expected = new Date(2014, 0, 1, 0, 0, 0);
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.toString(), expected.toString());

    date = new Date('11.2');
    expected = new Date(2011, 1, 1, 0, 0, 0);
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.toString(), expected.toString());

    date = new Date('Jan 1, 2000');
    expected = new Date(2000, 0, 1, 0, 0, 0);
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.toString(), expected.toString());

    date = new Date('99');
    expected = new Date(1999, 0, 1, 0, 0, 0);
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.toString(), expected.toString());
}

function testLeapYear() {
    let date = new Date('Feb 29, 2020');
    let expected = new Date(2020, 1, 29, 0, 0, 0);
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.getMonth(), expected.getMonth());

    date = new Date('February 29, 2020 12:00:00 UTC');
    expected = new Date(Date.UTC(2020, 1, 29, 12, 0, 0));
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.getMonth(), expected.getMonth());
    arktest.assertEQ(date.getDate(), expected.getDate());

    date = new Date('February 29, 2021 12:00:02 UTC');
    expected = new Date(Date.UTC(2021, 1, 29, 12, 0, 2));
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.getFullYear(), expected.getFullYear());
    arktest.assertEQ(date.getMonth(), expected.getMonth());
    arktest.assertEQ(date.getDate(), expected.getDate());
    arktest.assertEQ(date.getHours(), expected.getHours());
    arktest.assertEQ(date.getSeconds(), expected.getSeconds());

    date = new Date('29 February, 2008');
    expected = new Date(2008, 1, 29, 0, 0, 0);
    arktest.assertEQ(date.valueOf(), expected.valueOf());
    arktest.assertEQ(date.getFullYear(), expected.getFullYear());
    arktest.assertEQ(date.getMonth(), expected.getMonth());
    arktest.assertEQ(date.getDate(), expected.getDate());
}

function testInvalidDate() {
    arktest.expectError(() => { new Date(" 2024-02-19T08:00:00") }, "Invalid time value")
    arktest.expectError(() => { new Date("Not date") }, "Invalid time value")
}

function testDateToLocaleStringConstEvent0001() {
    const event = new Date(Date.UTC(2012, 11, 20, 3, 0, 0));
    const formattedDate = event.toLocaleString('en-GB', {
      timeZone: 'UTC'
    });
    arktest.assertEQ(formattedDate, '20/12/2012, 03:00:00');
}

function isUTC8Timezone() {
    const timeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
    return timeZone === 'Asia/Shanghai';
}

function testDateToLocaleTimeString0001() {
    let event = new Date('August 19, 1975 23:15:30 GMT+00:00');
    if (isUTC8Timezone()) {
        arktest.assertEQ(event.toLocaleTimeString('en-US'), '7:15:30 AM');
    }
    let expected = new Date(Date.UTC(1975, 7, 19, 23, 15, 30));
    arktest.assertEQ(event.toLocaleTimeString('en-US'), expected.toLocaleTimeString('en-US'));
}

function testDateToLocaleTimeStringAndLocaleString0001() {
    let date = new Date();
    // Defaults for toLocaleXXXString
    let dtfDate = new Intl.DateTimeFormat();
    let dtfTime = new Intl.DateTimeFormat(
      [], {hour: 'numeric', minute: 'numeric', second: 'numeric'});
      let dtfAll = new Intl.DateTimeFormat(
      [], {year: 'numeric', month: 'numeric', day: 'numeric',
           hour: 'numeric', minute: 'numeric', second: 'numeric'});
    arktest.assertEQ(dtfAll.format(date), date.toLocaleString());
    arktest.assertEQ(dtfDate.format(date), date.toLocaleDateString());
    arktest.assertEQ(dtfTime.format(date), date.toLocaleTimeString());
    // Specify locale, default options for toLocaleXXXString
    let locale = ['sr'];
    dtfDate = new Intl.DateTimeFormat(locale);
    dtfTime = new Intl.DateTimeFormat(
      locale, {hour: 'numeric', minute: 'numeric', second: 'numeric'});
    dtfAll = new Intl.DateTimeFormat(
      locale, {year: 'numeric', month: 'numeric', day: 'numeric',
               hour: 'numeric', minute: 'numeric', second: 'numeric'});
    arktest.assertEQ(dtfAll.format(date), date.toLocaleString(locale));
    arktest.assertEQ(dtfDate.format(date), date.toLocaleDateString(locale));
    arktest.assertEQ(dtfTime.format(date), date.toLocaleTimeString(locale));
    // Specify locale and options for toLocaleXXXString
    locale = ['ko'];
    let dtf = new Intl.DateTimeFormat(locale, {year: 'numeric', month: 'long', day: 'numeric',
        hour: 'numeric', minute: '2-digit', second: 'numeric'});
    arktest.assertEQ(dtf.format(date), date.toLocaleString(locale, {year: 'numeric', month: 'long', day: 'numeric',
        hour: 'numeric', minute: '2-digit', second: 'numeric'}));
    arktest.assertEQ(dtf.format(date), date.toLocaleDateString(locale, {year: 'numeric', month: 'long', day: 'numeric',
        hour: 'numeric', minute: '2-digit', second: 'numeric'}));
    arktest.assertEQ(dtf.format(date), date.toLocaleTimeString(locale, {year: 'numeric', month: 'long', day: 'numeric',
        hour: 'numeric', minute: '2-digit', second: 'numeric'}));   
}

function testInvalidDate0002() {
    const errorMsg = "Invalid time value";

    let birthday = new Date('August 0, 1975 23:15:30');
    arktest.expectError(() => { birthday.getDate() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCDate() }, errorMsg);
    arktest.expectError(() => { birthday.getDay() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCDay() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCFullYear() }, errorMsg);
    arktest.expectError(() => { birthday.getFullYear() }, errorMsg);
    arktest.expectError(() => { birthday.getHours() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCHours() }, errorMsg);
    arktest.expectError(() => { birthday.getMilliseconds() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMilliseconds() }, errorMsg);
    arktest.expectError(() => { birthday.getSeconds() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCSeconds() }, errorMsg);
    arktest.expectError(() => { birthday.getMinutes() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMinutes() }, errorMsg);
    arktest.expectError(() => { birthday.getMonth() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMonth() }, errorMsg);
    arktest.expectError(() => { birthday.getTime() }, errorMsg);
    arktest.expectError(() => { birthday.getTimezoneOffset() }, errorMsg);
    
    birthday = new Date('August 0, 1975 23:15:30 GMT+11:00');
    arktest.expectError(() => { birthday.getDate() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCDate() }, errorMsg);
    arktest.expectError(() => { birthday.getDay() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCDay() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCFullYear() }, errorMsg);
    arktest.expectError(() => { birthday.getFullYear() }, errorMsg);
    arktest.expectError(() => { birthday.getHours() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCHours() }, errorMsg);
    arktest.expectError(() => { birthday.getMilliseconds() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMilliseconds() }, errorMsg);
    arktest.expectError(() => { birthday.getSeconds() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCSeconds() }, errorMsg);
    arktest.expectError(() => { birthday.getMinutes() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMinutes() }, errorMsg);
    arktest.expectError(() => { birthday.getMonth() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMonth() }, errorMsg);
    arktest.expectError(() => { birthday.getTime() }, errorMsg);
    arktest.expectError(() => { birthday.getTimezoneOffset() }, errorMsg);

    birthday = new Date('August 32, 1975 23:15:30');
    arktest.expectError(() => { birthday.getDate() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCDate() }, errorMsg);
    arktest.expectError(() => { birthday.getDay() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCDay() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCFullYear() }, errorMsg);
    arktest.expectError(() => { birthday.getFullYear() }, errorMsg);
    arktest.expectError(() => { birthday.getHours() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCHours() }, errorMsg);
    arktest.expectError(() => { birthday.getMilliseconds() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMilliseconds() }, errorMsg);
    arktest.expectError(() => { birthday.getSeconds() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCSeconds() }, errorMsg);
    arktest.expectError(() => { birthday.getMinutes() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMinutes() }, errorMsg);
    arktest.expectError(() => { birthday.getMonth() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMonth() }, errorMsg);
    arktest.expectError(() => { birthday.getTime() }, errorMsg);
    arktest.expectError(() => { birthday.getTimezoneOffset() }, errorMsg);

    birthday = new Date('August 0, 1975 23:15:30 GMT+11:00');
    arktest.expectError(() => { birthday.getDate() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCDate() }, errorMsg);
    arktest.expectError(() => { birthday.getDay() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCDay() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCFullYear() }, errorMsg);
    arktest.expectError(() => { birthday.getFullYear() }, errorMsg);
    arktest.expectError(() => { birthday.getHours() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCHours() }, errorMsg);
    arktest.expectError(() => { birthday.getMilliseconds() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMilliseconds() }, errorMsg);
    arktest.expectError(() => { birthday.getSeconds() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCSeconds() }, errorMsg);
    arktest.expectError(() => { birthday.getMinutes() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMinutes() }, errorMsg);
    arktest.expectError(() => { birthday.getMonth() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMonth() }, errorMsg);
    arktest.expectError(() => { birthday.getTime() }, errorMsg);
    arktest.expectError(() => { birthday.getTimezoneOffset() }, errorMsg);

    birthday = new Date('');
    arktest.expectError(() => { birthday.getDate() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCDate() }, errorMsg);
    arktest.expectError(() => { birthday.getDay() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCDay() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCFullYear() }, errorMsg);
    arktest.expectError(() => { birthday.getFullYear() }, errorMsg);
    arktest.expectError(() => { birthday.getHours() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCHours() }, errorMsg);
    arktest.expectError(() => { birthday.getMilliseconds() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMilliseconds() }, errorMsg);
    arktest.expectError(() => { birthday.getSeconds() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCSeconds() }, errorMsg);
    arktest.expectError(() => { birthday.getMinutes() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMinutes() }, errorMsg);
    arktest.expectError(() => { birthday.getMonth() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMonth() }, errorMsg);
    arktest.expectError(() => { birthday.getTime() }, errorMsg);
    arktest.expectError(() => { birthday.getTimezoneOffset() }, errorMsg);

    birthday = new Date('March 13, 09 04:60');
    arktest.expectError(() => { birthday.getMinutes() }, errorMsg);
    arktest.expectError(() => { birthday.getUTCMinutes() }, errorMsg);
}

function testSetTime0002() {
    let birthday = new Date('');
    let copy = new Date();
    copy.setTime(birthday.getTime());
    arktest.assertTrue(Double.isNaN(copy.setTime(birthday.getTime())), 'result='+copy.setTime(birthday.getTime()));
}

function testSetTime0003() {
    let birthday = new Date('');
    birthday.setTime(0);
    arktest.assertEQ(birthday.getTime(), 0, 'result='+birthday.getTime());
}

function main() {
    let myTestsuite = new arktest.ArkTestsuite("DateTest");
    myTestsuite.addTest('testDateToUTCString', testDateToUTCString);
    myTestsuite.addTest('testGetDate0002', testGetDate0002);
    myTestsuite.addTest('testGetDay0002', testGetDay0002);
    myTestsuite.addTest('testGetFullYear0002', testGetFullYear0002);
    myTestsuite.addTest('testGetHours0003', testGetHours0003);
    myTestsuite.addTest('testGetMonth0002', testGetMonth0002);
    myTestsuite.addTest('testGetSeconds0003', testGetSeconds0003);
    myTestsuite.addTest('testGetTime0002', testGetTime0002);
    myTestsuite.addTest('testGetTimezoneOffset0004', testGetTimezoneOffset0004);
    myTestsuite.addTest('testGetUTCDate0002', testGetUTCDate0002);
    myTestsuite.addTest('testGetUTCDay0002', testGetUTCDay0002);
    myTestsuite.addTest('testGetUTCFullYear0002', testGetUTCFullYear0002);
    myTestsuite.addTest('testGetUTCHours0003', testGetUTCHours0003);
    myTestsuite.addTest('testGetUTCMinutes0003', testGetUTCMinutes0003);
    myTestsuite.addTest('testGetUTCMonth0002', testGetUTCMonth0002);
    myTestsuite.addTest('testGetUTCSeconds0003', testGetUTCSeconds0003);
    myTestsuite.addTest('testSetDate0004', testSetDate0004);
    myTestsuite.addTest('testSetHours0003', testSetHours0003);
    myTestsuite.addTest('testSetMinutes0002', testSetMinutes0002);
    myTestsuite.addTest('testSetMonth0006', testSetMonth0006);
    myTestsuite.addTest('testSetSeconds0001', testSetSeconds0001);
    myTestsuite.addTest('testSetTime0001', testSetTime0001);
    myTestsuite.addTest('testSetUTCDate0004', testSetUTCDate0004);
    myTestsuite.addTest('testSetUTCHours0002', testSetUTCHours0002);
    myTestsuite.addTest('testSetUTCMinutes0002', testSetUTCMinutes0002);
    myTestsuite.addTest('testSetUTCMonth0006', testSetUTCMonth0006);
    myTestsuite.addTest('testSetUTCSeconds0002', testSetUTCSeconds0002);
    myTestsuite.addTest('testDateToDateString0004', testDateToDateString0004);
    myTestsuite.addTest('testDateToISOString0001', testDateToISOString0001);
    myTestsuite.addTest('testDateToString0001', testDateToString0001);
    myTestsuite.addTest('testInvalidDate', testInvalidDate);
    myTestsuite.addTest('testLeapYear', testLeapYear);
    myTestsuite.addTest('testSpecialDateString', testSpecialDateString);
    myTestsuite.addTest('testDateToLocaleStringConstEvent0001', testDateToLocaleStringConstEvent0001);
    myTestsuite.addTest('testDateToLocaleTimeString0001', testDateToLocaleTimeString0001);
    myTestsuite.addTest('testDateToLocaleTimeStringAndLocaleString0001', testDateToLocaleTimeStringAndLocaleString0001);
    myTestsuite.addTest('testInvalidDate0002', testInvalidDate0002);
    myTestsuite.addTest('testSetTime0002', testSetTime0002);
    myTestsuite.addTest('testSetTime0003', testSetTime0003);
    myTestsuite.run();
}
