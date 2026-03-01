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

function test_int_array(): void {
    let data: Array<int> = [-128, -42, -14, 0, 16, 48, 127]
    check_array_reverse(data)
}

function test_number_array(): void {
    let data: Array<number> = [-1.7e+308, -3.6e+42, -3.6e+42, 0, -0.6e+16, 0.4e+105, -3.6e+42, 1.7e+308, 1.7e+308, 4294967294,
        1802338304.00001, -10000000000.99999]
    check_array_reverse(data)
}

function test_short_array(): void {
    let data: Array<short> = [2, 3, 2, 3, 3, 4, 5]
    check_array_reverse(data)
}

function test_long_array(): void {
    let data: Array<long> = [2, 3, 2, 3, 3, 4, 5]
    check_array_reverse(data)
}

function test_float_array(): void {
    let data: Array<float> = [1.1f, 2.2f, 3.3f, 3.3f, 5.5f, 2.2f, 4.4f]
    check_array_reverse(data)
}

function test_boolean_array(): void {
    let data: Array<boolean> = [true, false, false, true, false, true, false]
    check_array_reverse(data)
}

function test_char_array(): void {
    let data: Array<char> = [c'a', c'b', c'c', c'a', c'd', c'a', c'b']
    check_array_reverse(data)
}

function test_string_array(): void {
    let data: Array<string> = ["this", "is", "hello", "world", "hello"]
    check_array_reverse(data)
}

function test_union_array(): void {
    let data: Array<int | boolean | char | string> = ["this", c'i', 1, true, "0"]
    check_array_reverse(data)
}

function test_long_len_array_reverse() {
    let inp = new Array<number>();
    let orig_exp = new Array<number>();
    let rev_exp = new Array<number>();
    // SWAPPED_BETWEEN_SAFEPOINT_THRESHOLD = 50000UL;
    let len = 50001;
    for (let i: number = 0; i < len; i++) {
      inp.push(i);
      orig_exp.push(i);
      rev_exp.push(len - 1 - i);
    }
    check_array<number>(inp.reverse(), orig_exp);
    check_array<number>(inp.reverse(), rev_exp);
}

function check_array_reverse<T>(data: Array<T>): void {
    let data_rev = Array.from(data).reverse()
    check_array(data, data_rev)
}

function check_array<T>(data: Array<T>, dat_r: Array<T>): void {
    let size = data.length - 1
    arktest.assertEQ(data.length, dat_r.length, "reverse(): bad array len")

    for (let i: int = 0; i <= size; i++) {
        arktest.assertEQ(data[i], dat_r[size - i], "reverse(): unexpected element: i: " + i)
    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite("Array.reverse");
    testSuite.addTest("Arrays of int: reverse test", test_int_array);
    testSuite.addTest("Arrays of number: reverse test", test_number_array);
    testSuite.addTest("Arrays of short: reverse test", test_short_array);
    testSuite.addTest("Arrays of long: reverse test", test_long_array);
    testSuite.addTest("Arrays of float: reverse test", test_float_array);
    testSuite.addTest("Arrays of boolean: reverse test", test_boolean_array);
    testSuite.addTest("Arrays of char: reverse test", test_char_array);
    testSuite.addTest("Arrays of string: reverse test", test_string_array);
    testSuite.addTest("Arrays of union: reverse test", test_union_array);
    testSuite.addTest("Long len arrays: reverse test with safepoints", test_long_len_array_reverse);
    return testSuite.run()
}
