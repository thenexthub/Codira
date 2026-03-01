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

function main():int {
    let failures = 0;

    failures += test(checkBadRadix(0.0 as Double, -1), "0 with Negative Radix");

    failures += test(checkBadRadix(0.0 as Double, 1), "0 Radix 1");
    failures += test(checkBadRadix(0.0 as Double, 37), "0 Radix 37");

    failures += test(checkBadRadix(1.0 as Double, 1), "1 Radix 1");
    failures += test(checkBadRadix(1.0 as Double, 37), "1 Radix 37");

    failures += test(checkBadRadix(Double.MAX_VALUE as Double, 1), "MAX_VALUE Radix 1");
    failures += test(checkBadRadix(Double.MAX_VALUE as Double, 37), "MAX_VALUE Radix 37");


    failures += test(checkBadRadix(Double.MIN_VALUE as Double, 1), "MIN_VALUE Radix 1");
    failures += test(checkBadRadix(Double.MIN_VALUE as Double, 37), "MIN_VALUE Radix 37");

    failures += test(checkBadRadix(Double.POSITIVE_INFINITY as Double, 1), "POSITIVE_INFINITY Radix 1");
    failures += test(checkBadRadix(Double.POSITIVE_INFINITY as Double, 37), "POSITIVE_INFINITY Radix 37");

    failures += test(checkBadRadix(Double.NEGATIVE_INFINITY as Double, 1), "NEGATIVE_INFINITY Radix 1");
    failures += test(checkBadRadix(Double.NEGATIVE_INFINITY as Double, 37), "NEGATIVE_INFINITY Radix 37");

    failures += test(checkBadRadix(Double.NaN as Double, 1), "NaN Radix 1");
    failures += test(checkBadRadix(Double.NaN as Double, 37), "NaN Radix 37");

    return test(failures, "All tests run");
}

function test(result: int, message: String ): int {
    if(result == 0) {
      console.println("PASSED: " + message)
      return 0;
    }
    console.println("FAILED: " + message);
    return 1;
}

function checkBadRadix(val: Double, radix: int):int {

    let sv: String = "";
    try {
        sv = val.toString(radix);
    } catch(e) {
        if (e instanceof ArgumentOutOfRangeError) {
            return 0;
        } else {
            console.println("Unknown exeption catched " + e)
        }
    }

    console.print("String: ");
    console.print(sv);
    console.println();
    return 1;
}
