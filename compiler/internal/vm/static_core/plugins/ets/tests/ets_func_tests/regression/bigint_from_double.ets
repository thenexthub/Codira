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
desc: BigInt constructor from a string contained double value
tags: []
---*/

function test(s: string): String {
    try {
        return (new BigInt(s)).toString();
    } catch (e) {
        return e.toString();
    }
}

function positiveTest(s: string, expected: string): void {
    let result = test(s);
    arktest.assertEQ( result, expected,  result + " [" + s + "] != " + expected)
}

function negativeTest(s: string, expected: string): void {
    let result = test(s);
    arktest.assertTrue( result.match(expected) != null,  result + " [" + s + "] != " + expected)
}

function main(): void {
    positiveTest("1000e-2", "10");
    positiveTest("1.23e2", "123");
    positiveTest("0e-6", "0");
    positiveTest("0e+6", "0");
    positiveTest("0.00023e5", "23");
    positiveTest(".e+4", "0");
    positiveTest("+1e+1", "10");
    positiveTest("-0.24e3", "-240");
    positiveTest("-0.241e+5", "-24100");
    positiveTest("-125.0", "-125");
    positiveTest("-126000e-2", "-1260");

    negativeTest("1.234e2", "not an integer");
    negativeTest("1e-7", "not an integer");
    negativeTest("0.0035", "not an integer");
    negativeTest("0.00023e4", "not an integer");
    negativeTest("+1e-1", "not an integer");
    negativeTest("-35.42", "not an integer");
    negativeTest("-3700e-3", "not an integer");
    negativeTest("-0.039", "not an integer");
    negativeTest("-0.0485e+3", "not an integer");
    negativeTest("0.00023e--4", "is invalid");
    negativeTest("1e-e8", "is invalid");
    negativeTest("1e++e", "is invalid");
    negativeTest("58e10.0", "is invalid");
    negativeTest("5.99.73e0", "is invalid");
}
