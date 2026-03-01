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

function main(): int {
    let roundTestsuite = new arktest.ArkTestsuite("roundTest")
    roundTestsuite.addTest("round_normal", roundNormals)
    roundTestsuite.addTest("round_boundary", roundBoundaries)
    return roundTestsuite.run()
}

function roundNormals() {
    arktest.assertEQ(Math.round(0.0), 0.0)
    arktest.assertEQ(Math.round(1.0), 1.0)
    arktest.assertEQ(Math.round(-1.0), -1.0)
    arktest.assertEQ(Math.round(1.3), 1.0)
    arktest.assertEQ(Math.round(-1.3), -1.0)
    arktest.assertEQ(Math.round(1.5), 2)
    arktest.assertEQ(Math.round(-1.5), -1)
    arktest.assertEQ(Math.round(1.6), 2.0)
    arktest.assertEQ(Math.round(-1.6), -2.0)
    arktest.assertEQ(Math.round(0.1 + 0.4), 1.0)
    arktest.assertEQ(Math.round(-0.1 - 0.4), -0.0)
    arktest.assertEQ(Math.round(0.3), 0.0)
    arktest.assertEQ(Math.round(-0.3), -0.0)
}

function roundBoundaries() {
    arktest.assertEQ(Math.round(Double.POSITIVE_INFINITY), Double.POSITIVE_INFINITY, "round(Double.POSITIVE_INFINITY)")
    arktest.assertEQ(Math.round(Double.NEGATIVE_INFINITY), Double.NEGATIVE_INFINITY, "round(Double.NEGATIVE_INFINITY)")
    arktest.assertEQ(Double.toString(Math.round(Double.NaN)), Double.toString(Double.NaN), "Double.toString(Math.round(Double.NaN))")
    arktest.assertEQ(Double.toString(Math.round(-Double.NaN)), Double.toString(-Double.NaN), "Double.toString(Math.round(-Double.NaN))")
    arktest.assertEQ(Math.round(3.5000000000000000000000000001), 4.0, "round(3.5000000000000000000000000001)")       //Exceeds double storage precision, actual storage is 3.5
    arktest.assertEQ(Math.round(-3.5000000000000000000000000001), -3.0, "round(-3.5000000000000000000000000001)")    //Exceeds double storage precision, actual storage is -3.5
    arktest.assertEQ(Math.round(3.500000000000001), 4.0, "round(3.500000000000001)")
    arktest.assertEQ(Math.round(-3.500000000000001), -4.0, "round(-3.500000000000001)")
    arktest.assertEQ(Math.round(5.4999999999999999999999999999), 6.0, "round(5.4999999999999999999999999999)")       //Exceeds double storage precision, actual storage is 5.5
    arktest.assertEQ(Math.round(-5.4999999999999999999999999999), -5.0, "round(-5.4999999999999999999999999999)")    //Exceeds double storage precision, actual storage is -5.5
    arktest.assertEQ(Math.round(5.499999999999), 5.0, "round(5.499999999999)")
    arktest.assertEQ(Math.round(-5.499999999999), -5.0, "round(-5.499999999999)")
    arktest.assertEQ(Math.round(0.5 - Number.EPSILON / 4), 0.0, "round(0.5 - Number.EPSILON / 4)")
    arktest.assertEQ(Math.round(0.5 + Number.EPSILON / 4), 1.0, "round(0.5 + Number.EPSILON / 4)")
    arktest.assertEQ(Math.round(-0.5 - Number.EPSILON / 4), 0.0, "round(-0.5 - Number.EPSILON / 4)")
    arktest.assertEQ(Math.round(-0.5 + Number.EPSILON / 4), 0.0, "round(-0.5 + Number.EPSILON / 4)")
}

function Math_round_zero_with_sign () {
    const negativeZero = Math.round(-0.1);
    const positiveZero = Math.round(0.1);
    arktest.assertEQ(1 / negativeZero, Double.NEGATIVE_INFINITY, "1 / Math.round(-0.1) => -Infinity");
    arktest.assertEQ(1 / positiveZero, Double.POSITIVE_INFINITY, "1 / Math.round(0.1) => Infinity");
}