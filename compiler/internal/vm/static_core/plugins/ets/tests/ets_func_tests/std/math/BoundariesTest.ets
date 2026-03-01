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

import { PI } from "std/math/consts";

const nan = Double.NaN;
const posInf = Double.POSITIVE_INFINITY;
const negInf = Double.NEGATIVE_INFINITY;

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name);
        return 0;
    }
    console.println("FAILED: " + name);
    return 1;
}

function test_abs(): boolean {
    return Double.isNaN(abs(nan))
        && Double.compare(abs(Double.MAX_VALUE), Double.MAX_VALUE)
        && Double.compare(abs(-Double.MAX_VALUE), Double.MAX_VALUE)
        && !Double.isFinite(abs(posInf))
        && !Double.isFinite(abs(negInf));
}

function test_sin(): boolean {
    return Double.compare(sin(PI), 0)
        && Double.compare(sin(PI/6), 0.5)
        && Double.isNaN(sin(nan))
        && Double.isNaN(sin(posInf))
        && Double.isNaN(sin(negInf));
}

function test_cos(): boolean {
    return Double.compare(cos(2*PI), 1)
        && Double.compare(cos(PI/3), 0.5)
        && Double.isNaN(cos(nan))
        && Double.isNaN(cos(posInf))
        && Double.isNaN(cos(negInf));
}

function test_acosh(): boolean {
    return Double.isNaN(acosh(nan))
        && !Double.isFinite(acosh(posInf))
        && !Double.isFinite(acosh(negInf));
}

function test_acos(): boolean {
    return Double.isNaN(acos(nan))
        && Double.isNaN(acos(posInf))
        && Double.isNaN(acos(negInf));
}

function test_asin(): boolean {
    return Double.isNaN(asin(nan))
        && Double.isNaN(asin(posInf))
        && Double.isNaN(asin(negInf));
}

function test_atan(): boolean {
    return Double.isNaN(atan(nan))
        && Double.compare(atan(posInf), PI/2)
        && Double.compare(atan(negInf), -PI/2)
        && (atan(0) == 0)
        && Float.compare(atan(1), (PI/4).toFloat());
}

function test_signbit(): boolean {
    return signbit(nan)
        && !signbit(posInf)
        && signbit(negInf);
}

function test_round(): boolean {
    return Double.isNaN(round(nan))
        && !Double.isFinite(round(posInf))
        && !Double.isFinite(round(negInf));
}

function test_floor(): boolean {
    return Double.isNaN(floor(nan))
        && !Double.isFinite(floor(negInf))
        && !Double.isFinite(floor(posInf));
}

function test_ceil(): boolean {
    return Double.isNaN(ceil(nan))
        && !Double.isFinite(ceil(negInf))
        && !Double.isFinite(ceil(posInf));
}

function test_power(): boolean {
    return Double.isNaN(power(nan, posInf))
        && !Double.isFinite(power(posInf, posInf));
}

function test_mod(): boolean {
    return Double.isNaN(mod(nan, posInf))
        && !Double.isFinite(mod(posInf, posInf));
}

function test_log(): boolean {
    return Double.isNaN(log(nan))
        && Double.isNaN(log(negInf))
        && !Double.isFinite(log(posInf));
}

function test_log2(): boolean {
    return Double.isNaN(log2(nan))
        && Double.isNaN(log2(negInf))
        && !Double.isFinite(log2(posInf));
}

function test_log10(): boolean {
    return Double.isNaN(log10(nan))
        && Double.isNaN(log10(negInf))
        && !Double.isFinite(log10(posInf));
}

function test_exp(): boolean {
    return Double.isNaN(exp(nan))
        && !Double.isFinite(exp(posInf))
        && (exp(negInf) == 0);
}

function test_clz32(): boolean {
    return (clz32(-0x1) == 0)
        && (clz32(0x0000FFFF) == 16)
        && (clz32(0x0) == 32);
}

function test_clz64(): boolean {
    return (clz64(-0x1) == 0)
        && (clz64(0x0000FFFFFFFFFFFF) == 16)
        && (clz64(0x0) == 64);
}

function main(): int {
    let failures: int = 0;
    failures += test(test_abs(), "Test abs()");
    failures += test(test_sin(), "Test sin()");
    failures += test(test_cos(), "Test cos()");
    failures += test(test_acosh(), "Test acosh()");
    failures += test(test_acos(), "Test acos()");
    failures += test(test_asin(), "Test asin()");
    failures += test(test_atan(), "Test atan()");
    // TODO: curently fails on arm64
    // failures += test(test_signbit(), "Test signbit()");
    failures += test(test_round(), "Test round()");
    failures += test(test_ceil(), "Test ceil()");
    failures += test(test_floor(), "Test floor()");
    failures += test(test_power(), "Test power()");
    failures += test(test_mod(), "Test mod()");
    failures += test(test_log(), "Test log()");
    failures += test(test_log2(), "Test log2()");
    failures += test(test_log10(), "Test log10()");
    failures += test(test_clz32(), "Test clz32()");
    failures += test(test_clz64(), "Test clz64()");
    failures += test(test_exp(), "Test exp()");
    if (failures == 0) {
        console.println("PASSED: All tests run");
    } else {
        console.println("FAILED: All tests run");
    }
    return failures;
}
