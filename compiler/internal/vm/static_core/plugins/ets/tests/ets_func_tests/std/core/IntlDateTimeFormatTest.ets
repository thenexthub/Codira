/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function main(): int {
    let dtfTestsuite = new arktest.ArkTestsuite("dateTimeFormatTest")
    dtfTestsuite.addTest("dateTimeFormatTest1", dateTimeFormatTest1)
    dtfTestsuite.addTest("dateTimeFormatTest2", dateTimeFormatTest2)
    dtfTestsuite.addTest("dateTimeFormatTest3", dateTimeFormatTest3)
    dtfTestsuite.addTest("dateTimeFormatTest4", dateTimeFormatTest4)
    dtfTestsuite.addTest("dateTimeFormatTest5", dateTimeFormatTest5)
    dtfTestsuite.addTest("dateTimeFormatTest6", dateTimeFormatTest6)
    dtfTestsuite.addTest("dateTimeFormatTest7", dateTimeFormatTest7)
    dtfTestsuite.addTest("dateTimeFormatTest8", dateTimeFormatTest8)

    dtfTestsuite.addTest("dateToLocaleString1", dateToLocaleString1)
    dtfTestsuite.addTest("dateToLocaleString2", dateToLocaleString2)
    dtfTestsuite.addTest("dateToLocaleString3", dateToLocaleString3)

    return dtfTestsuite.run()
}

function dateTimeFormatTest1() {
    let newDate = new Date((24 * 60 * 60 * 1000).toLong() * (52 * 365 + 9 * 30 + 23));
    let options: Intl.DateTimeFormatOptions =
    {
        year: "numeric", month: "long", day: "numeric",
        hour: "2-digit", minute: "2-digit",
        timeZoneName: "short", timeZone: "UTC"
    };
    let intlObj = new Intl.DateTimeFormat(["pt-BR", "pt-PT"], options).format;
    arktest.assertEQ("8 de outubro de 2022 00:00 UTC", intlObj(newDate))
}

function dateTimeFormatTest2() {
    let newDate = new Date((24 * 60 * 60 * 1000).toLong() * (52 * 365 + 9 * 30 + 23));
    let swissLocales = ["de-CH", "fr-CH", "it-CH", "rm-CH"];
    let options: Intl.DateTimeFormatOptions =
    {
        weekday: "short",
        hour: "numeric", minute: "numeric",
        timeZone: "UTC", timeZoneName: "short"
    };
    let intlObj = new Intl.DateTimeFormat(swissLocales, options).format;
    arktest.assertEQ("Sa, 00:00 UTC", intlObj(newDate))
}

function dateTimeFormatTest3() {
    let newDate = new Date((24 * 60 * 60 * 1000).toLong() * (52 * 365 + 9 * 30 + 23));
    let options: Intl.DateTimeFormatOptions = {
        year: "numeric", month: "long", day: "numeric",
        hour: "2-digit", minute: "2-digit", timeZoneName: "short", timeZone: "UTC"
    };
    let intlObj = new Intl.DateTimeFormat("zh-CN", options);
    arktest.assertEQ('2022年10月8日 UTC 00:00', intlObj.format(newDate));
    arktest.assertEQ('UTC', intlObj.resolvedOptions().timeZone)
    arktest.assertEQ('short', intlObj.resolvedOptions().timeZoneName)

}

function dateTimeFormatTest4() {
    let options: Intl.DateTimeFormatOptions = {
            year: "numeric", month: "long", day: "numeric",
            hour: "2-digit", minute: "2-digit", timeZoneName: "short", timeZone: "UTC", hour12: true
    };
    let intlObj = new Intl.DateTimeFormat("en-us", options);
    let rops = intlObj.resolvedOptions();
    arktest.assertEQ(rops.locale, 'en-US');
    arktest.assertEQ(rops.calendar, 'gregory');
}

function dateTimeFormatTest5() {
    let options: Intl.DateTimeFormatOptions = {
            year: "numeric", month: "long", day: "numeric",
            hour: "2-digit", minute: "2-digit", timeZoneName: "short", timeZone: "UTC", hour12: true
    };
    let intlObj = new Intl.DateTimeFormat("zh-CN", options);
    let rops = intlObj.resolvedOptions();
    arktest.assertEQ(rops.locale, 'zh-CN');
    arktest.assertEQ(rops.calendar, 'gregory');
}

function dateTimeFormatTest6() {
    let options: Intl.DateTimeFormatOptions = {
            year: "numeric", month: "long", day: "numeric",
            hour: "2-digit", minute: "2-digit", timeZoneName: "short", timeZone: "UTC", hour12: true
    };
    let intlObj = new Intl.DateTimeFormat("am-ET-u-ca-ethioaa", options);
    let rops = intlObj.resolvedOptions();
    arktest.assertEQ(rops.locale, 'am-ET-u-ca-ethioaa');
    arktest.assertEQ(rops.calendar, 'ethioaa');
}

function dateTimeFormatTest7() {
    let date = new Date(Date.UTC(2021, 11, 20, 3, 23, 16));
    let options: Intl.DateTimeFormatOptions = { dateStyle: 'full', timeZone: 'UTC' };
    let dtf = new Intl.DateTimeFormat('en-GB', options);
    arktest.assertEQ("Monday, 20 December 2021", dtf.format(date));
}

function dateTimeFormatTest8() {
    // Test a specific timezone and offset formatting
    let date = new Date(Date.UTC(2023, 5, 15, 12, 0, 0)); // A date in summer (DST for NY)
    let options: Intl.DateTimeFormatOptions = {
        hour: 'numeric',
        timeZone: 'America/New_York',
        timeZoneName: 'shortOffset'
    };
    let dtf = new Intl.DateTimeFormat('en-US', options);
    // 12:00 UTC is 8:00 AM in New York (EDT = UTC-4), formatted with offset
    arktest.assertEQ("8 AM GMT-4", dtf.format(date));
}

function dateToLocaleString1() {
    let date = new Date(Date.UTC(2022, 9, 20, 18, 0, 0));
    let value = date.toLocaleString('en-US', {timeZone: 'UTC'});
    let zerotime = "10/20/2022, 6:00:00 PM";
    arktest.assertEQ(value, zerotime)
}

function dateToLocaleString2() {
    let date = new Date(Date.UTC(2022, 9, 20, 18, 0, 0));
    let value = date.toLocaleString('ko-KR', {timeZone: 'UTC'});
    let zerotime = "2022. 10. 20. 오후 6:00:00";
    arktest.assertEQ(value, zerotime)
}

function dateToLocaleString3() {
    let date = new Date(Date.UTC(2022, 9, 20, 18, 0, 0));
    let value = date.toLocaleTimeString('it-IT', { hour12: false, timeZone: 'UTC', timeZoneName: 'short' });
    let zerotime = "18:00:00 UTC";
    arktest.assertEQ(value, zerotime)
}
