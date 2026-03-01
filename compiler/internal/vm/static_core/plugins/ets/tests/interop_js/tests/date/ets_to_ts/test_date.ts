/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE- 2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const etsVm = globalThis.gtest.etsVm;

const DATASTR = etsVm.getClass('Ldate/test/ETSGLOBAL;').DATASTR;
const TIMESTAMP = etsVm.getClass('Ldate/test/ETSGLOBAL;').TIMESTAMP;

let etsDate = etsVm.getClass('Ldate/test/ETSGLOBAL;').etsDate;
let etsStrDate = etsVm.getClass('Ldate/test/ETSGLOBAL;').strDate;
let etsStampDate = etsVm.getClass('Ldate/test/ETSGLOBAL;').stampDate;

function checkGetofDate(etsDate: Date, jsDate: Date): void {
    ASSERT_TRUE(typeof etsDate === 'object');
    ASSERT_TRUE(etsDate.getFullYear() === jsDate.getFullYear());
    ASSERT_TRUE(etsDate.getMonth() === jsDate.getMonth());
    ASSERT_TRUE(etsDate.getDate() === jsDate.getDate());
    ASSERT_TRUE(etsDate.getDay() === jsDate.getDay());
    ASSERT_TRUE(etsDate.getHours() === jsDate.getHours());
    ASSERT_TRUE(etsDate.getMinutes() === jsDate.getMinutes());
    ASSERT_TRUE(etsDate.getSeconds() === jsDate.getSeconds());
    ASSERT_TRUE(etsDate.getMilliseconds() === jsDate.getMilliseconds());
    ASSERT_TRUE(etsDate.getTime() === jsDate.getTime());
    ASSERT_TRUE(etsDate.getUTCDate() === jsDate.getUTCDate());
    ASSERT_TRUE(etsDate.getUTCFullYear() === jsDate.getUTCFullYear());
    ASSERT_TRUE(etsDate.getUTCMonth() === jsDate.getUTCMonth());
    ASSERT_TRUE(etsDate.getUTCDay() === jsDate.getUTCDay());
    ASSERT_TRUE(etsDate.getUTCHours() === jsDate.getUTCHours());
    ASSERT_TRUE(etsDate.getUTCMinutes() === jsDate.getUTCMinutes());
    ASSERT_TRUE(etsDate.getUTCSeconds() === jsDate.getUTCSeconds());
    ASSERT_TRUE(etsDate.getUTCMilliseconds() === jsDate.getUTCMilliseconds());
    ASSERT_TRUE(etsDate.getTimezoneOffset() === jsDate.getTimezoneOffset());
    ASSERT_TRUE(etsDate.toJSON() === jsDate.toJSON());
    ASSERT_TRUE(etsDate.toDateString() === jsDate.toDateString());
    ASSERT_TRUE(etsDate.toISOString() === jsDate.toISOString());
    ASSERT_TRUE(etsDate.valueOf() === jsDate.valueOf());
    ASSERT_TRUE(etsDate.toUTCString() === jsDate.toUTCString());
    ASSERT_TRUE(etsDate.toLocaleDateString() === jsDate.toLocaleDateString())
}

function testInstanceofDate(): boolean {
    let res: boolean = false;
    res = (etsDate instanceof Date) &&
        (etsStampDate instanceof Date) &&
        (etsStrDate instanceof Date);
    return res;
}

function testGetOfDate(): void {
    let jsDate = new Date(2025, 2, 1, 1, 2, 3, 4);
    let jsStrDate = new Date(DATASTR);
    let jsStampDate = new Date(TIMESTAMP);
    checkGetofDate(etsDate, jsDate);
    checkGetofDate(etsStrDate, jsStrDate);
    checkGetofDate(etsStampDate, jsStampDate);
}

function testSetOfDate(): void {
    etsDate.setTime(1633024800000);
    ASSERT_TRUE(etsDate.getTime() === 1633024800000);

    etsDate.setDate(15);
    ASSERT_TRUE(etsDate.getDate() === 15);

    etsDate.setFullYear(2023);
    ASSERT_TRUE(etsDate.getFullYear() === 2023);
    etsDate.setFullYear(2024, 5, 15);
    ASSERT_TRUE(etsDate.getFullYear() === 2024);

    etsDate.setMonth(11);
    ASSERT_TRUE(etsDate.getMonth() === 11);

    etsDate.setHours(12);
    ASSERT_TRUE(etsDate.getHours() === 12);
    etsDate.setHours(14, 30, 15, 500);
    ASSERT_TRUE(etsDate.getHours() === 14);

    etsDate.setMinutes(45);
    ASSERT_TRUE(etsDate.getMinutes() === 45);
    etsDate.setMinutes(30, 15, 500);
    ASSERT_TRUE(etsDate.getMinutes() === 30);

    etsDate.setSeconds(30);
    ASSERT_TRUE(etsDate.getSeconds() === 30);
    etsDate.setSeconds(45, 500);
    ASSERT_TRUE(etsDate.getSeconds() === 45);

    etsDate.setUTCDate(20);
    ASSERT_TRUE(etsDate.getUTCDate() === 20);

    etsDate.setUTCFullYear(2025);
    ASSERT_TRUE(etsDate.getUTCFullYear() === 2025);
    etsDate.setUTCFullYear(2124, 6, 15);
    ASSERT_TRUE(etsDate.getUTCFullYear() === 2124);

    etsDate.setUTCMonth(11);
    ASSERT_TRUE(etsDate.getUTCMonth() === 11);
    etsDate.setUTCMonth(1, 28);
    ASSERT_TRUE(etsDate.getUTCMonth() === 1);

    etsDate.setUTCHours(12);
    ASSERT_TRUE(etsDate.getUTCHours() === 12);
    etsDate.setUTCHours(14, 30, 15, 500);
    ASSERT_TRUE(etsDate.getUTCHours() === 14);

    etsDate.setUTCMinutes(45);
    ASSERT_TRUE(etsDate.getUTCMinutes() === 45);
    etsDate.setUTCMinutes(30, 15, 500);
    ASSERT_TRUE(etsDate.getUTCMinutes() === 30);

    etsDate.setUTCSeconds(30);
    ASSERT_TRUE(etsDate.getUTCSeconds() === 30);
    etsDate.setUTCSeconds(45, 500);
    ASSERT_TRUE(etsDate.getUTCSeconds() === 45);

    etsDate.setMilliseconds(123);
    ASSERT_TRUE(etsDate.getMilliseconds() === 123);

    etsDate.setUTCMilliseconds(456);
    ASSERT_TRUE(etsDate.getUTCMilliseconds() === 456);
}

ASSERT_TRUE(testInstanceofDate());
testGetOfDate();
testSetOfDate();
