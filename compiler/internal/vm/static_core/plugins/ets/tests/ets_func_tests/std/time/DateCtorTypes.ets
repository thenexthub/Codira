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


/*---
desc: Data ctor tests
tags: [compile-only]
---*/


const intYears: int[] = [-300, -72, 1, 1900, 1969, 1970, 1971, 1980, 2000, 2020, 2023, 2024, 2030, 2038, 2039];
const intMonths: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2];
const intHours: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 15, 17, 22];
const intMinutes: int[] = [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56];


function tests_long(): void {
    for (let i: int = 0; i < intYears.length; i++) {
        let cd = new Date(intYears[i]);
        let cd1 = new Date(intYears[i]);
        let day: int = (i % 28) + 1;
        let cdYMD = new Date(intYears[i], intMonths[i % intMonths.length], day);
        let cdYMDOrig = new Date(intYears[i], intMonths[i % intMonths.length], day);
        let hour: int = intHours[i % intHours.length];
        let minute: int = intMinutes[i % intMinutes.length];
        let cdFull = new Date(
            intYears[i],
            intMonths[i % intMonths.length],
            day,
            hour,
            minute,
            hour);
        let cdFullOrig = new Date(
            intYears[i],
            intMonths[i % intMonths.length],
            day,
            hour,
            minute,
            hour);
    }

    for (let i: int = 0; i < intMonths.length; i++) {
        let cd = new Date(intMonths[i]);
        let cd1 = new Date(intMonths[i]);
    }

    for (let i: int = 0; i < intHours.length; i++) {
        let cd = new Date(intHours[i]);
        let cd1 = new Date(intHours[i]);
    }

    for (let i: int = 0; i < intMinutes.length; i++) {
        let cd = new Date(intMinutes[i]);
        let cd1 = new Date(intMinutes[i]);
    }
}

function main(): int {
    tests_long()

    return 0;
}
