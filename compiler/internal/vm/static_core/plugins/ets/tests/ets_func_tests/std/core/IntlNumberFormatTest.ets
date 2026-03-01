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
    failures += check(FormatBigIntUnion(), "Intl NumberFormat.format +-BigInt. Called union-type arg");
    return failures;
}

function check(result: number, message: string): int {
    if (result == 0) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}

const languageCodes: string[] = [ "en", "fr", "ca", "el", "no", "it", "zh", "ja", "de", "ru"];
const countryCodes: string[] = [ "US", "FR", "ES", "GR", "NO",  "",   "CN", "JP", "GE", "RU"];
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

function FormatBigIntUnion(): int {
    const funcName = "FormatBigIntUnion";
    const values = bigIntValues;
    const exp = Array.of<Array<string>>(negativeBigIntsAsStrings(), positiveBigIntsAsStrings());
    for (let i = 0; i < languageCodes.length; i++) {
        let locale: string = languageCodes[i];
        if (countryCodes[i] != "") {
            locale = locale + "-" + countryCodes[i];
        }
        for (let j = 0; j < values.length; j++) {
            const expected = exp[j][i];
            if (CheckFormatUnion(locale, funcName, values[j], expected) == fail) {
                return fail;
            }
        }
    }
    return success;
}

function separatorsBetweenNegAndPosAsStrings(): Array<string> {
    let separators = new Array<string>(languageCodes.length);
    for (let i: int = 0; i < separators.length; ++i) {
        separators[i] = new string("");
    }
    separators[EN] = String.fromCharCode(32, 8211, 32);
    separators[FR] = separators[EN];
    separators[ES] = String.fromCharCode(32, 45, 32);
    separators[GR] = separators[EN];
    separators[NO] = separators[EN];
    separators[IT] = separators[ES];
    separators[CH] = separators[ES];
    separators[JP] = String.fromCharCode(32, 65374, 32);
    separators[GE] = separators[EN];
    separators[RU] = separators[EN];
    return separators;
}

function positiveBigIntsAsStrings(): Array<string> {
    // Codes for bigIntValues[1]
    let expected = new Array<string>(languageCodes.length);
    for (let i: int = 0; i < expected.length; ++i) {
        expected[i] = new string("");
    }
    expected[EN] = String.fromCharCode(49, 50, 51, 44, 52, 53, 54, 44, 55, 56, 57, 44, 48, 48, 48, 44, 49, 50, 51, 44, 52, 53, 54, 44, 55, 56, 57, 44, 48, 48, 48);
    expected[FR] = String.fromCharCode(49, 50, 51, 8239, 52, 53, 54, 8239, 55, 56, 57, 8239, 48, 48, 48, 8239, 49, 50, 51, 8239, 52, 53, 54, 8239, 55, 56, 57, 8239, 48, 48, 48);
    expected[ES] = String.fromCharCode(49, 50, 51, 46, 52, 53, 54, 46, 55, 56, 57, 46, 48, 48, 48, 46, 49, 50, 51, 46, 52, 53, 54, 46, 55, 56, 57, 46, 48, 48, 48);
    expected[GR] = expected[ES];
    expected[NO] = String.fromCharCode(49, 50, 51, 160, 52, 53, 54, 160, 55, 56, 57, 160, 48, 48, 48, 160, 49, 50, 51, 160, 52, 53, 54, 160, 55, 56, 57, 160, 48, 48, 48);
    expected[IT] = expected[GR];
    expected[CH] = expected[EN];
    expected[JP] = expected[CH];
    expected[GE] = expected[ES];
    expected[RU] = String.fromCharCode(49, 50, 51, 160, 52, 53, 54, 160, 55, 56, 57, 160, 48, 48, 48, 160, 49, 50, 51, 160, 52, 53, 54, 160, 55, 56, 57, 160, 48, 48, 48);
    return expected;
}

