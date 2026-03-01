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
const dateString = '2025-03-01T01:02:03.000Z';
const timeStamp = 1609459200000;

let funDateInstanceOf = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateInstanceOf');
let funDateTypeOf = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateTypeOf');
let funDateGetFullYear = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetFullYear');
let funDateGetMonth = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetMonth');
let funDateGetDate = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetDate');
let funDateGetDay = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetDay');
let funDateGetHours = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetHours');
let funDateGetMinutes = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetMinutes');
let funDateGetSeconds = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetSeconds');
let funDateGetMilliseconds = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetMilliseconds');
let funDateToISOString = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateToISOString');
let funDateGetTime = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetTime');
let funObject = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funObject');
let funDateGetUTCDate = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetUTCDate');
let funDateGetUTCFullYear = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetUTCFullYear');
let funDateGetUTCMonth = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetUTCMonth');
let funDateGetUTCDay = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetUTCDay');
let funDateGetUTCHours = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetUTCHours');
let funDateGetUTCMinutes = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetUTCMinutes');
let funDateGetUTCSeconds = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetUTCSeconds');
let funDateGetUTCMilliSeconds = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetUTCMilliSeconds');
let funDateGetTimeZoneOffSet = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateGetTimeZoneOffSet');
let funDateToJSON = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateToJSON');
let funDateToDateString = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateToDateString');
let funDateValueOf = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'funDateValueOf');

let testSetDateOfSetTime = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfSetTime');
let testSetDateOfSetDate = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfSetDate');
let testSetDateOfSetFullYear = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfSetFullYear');
let testSetDateOfSetMonth = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfSetMonth');
let testSetDateOfSetUTCDate = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfSetUTCDate');
let testSetDateOfSetUTCFullYear = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfSetUTCFullYear');
let testSetDateOfSetUTCMonth = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfSetUTCMonth');
let testSetDateOfMilliSeconds = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testSetDateOfMilliSeconds');
let testDateOfSetUTCMilliSeconds = etsVm.getFunction('Ldate/test/ETSGLOBAL;', 'testDateOfSetUTCMilliSeconds');

let dateStr = new Date(dateString);
let dateTimeStamp = new Date(timeStamp);

ASSERT_TRUE(funObject(dateStr));
ASSERT_TRUE(funDateInstanceOf(dateStr));
ASSERT_TRUE(funDateTypeOf(dateStr));
ASSERT_TRUE(funDateGetFullYear(dateStr));
ASSERT_TRUE(funDateGetMonth(dateStr));
ASSERT_TRUE(funDateGetDate(dateStr));
ASSERT_TRUE(funDateGetDay(dateStr));
ASSERT_TRUE(funDateGetHours(dateStr));
ASSERT_TRUE(funDateGetMinutes(dateStr));
ASSERT_TRUE(funDateGetSeconds(dateStr));
ASSERT_TRUE(funDateGetMilliseconds(dateStr));
ASSERT_TRUE(funDateToISOString(dateStr));
ASSERT_TRUE(funDateGetTime(dateTimeStamp));
ASSERT_TRUE(funDateGetUTCDate(dateTimeStamp, dateTimeStamp.getUTCDate()));
ASSERT_TRUE(funDateGetUTCFullYear(dateTimeStamp, dateTimeStamp.getUTCFullYear()));
ASSERT_TRUE(funDateGetUTCMonth(dateTimeStamp, dateTimeStamp.getUTCMonth()));
ASSERT_TRUE(funDateGetUTCDay(dateTimeStamp, dateTimeStamp.getUTCDay()));
ASSERT_TRUE(funDateGetUTCHours(dateTimeStamp, dateTimeStamp.getUTCHours()));
ASSERT_TRUE(funDateGetUTCMinutes(dateTimeStamp, dateTimeStamp.getUTCMinutes()));
ASSERT_TRUE(funDateGetUTCSeconds(dateTimeStamp, dateTimeStamp.getUTCSeconds()));
ASSERT_TRUE(funDateGetUTCMilliSeconds(dateTimeStamp, dateTimeStamp.getUTCMilliseconds()));
ASSERT_TRUE(funDateGetTimeZoneOffSet(dateTimeStamp, dateTimeStamp.getTimezoneOffset()));
ASSERT_TRUE(funDateToJSON(dateTimeStamp, dateTimeStamp.toJSON()));
ASSERT_TRUE(funDateToDateString(dateTimeStamp, dateTimeStamp.toDateString()));
ASSERT_TRUE(funDateValueOf(dateTimeStamp, dateTimeStamp.valueOf()));

ASSERT_TRUE(testSetDateOfSetTime(dateTimeStamp));
ASSERT_TRUE(testSetDateOfSetDate(dateTimeStamp));
ASSERT_TRUE(testSetDateOfSetFullYear(dateTimeStamp));
ASSERT_TRUE(testSetDateOfSetMonth(dateTimeStamp));
ASSERT_TRUE(testSetDateOfSetUTCDate(dateTimeStamp));
ASSERT_TRUE(testSetDateOfSetUTCFullYear(dateTimeStamp));
ASSERT_TRUE(testSetDateOfSetUTCMonth(dateTimeStamp));
ASSERT_TRUE(testSetDateOfMilliSeconds(dateTimeStamp));
ASSERT_TRUE(testDateOfSetUTCMilliSeconds(dateTimeStamp));
