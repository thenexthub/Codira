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

function check(current_answer: number, correct_answer: number, test: string) {
    if (current_answer != correct_answer) {
        if (!Number.isNaN(current_answer) || !Number.isNaN(correct_answer)) {
            arktest.assertTrue( false, "incorrect " + test + " answer " + current_answer + " instead of " + correct_answer)
            return 1;
        }
    }
    return 0;
}

function main() {
    let fails = 0;
    let tests: string[] = [
        "",
        "123",
        "NaN",
        "nan",
        "-1",
        "1e8",
        "e8",
        "1e",
        "1rew",
        "9.8",
        "Infinity",
        "-Infinity",
        "-1.11111",
        "aaa",
        "\n",
        "\t",
        "    ",
        "   123   ",
        "0x11",
        "0o123",
        "0b0101010",
        "0b222",
        "0x00 001",
        "0x11 abacaba",
        "0xR",
        " 0x1",
        "0x01R",
        "+5",
        " +5",
        " +5a",
        ".5",
        "00015"
    ]
    const correct_cts: number[] = [
        0,
        123,
        NaN,
        NaN,
        -1,
        100000000,
        NaN,
        NaN,
        NaN,
        9.8,
        Infinity,
        -Infinity,
        -1.11111,
        NaN,
        0,
        0,
        0,
        123,
        17,
        83,
        42,
        NaN,
        NaN,
        NaN,
        NaN,
        1,
        NaN,
        5,
        5,
        NaN,
        0.5,
        15
    ];
    const correct_pfs: number[] = [
        NaN,
        123,
        NaN,
        NaN,
        -1,
        100000000,
        NaN,
        1,
        1,
        9.8,
        Infinity,
        -Infinity,
        -1.11111,
        NaN,
        NaN,
        NaN,
        NaN,
        123,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        5,
        5,
        5,
        0.5,
        15
    ];

    const force_log = false;
    for (let test_id = 0; test_id < tests.length; test_id++) {
        let test = tests[test_id];
        if (force_log) {
            console.log("[" + test + "]");
        }
        let ctor_answer = (new Number(test)); // change to .valueOf to run script in TS
        let parse_answer = Number.parseFloat(test);
        fails += check(ctor_answer, correct_cts[test_id], "Number(" + test + ")");
        fails += check(parse_answer, correct_pfs[test_id], "parseFloat(" + test + ")");
    }
    return fails;
}
