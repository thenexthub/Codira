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
    failures += test(checkGoodRadix(Double.POSITIVE_INFINITY, 2, "Infinity"), "Check Positive Infinity with radix 2");
    failures += test(checkGoodRadix(Double.NEGATIVE_INFINITY, 2, "-Infinity"), "Check Negatibe Infinity with radix 2");
    failures += test(checkGoodRadix(Double.NaN, 2, "NaN"), "Check Nan with radix 2");

    failures += test(checkGoodRadix(Double.POSITIVE_INFINITY, 8, "Infinity"), "Check Positive Infinity with radix 8");
    failures += test(checkGoodRadix(Double.NEGATIVE_INFINITY, 8, "-Infinity"), "Check Negatibe Infinity with radix 8");
    failures += test(checkGoodRadix(Double.NaN, 8, "NaN"), "Check Nan with radix 8");

    failures += test(checkGoodRadix(Double.POSITIVE_INFINITY, 10, "Infinity"), "Check Positive Infinity with radix 10");
    failures += test(checkGoodRadix(Double.NEGATIVE_INFINITY, 10, "-Infinity"), "Check Negatibe Infinity with radix 10");
    failures += test(checkGoodRadix(Double.NaN, 10, "NaN"), "Check Nan with radix 10");

    failures += test(checkGoodRadix(Double.POSITIVE_INFINITY, 16, "Infinity"), "Check Positive Infinity with radix 16");
    failures += test(checkGoodRadix(Double.NEGATIVE_INFINITY, 16, "-Infinity"), "Check Negatibe Infinity with radix 16");
    failures += test(checkGoodRadix(Double.NaN, 16, "NaN"), "Check Nan with radix 10");

    return test(failures, "All tests run");

}

function test(result: int, message: String ): int {
    if(result == 0) {
      return 0;      
    }
    console.println("FAILED: " + message);
    return 1;
}

function checkGoodRadix(val: Double, radix: int, expected: String):int {
    
    let sv: String;
    try {
        sv = val.toString(radix);
    } catch(e) {
        if (e instanceof ArgumentOutOfRangeError) {
            return 1;
        }
        sv = "ERROR"
    }
    
    if (sv == expected) {
        return 0;
    }

    return 1;
}
