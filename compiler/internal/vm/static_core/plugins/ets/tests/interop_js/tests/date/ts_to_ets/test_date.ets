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

package date.test;

const dateString = '2025-03-01T01:02:03.000Z';
const timeStamp = 1609459200000;

function funObject(val: Object) {
    return val instanceof Date;
}

function funDateInstanceOf(val: Date) {
    return val instanceof Date;
}

function funDateTypeOf(val: Date) {
    return typeof val === 'object';
}

function funDateGetFullYear(val: Date) {
    return val.getUTCFullYear() === 2025;
}

function funDateGetMonth(val: Date) {
    return val.getUTCMonth() === 2;
}

function funDateGetDate(val: Date) {
    return val.getUTCDate() === 1;
}

function funDateGetDay(val: Date) {
    return val.getUTCDay() === 6;
}

function funDateGetHours(val: Date) {
    return val.getUTCHours() === 1;
}

function funDateGetMinutes(val: Date) {
    return val.getUTCMinutes() === 2;
}

function funDateGetSeconds(val: Date) {
    return val.getUTCSeconds() === 3;
}

function funDateGetMilliseconds(val: Date) {
    return val.getUTCMilliseconds() === 0;
}

function funDateToISOString(val: Date) {
    return val.toISOString() === dateString;
}

function funDateGetTime(val: Date) {
    return val.getTime() === timeStamp;
}

function funDateGetUTCDate(val: Date, obj: number): boolean {
    return val.getUTCDate() === obj;
}

function funDateGetUTCFullYear(val: Date, obj: number): boolean {
    return val.getUTCFullYear() === obj;
}

function funDateGetUTCMonth(val: Date, obj: number): boolean {
    return val.getUTCMonth() === obj;
}

function funDateGetUTCDay(val: Date, obj: number): boolean {
    return val.getUTCDay() === obj;
}

function funDateGetUTCHours(val: Date, obj: number): boolean {
    return val.getUTCHours() === obj;
}

function funDateGetUTCMinutes(val: Date, obj: number): boolean {
    return val.getUTCMinutes() === obj;
}

function funDateGetUTCSeconds(val: Date, obj: number): boolean {
    return val.getUTCSeconds() === obj;
}

function funDateGetUTCMilliSeconds(val: Date, obj: number): boolean {
    return val.getUTCMilliseconds() === obj;
}

function funDateGetTimeZoneOffSet(val: Date, obj: number): boolean {
    return val.getTimezoneOffset() === obj;
}

function funDateToJSON(val: Date, obj: string): boolean {
    return val.toJSON() === obj;
}

function funDateToDateString(val: Date, obj: string): boolean {
    return val.toDateString() === obj;
}

function funDateValueOf(val: Date, obj: number): boolean {
    return val.valueOf() === obj;
}

function testSetDateOfSetTime(val: Date): boolean {
    val.setTime(1633024800000);
    return val.getTime() === 1633024800000;
}

function testSetDateOfSetDate(val: Date): boolean {
    val.setDate(15); // workaround for #26000
    return val.getDate() === 15;
}

function testSetDateOfSetFullYear(val: Date): boolean {
    val.setFullYear(2023);
    return val.getFullYear() === 2023;
}

function testSetDateOfSetMonth(val: Date): boolean {
    val.setMonth(11);
    return val.getMonth() === 11;
}

function testSetDateOfSetUTCDate(val: Date): boolean {
    val.setUTCDate(20); // workaround for #26000
    return val.getUTCDate() === 20;
}

function testSetDateOfSetUTCFullYear(val: Date): boolean {
    val.setUTCFullYear(2025);
    return val.getUTCFullYear() === 2025;
}

function testSetDateOfSetUTCMonth(val: Date): boolean {
    val.setUTCMonth(10);
    return val.getUTCMonth() === 10;
}

function testSetDateOfMilliSeconds(val: Date): boolean {
    val.setMilliseconds(55); // workaround for #26000
    return val.getMilliseconds() === 55;
}

function testDateOfSetUTCMilliSeconds(val: Date): boolean {
    val.setUTCMilliseconds(11); // workaround for #26000
    return val.getUTCMilliseconds() === 11;
}

function testSetDateOfSetHours(val: Date): boolean {
    val.setHours(20); // workaround for #26000
    return val.getHours() === 20;
}

function testSetDateOfSetMinutes(val: Date): boolean {
    val.setMinutes(45); // workaround for #26000
    return val.getMinutes() === 45;
}

function testSetDateOfSetSeconds(val: Date): boolean {
    val.setSeconds(30); // workaround for #26000
    return val.getSeconds() === 30;
}

function testSetDateOfSetUTCHours(val: Date): boolean {
    val.setUTCHours(10); // workaround for #26000
    return val.getUTCHours() === 10;
}

function testSetDateOfSetUTCMinutes(val: Date): boolean {
    val.setUTCMinutes(55); // workaround for #26000
    return val.getUTCMinutes() === 55;
}

function testSetDateOfSetUTCSeconds(val: Date): boolean {
    val.setUTCSeconds(55); // workaround for #26000
    return val.getUTCSeconds() === 55;
}
