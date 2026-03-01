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

const success: int = 0;
const fail: int = 1;

function main(): int {
    arktest.assertEQ( languageCodes.length, countryCodes.length,  "Error! Size of languageCodes should be the same as countryCodes")

    let failures: int = 0;
    failures += check(FormatRangeNaNNumberUnion(), "Intl NumberFormat.formatRange NaN +number. Called union-types args");
    failures += check(FormatRangeNumberNaNUnion(), "Intl NumberFormat.formatRange -number NaN. Called union-types args");
    failures += check(FormatRangeNaNBigIntUnion(), "Intl NumberFormat.formatRange NaN +BigInt. Called union-types args");
    failures += check(FormatRangeBigIntNaNUnion(), "Intl NumberFormat.formatRange -BigInt NaN. Called union-types args");
    failures += check(FormatRangeNaNNaNUnion(), "Intl NumberFormat.formatRange NaN NaN. Called union-types args");
    return failures;
}

function check(result: number, message: string): int {
    if (result == 0) {
        return success;
    }
    console.println("FAILED: " + message + "\n");
    return fail;
}

const languageCodes: string[] = [ "en", "fr", "ca", "el", "no", "it", "zh", "ja", "de", "ru", "fa"];
const countryCodes: string[] = [ "US", "FR", "ES", "GR", "NO",  "",   "CN", "JP", "GE", "RU", "IR"];
const numberValues: Number[] = [-1234.5, 1234.5, -Infinity];
const bigIntValues = Array.of<BigInt>(-123456789000123456789000n, 123456789000123456789000n);
const EN: int = 0;
const FR: int = 1;
const ES: int = 2;
const GR: int = 3;
const NO: int = 4;
const IT: int = 5;
const CH: int = 6;
const JP: int = 7;
const GE: int = 8;
const RU: int = 9;
const IR: int = 10;


function CheckFormatRangeNaNUnion(locale: string, funcName: string, start: number | bigint, end: number | bigint, expected: string): int {
    let nf: Intl.NumberFormat;
    try {
        nf = new Intl.NumberFormat(locale);
    } catch(e) {
        console.log(funcName + ". Crashed call \"nf = new Intl.NumberFormat(" + locale + ")\"");
        return fail;
    }

    let str: string;
    try {
        str = nf.formatRange(start, end);
    } catch(e) {
        if (e.toString() == expected) {
            return success;
        }
        console.log(funcName + ". Error msg \"" + e + "\" is mismatched expected \"" + expected + "\" when call nf.formatRange with NaN");
        return fail;
    }
    console.log(funcName + ". Not issued TypeError when call nf.formatRange with NaN ");
    console.log(funcName + ". Detected on " + locale);
    return fail;
}

function FormatRangeNaNNumberUnion(): int {
    const funcName = "FormatRangeNaNNumberUnion";
    const startValue = NaN;
    const endValue = numberValues[0];
    const expected = "TypeError: Invalid start: NaN";
    for (let i = 0; i < languageCodes.length; i++) {
        let locale: string = languageCodes[i];
        if (countryCodes[i] != "") {
            locale = locale + "-" + countryCodes[i];
        }
        if (CheckFormatRangeNaNUnion(locale, funcName, startValue, endValue, expected) == fail) {
            return fail;
        }
    }
    return success;
}

function FormatRangeNumberNaNUnion(): int {
    const funcName = "FormatRangeNumberNaNUnion";
    const startValue = numberValues[1];
    const endValue = NaN;
    const expected = "TypeError: Invalid end: NaN";
    for (let i = 0; i < languageCodes.length; i++) {
        let locale: string = languageCodes[i];
        if (countryCodes[i] != "") {
            locale = locale + "-" + countryCodes[i];
        }
        if (CheckFormatRangeNaNUnion(locale, funcName, startValue, endValue, expected) == fail) {
            return fail;
        }
    }
    return success;
}

function FormatRangeNaNBigIntUnion(): int {
    const funcName = "FormatRangeNaNBigIntUnion";
    const startValue = NaN;
    const endValue = bigIntValues[1];
    const expected = "TypeError: Invalid start: NaN";
    for (let i = 0; i < languageCodes.length; i++) {
        let locale: string = languageCodes[i];
        if (countryCodes[i] != "") {
            locale = locale + "-" + countryCodes[i];
        }
        if (CheckFormatRangeNaNUnion(locale, funcName, startValue, endValue, expected) == fail) {
            return fail;
        }
    }
    return success;
}

function FormatRangeBigIntNaNUnion(): int {
    const funcName = "FormatRangeBigIntNaNUnion";
    const startValue = bigIntValues[0];
    const endValue = NaN;
    const expected = "TypeError: Invalid end: NaN";
    for (let i = 0; i < languageCodes.length; i++) {
        let locale: string = languageCodes[i];
        if (countryCodes[i] != "") {
            locale = locale + "-" + countryCodes[i];
        }
        if (CheckFormatRangeNaNUnion(locale, funcName, startValue, endValue, expected) == fail) {
            return fail;
        }
    }
    return success;
}

function FormatRangeNaNNaNUnion(): int {
    const funcName = "FormatRangeNaNNaNUnion";
    const startValue = NaN;
    const endValue = NaN;
    const expected = "TypeError: Invalid start: NaN";
    for (let i = 0; i < languageCodes.length; i++) {
        let locale: string = languageCodes[i];
        if (countryCodes[i] != "") {
            locale = locale + "-" + countryCodes[i];
        }
        if (CheckFormatRangeNaNUnion(locale, funcName, startValue, endValue, expected) == fail) {
            return fail;
        }
    }
    return success;
}