function negativeBigIntsAsStrings(): Array<string> {
    // Codes for bigIntValues[0]
    let expected = new Array<string>(languageCodes.length);
    for (let i: int = 0; i < expected.length; ++i) {
        expected[i] = new string("");
    }
    expected[EN] = String.fromCharCode(45, 49, 50, 51, 44, 52, 53, 54, 44, 55, 56, 57, 44, 48, 48, 48, 44, 49, 50, 51, 44, 52, 53, 54, 44, 55, 56, 57, 44, 48, 48, 48);
    expected[FR] = String.fromCharCode(45, 49, 50, 51, 8239, 52, 53, 54, 8239, 55, 56, 57, 8239, 48, 48, 48, 8239, 49, 50, 51, 8239, 52, 53, 54, 8239, 55, 56, 57, 8239, 48, 48, 48);
    expected[ES] = String.fromCharCode(45, 49, 50, 51, 46, 52, 53, 54, 46, 55, 56, 57, 46, 48, 48, 48, 46, 49, 50, 51, 46, 52, 53, 54, 46, 55, 56, 57, 46, 48, 48, 48);
    expected[GR] = expected[ES];
    expected[NO] = String.fromCharCode(8722, 49, 50, 51, 160, 52, 53, 54, 160, 55, 56, 57, 160, 48, 48, 48, 160, 49, 50, 51, 160, 52, 53, 54, 160, 55, 56, 57, 160, 48, 48, 48);
    expected[IT] = expected[GR];
    expected[CH] = expected[EN];
    expected[JP] = expected[CH];
    expected[GE] = expected[ES];
    expected[RU] = String.fromCharCode(45, 49, 50, 51, 160, 52, 53, 54, 160, 55, 56, 57, 160, 48, 48, 48, 160, 49, 50, 51, 160, 52, 53, 54, 160, 55, 56, 57, 160, 48, 48, 48);
    return expected;
}

function CheckFormatNumber(locale: string, funcName: string, value: number, expected: string): int {
    let nf: Intl.NumberFormat;
    try {
        nf = new Intl.NumberFormat(locale);
    } catch(e) {
        console.log(funcName + ". Crashed call \"nf = new Intl.NumberFormat(" + locale + ")\"");
        return fail;
    }

    let str: string;
    try {
        str = nf.format(value);
    } catch(e) {
        console.log(funcName + ". Crashed call \"nf.format\". " + locale );
        return fail;
    }

    if (str == "") {
        console.log(funcName + ". Got empty str result. " + locale);
        return fail;
    }

    if (expected != str) {
        let arr = new Array<Number>(str.length);
        for (let ii = 0; ii < str.length; ++ii) {
            arr[ii] = str.codePointAt(ii)!;
        }
        console.log(funcName + ". Got: \"" + str + "\" Codes: " + arr.join(","));
        arr = new Array<Number>(expected.length);
        for (let ii = 0; ii < expected.length; ++ii) {
            arr[ii] = expected.codePointAt(ii)!;
        }
        console.log(funcName + ". Exp: Codes: " + arr.join(","));
        console.log(funcName + ". Detected on " + locale);
        return fail;
    }
    return success;
}

function CheckFormatUnion(locale: string, funcName: string, value: number | bigint, expected: string): int {
    let nf: Intl.NumberFormat;
    try {
        nf = new Intl.NumberFormat(locale);
    } catch(e) {
        console.log(funcName + ". Crashed call \"nf = new Intl.NumberFormat(" + locale + ")\"");
        return fail;
    }

    let str: string;
    try {
        str = nf.format(value);
    } catch(e) {
        console.log(funcName + ". Crashed call \"nf.format\". " + locale );
        return fail;
    }

    if (str == "") {
        console.log(funcName + ". Got empty str result. " + locale);
        return fail;
    }

    if (expected != str) {
        let arr = new Array<Number>(str.length);
        for (let ii = 0; ii < str.length; ++ii) {
            arr[ii] = str.codePointAt(ii)!;
        }
        console.log(funcName + ". Got: \"" + str + "\" Codes: " + arr.join(","));
        arr = new Array<Number>(expected.length);
        for (let ii = 0; ii < expected.length; ++ii) {
            arr[ii] = expected.codePointAt(ii)!;
        }
        console.log(funcName + ". Exp: Codes: " + arr.join(","));
        console.log(funcName + ". Detected on " + locale);
        return fail;
    }
    return success;
}
