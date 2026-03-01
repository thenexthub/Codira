/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package std.core;

import {PI, E, LN10, LN2, LOG2E, LOG10E, SQRT1_2, SQRT2} from "std/math/consts";

/**
 * The Math class contains static properties and methods for mathematical constants and functions.
 */
export class Math {
    // TODO(dmitrys99) Reuse constants from "std/math"
    /**
     * PI as double value
     */
    public static readonly PI: double = PI;

    /**
     * Euler number as double value
     */
    public static readonly E: double = E;

    /**
     * Natural logarithm of 10 as double value
     */
    public static readonly LN10: double = LN10;

    /**
     * Natural logarithm of 2 as double value
     */
    public static readonly LN2: double = LN2;

    /**
     * Logarithm base 2 of Euler number as double value
     */
    public static readonly LOG2E: double = LOG2E;

    /**
     * Logarithm base 10 of Euler number as double value
     */
    public static readonly LOG10E: double = LOG10E;

    /**
     * Square root of 1/2 as double value
     */
    public static readonly SQRT1_2: double = SQRT1_2;

    /**
     * Square root of 2 as double value
     */
    public static readonly SQRT2: double = SQRT2;

        /**
     * Absolute value
     *
     * @param v - Some number value
     *
     * @returns Absolute value of `v`
     *
     * @example
     * ```
     * Math.abs(-1) = 1
     * Math.abs(1) = 1
     * Math.abs(0) = 0
     * ```
     */
    public static abs(x: byte): byte {
        return abs(x);
    }

    /**
     * Absolute value
     *
     * @param v - Some number value
     *
     * @returns Absolute value of `v`
     *
     * @example
     * ```
     * Math.abs(-1) = 1
     * Math.abs(1) = 1
     * Math.abs(0) = 0
     * ```
     */
    public static abs(x: short): short {
        return abs(x);
    }

    /**
     * Absolute value
     *
     * @param v - Some number value
     *
     * @returns Absolute value of `v`
     *
     * @example
     * ```
     * Math.abs(-1) = 1
     * Math.abs(1) = 1
     * Math.abs(0) = 0
     * ```
     */
    public static abs(x: int): int {
        return abs(x);
    }

    /**
     * Absolute value
     *
     * @param v - Some number value
     *
     * @returns Absolute value of `v`
     *
     * @example
     * ```
     * Math.abs(-1) = 1
     * Math.abs(1) = 1
     * Math.abs(0) = 0
     * ```
     */
    public static abs(x: long): long {
        return abs(x);
    }

    /**
     * Absolute value
     *
     * @param v - Some number value
     *
     * @returns Absolute value of `v`
     *
     * @example
     * ```
     * Math.abs(-1) = 1
     * Math.abs(1) = 1
     * Math.abs(0) = 0
     * ```
     */
    public static abs(x: float): float {
        return abs(x);
    }

    /**
     * Absolute value
     *
     * @param v - Some number value
     *
     * @returns Absolute value of `v`
     *
     * @example
     * ```
     * Math.abs(doubleNaN) = NaN
     * Math.abs(doubleInf) = Inf
     * Math.abs(doubleNegInf) = Inf
     * ```
     */
    public static abs(x: double): double {
        return abs(x);
    }

    /**
     * Arccosine of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Arccosine of angle `v`
     *
     * @remark
     * Implemented as native function,  @see `acos()` intrinsic.
     *
     * @example
     * ```
     * acos(doubleNaN) = -nan
     * acos(doubleInf) = nan
     * acos(doubleNegInf) = nan
     */
    public static acos(x: double): double {
        return acos(x);
    }

    /**
     * Arccosine of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Arccosine of angle `v`
     *
     * @remark
     * Implemented as native function,  @see `acos()` intrinsic.
     *
     * @example
     * ```
     * acos(doubleNaN) = -nan
     * acos(doubleInf) = nan
     * acos(doubleNegInf) = nan
     */
    public static acos(x: float): float {
        return acos(x);
    }

    /**
     * Hyperbolic arccosine of an angle `v`
     *
     * @param v angle in radians
     *
     * @returns Hyperbolic arccosine of angle `v`
     *
     * @example
     * ```
     * Math.acosh(doubleNaN) = -NaN
     * Math.acosh(doubleInf) = Inf
     * Math.acosh(doubleNegInf) = -NaN
     * ```
     */
    public static acosh(x: double): double {
        return acosh(x);
    }

    /**
     * Hyperbolic arccosine of an angle `v`
     *
     * @param v angle in radians
     *
     * @returns Hyperbolic arccosine of angle `v`
     *
     * @example
     * ```
     * Math.acosh(doubleNaN) = -NaN
     * Math.acosh(doubleInf) = Inf
     * Math.acosh(doubleNegInf) = -NaN
     * ```
     */
    public static acosh(x: float): float {
        return acosh(x);
    }

    /**
     * Arcsine of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Arcsine of angle `v`
     */
    public static asin(x: double): double {
        return asin(x);
    }

    /**
     * Arcsine of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Arcsine of angle `v`
     */
    public static asin(x: float): float {
        return asin(x);
    }

    /**
     * Hyperbolic arcsine of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Hyperbolic arcsine of angle `v`
     *
     * @example
     * ```
     * Math.asinh(doubleNaN) = -NaN
     * Math.asinh(doubleInf) = Inf
     * Math.asinh(doubleNegInf) = -Inf
     * ```
     */
    public static asinh(x: double): double {
        return asinh(x);
    }

    /**
     * Hyperbolic arcsine of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Hyperbolic arcsine of angle `v`
     *
     * @example
     * ```
     * Math.asinh(doubleNaN) = -NaN
     * Math.asinh(doubleInf) = Inf
     * Math.asinh(doubleNegInf) = -Inf
     * ```
     */
    public static asinh(x: float): float {
        return asinh(x);
    }

    /**
     * Arctangent of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Arctangent of angle `v`
     *
     * @example
     * ```
     * Math.atan(doubleNaN) = -NaN
     * Math.atan(doubleInf) = 1.5708
     * Math.atan(doubleNegInf) = -1.5708
     * ```
     */
    public static atan(x: double): double {
        return atan(x);
    }

    /**
     * Arctangent of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Arctangent of angle `v`
     *
     * @example
     * ```
     * Math.atan(doubleNaN) = -NaN
     * Math.atan(doubleInf) = 1.5708
     * Math.atan(doubleNegInf) = -1.5708
     * ```
     */
    public static atan(x: float): float {
        return atan(x);
    }

    /**
     * Method returns the angle in the plane (in radians) between the positive x-axis and the ray from (0, 0) to the point (x, y), for Math.atan2(y, x).
     *
     * @returns The angle in radians (between -π and π, inclusive) between the positive x-axis and the ray from (0, 0) to the point (x, y).
     *
     * @remark
     * The atan2() method measures the counterclockwise angle θ, in radians, between the positive x-axis and the point (x, y).
     * Note that the arguments to this function pass the y-coordinate first and the x-coordinate second.
     */
    public static atan2(y: double, x: double): double {
        return atan2(y, x);
    }

    /**
     * Method returns the angle in the plane (in radians) between the positive x-axis and the ray from (0, 0) to the point (x, y), for Math.atan2(y, x).
     *
     * @returns The angle in radians (between -π and π, inclusive) between the positive x-axis and the ray from (0, 0) to the point (x, y).
     *
     * @remark
     * The atan2() method measures the counterclockwise angle θ, in radians, between the positive x-axis and the point (x, y).
     * Note that the arguments to this function pass the y-coordinate first and the x-coordinate second.
     */
    public static atan2(y: float, x: float): float {
        return atan2(y, x);
    }

    /**
     * Hyperbolic arctangent of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Hyperbolic arctangent of angle `v`
     *
     * @example
     * ```
     * Math.atanh(doubleNaN) = -NaN
     * Math.atanh(doubleInf) = -NaN
     * Math.atanh(doubleNegInf) = -NaN
     * ```
     */
    public static atanh(x: double): double {
        return atanh(x);
    }

    /**
     * Hyperbolic arctangent of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Hyperbolic arctangent of angle `v`
     *
     * @example
     * ```
     * Math.atanh(doubleNaN) = -NaN
     * Math.atanh(doubleInf) = -NaN
     * Math.atanh(doubleNegInf) = -NaN
     * ```
     */
    public static atanh(x: float): float {
        return atanh(x);
    }

    /**
     * Cube root of a number.
     *
     * @param x arbitrary number
     *
     * @remark
     * Math.𝚌brt(𝚡) = ∛x = the unique y such that y³ = x.
     */
    public static cbrt(x: double): double {
        if (isNaN(x) || x == Infinity || x == -Infinity) return x;
        return cbrt(x);
    }

    /**
     * Cube root of a number.
     *
     * @param x arbitrary number
     *
     * @remark
     * Math.𝚌brt(𝚡) = ∛x = the unique y such that y³ = x.
     */
    public static cbrt(x: float): float {
        if (isNaN(x) || x == Float.POSITIVE_INFINITY || x == Float.NEGATIVE_INFINITY) return x;
        return cbrt(x);
    }

    /**
     * Smallest integer greater or equal to `v`
     *
     * @param v arbitrary number
     *
     * @returns Smallest integer greater or equal to v
     *
     * @example
     * ```
     * Math.ceil(doubleNaN) = -nan
     * Math.ceil(doubleInf) = inf
     * Math.ceil(doubleNegInf) = -inf
     * ```
     */
    public static ceil(x: double): double {
        return ceil(x);
    }

    /**
     * Leading zero bits count in 32-bit representation of `v`
     *
     * @param v 32-bit integer
     *
     * @returns Number of leading zero bits count in bit representation of `v`
     *
     * @example
     * ```
     * Math.clz32(0xFFFFFFFF) == 0
     * Math.clz32(0x0000FFFF) == 16
     * Math.clz32(0x0) == 32
     * ```
     */
    public static clz32(x: Int): Int {
        return new Int(clz32(x.toInt()));
    }

    /**
     * Leading zero bits count in 32-bit representation of `v`
     *
     * @param v 32-bit integer
     *
     * @returns Number of leading zero bits count in bit representation of `v`
     *
     * @example
     * ```
     * Math.clz32(0xFFFFFFFF) == 0
     * Math.clz32(0x0000FFFF) == 16
     * Math.clz32(0x0) == 32
     * ```
     */
    public static clz32(value: double): double {
        return clz32Double(value);
    }

    /**
     * Rounding-up argument value to the closest integral power of 2
     * Thus, Math.ceilPow2(n) is the least power of 2 that is >= n
     *
     * @param n 32-bit integer
     *
     * @returns Argument value rounded-up to the closest integral power of 2
     * @example
     * ```
     * Math.ceilPow2(1334) == 2048
     * Math.ceilPow2(0) == 0
     * Math.ceilPow2(1) == 1
     * Math.ceilPow2(2) == 2
     * ```
     */
    public static ceilPow2(n: int): int {
        if (n < 0) {
            throw new RangeError("Round-up to the Next Power of 2 function is undefined for negative arguments")
        }

        let r = n - 1

        r |= r >> 1
        r |= r >> 2
        r |= r >> 4
        r |= r >> 8
        r |= r >> 16

        return r + 1
    }

    /**
     * Cosine of `x`
     *
     * @param x Angle in radians
     *
     * @returns Cosine of angle in radians
     *
     * @example
     * ```
     * Math.cos(doubleNaN) = -NaN
     * Math.cos(doubleInf) = -NaN
     * Math.cos(doubleNegInf) = -NaN
     * ```
     */
    public static cos(x: double): double {
        return cos(x);
    }

    /**
     * Hyperbolic cosine of an angle `x`
     *
     * @param x angle in radians
     *
     * @returns Hyperbolic cosine of angle `x`
     *
     * @example
     * ```
     * Math.cosh(doubleNaN) = -NaN
     * Math.cosh(doubleInf) = Inf
     * Math.cosh(doubleNegInf) = Inf
     * ```
     */
    public static cosh(x: double): double {
        return cosh(x);
    }

    /**
     * Hyperbolic cosine of an angle `x`
     *
     * @param x angle in radians
     *
     * @returns Hyperbolic cosine of angle `x`
     *
     * @example
     * ```
     * Math.cosh(doubleNaN) = -NaN
     * Math.cosh(doubleInf) = Inf
     * Math.cosh(doubleNegInf) = Inf
     * ```
     */
    public static cosh(x: float): float {
        return cosh(x);
    }

    /**
     * e raised to power `x`
     *
     * @param x power value
     *
     * @returns {@link Math.E} raised to the power of a number `x`.
     *
     * @example
     * ```
     * Math.exp(doubleNaN) = -NaN
     * Math.exp(doubleInf) = Inf
     * Math.exp(doubleNegInf) = 0
     * ```
     */
    public static exp(x: float): float {
        return exp(x);
    }

    /**
     * e raised to power `x`
     *
     * @param x power value
     *
     * @returns {@link Math.E} raised to the power of a number `x`.
     *
     * @example
     * ```
     * Math.exp(doubleNaN) = -NaN
     * Math.exp(doubleInf) = Inf
     * Math.exp(doubleNegInf) = 0
     * ```
     */
    public static exp(x: double): double {
        return exp(x);
    }

    /**
     * (`e` raised to power `v`) - 1
     *
     * @param x power value
     *
     * @returns {@link Math.E} raised to the power of a number `x`, subtracted by `1`.
     *
     * @example
     * ```
     * Math.expm1(doubleNaN) = -NaN
     * Math.expm1(doubleInf) = Inf
     * Math.expm1(doubleNegInf) = -1
     * ```
     */
    public static expm1(x: float): float {
        return expm1(x);
    }

    /**
     * (`e` raised to power `v`) - 1
     *
     * @param x power value
     *
     * @returns {@link Math.E} raised to the power of a number `x`, subtracted by `1`.
     *
     * @example
     * ```
     * Math.expm1(doubleNaN) = -NaN
     * Math.expm1(doubleInf) = Inf
     * Math.expm1(doubleNegInf) = -1
     * ```
     */
    public static expm1(x: double): double {
        return expm1(x);
    }

    /**
     * Largest integer less or equal to `v`
     *
     * @param v arbitrary number
     *
     * @returns Largest integer less or equal to `v`
     *
     * @example
     * ```
     * Math.floor(doubleNaN) = -NaN
     * Math.floor(doubleInf) = Inf
     * Math.floor(doubleNegInf) = -Inf
     * ```
     */
    public static floor(x: double): double {
        return floor(x);
    }

    /**
    * "fround" returns nearest 32-bit single fp representation of a number
    *  in a 64-bit form
    *
    *  Math.fround(1.337) == 1.337 // false, result would be 1.3370000123977661
    *  Math.fround(1.5) == 1.5 // true
    *  Math.fround(-5.05) == -5.05 //false, result would be -5.050000190734863
    *  Math.fround(Infinity) // Infinity
    *  Math.fround(NaN) // NaN
    */
    public static fround(x: double): double {
        return fround(x);
    }

    /**
     * Square root of the sum of squares of `v` and `u`
     *
     * @param u arbitrary number
     * @param v arbitrary number
     *
     * @returns The square root of the sum of squares of its arguments
     */
    public static hypot(...values: double[]): double {
        let max: double = 0;
        let hasNaN = false;

        for (const value of values) {
            if (value == Infinity || value == -Infinity) {
                return Infinity;
            }
            if (isNaN(value)) {
                hasNaN = true;
            }
            max = Math.max(max, Math.abs(value));
        }
        if (hasNaN) return NaN;
        if (max == 0) return 0;
        let sum: double = 0.0;
        for (const value of values) {
            let normalized = value / max;
            sum += normalized * normalized;
        }
        return max * Math.sqrt(sum);
    }

    /**
     *  Method returns the result of the C-like 32-bit manipulation of the two parameters
     *
     *  @returns (a * b) % 2 ** 32
     *
     *  Math.imul(Infinity, 1) = 0
     *  Math.imul(NaN, 1) = 0
     *  Math.imul(2.5, 2.5) = 4
     *  Math.imul(-5, 5.05) = 25
     *
     */
    public static imul(a: double, b: double): double {
        return imul(a, b);
    }


    /**
     * Natural logarithm of `v`
     *
     * @param v: double - arbitrary number
     *
     * @returns The natural logarithm (base e) of x. If x is ±0, returns -Infinity. If x < 0, returns NaN.
     *
     * @example
     * ```
     * Math.log(doubleNaN) = -nan
     * Math.log(doubleInf) = inf
     * Math.log(doubleNegInf) = -nan
     * ```
     */
    public static log(x: float): float {
        return log(x);
    }

    /**
     * Natural logarithm of `v`
     *
     * @param v: double - arbitrary number
     *
     * @returns The natural logarithm (base e) of x. If x is ±0, returns -Infinity. If x < 0, returns NaN.
     *
     * @example
     * ```
     * Math.log(doubleNaN) = -nan
     * Math.log(doubleInf) = inf
     * Math.log(doubleNegInf) = -nan
     * ```
     */
    public static log(x: double): double {
        return log(x);
    }

    /**
     * Base 10 logarithm of `x`
     *
     * @param v arbitrary number
     *
     * @returns The base 10 logarithm of x. If x < 0, returns NaN.
     *
     * @example
     * ```
     * Math.log10(doubleNaN) = -NaN
     * Math.log10(doubleInf) = Inf
     * Math.log10(doubleNegInf) = -NaN
     * ```
     */
    public static log10(x: double): double {
        return log10(x);
    }

    /**
     * Base 10 logarithm of `x`
     *
     * @param v arbitrary number
     *
     * @returns The base 10 logarithm of x. If x < 0, returns NaN.
     *
     * @example
     * ```
     * Math.log10(doubleNaN) = -NaN
     * Math.log10(doubleInf) = Inf
     * Math.log10(doubleNegInf) = -NaN
     * ```
     */
    public static log10(x: float): float {
        return log10(x);
    }

    /**
     * Natural logarithm of (1 + `x`)
     *
     * @param x arbitrary number
     *
     * @returns The natural logarithm (base e) of x + 1. If x is -1, returns -Infinity. If x < -1, returns NaN.
     *
     * @example
     * ```
     * Math.log1p(doubleNaN) = -NaN
     * Math.log1p(doubleInf) = Inf
     * Math.log1p(doubleNegInf) = -NaN
     * ```
     */
    public static log1p(x: double): double {
        return log1p(x);
    }

    /**
     * Natural logarithm of (1 + `x`)
     *
     * @param x arbitrary number
     *
     * @returns The natural logarithm (base e) of x + 1. If x is -1, returns -Infinity. If x < -1, returns NaN.
     *
     * @example
     * ```
     * Math.log1p(doubleNaN) = -NaN
     * Math.log1p(doubleInf) = Inf
     * Math.log1p(doubleNegInf) = -NaN
     * ```
     */
    public static log1p(x: float): float {
        return log1p(x);
    }

    /**
     * Base 2 logarithm of `x`
     *
     * @param x arbitrary number
     *
     * @returns The base 2 logarithm of x. If x < 0, returns NaN.
     *
     * @example
     * ```
     * Math.log2(doubleNaN) = -nan
     * Math.log2(doubleInf) = inf
     * Math.log2(doubleNegInf) = -nan
     * ```
     */
    public static log2(x: double): double {
        return log2(x);
    }

    /**
     * Base 2 logarithm of `x`
     *
     * @param x arbitrary number
     *
     * @returns The base 2 logarithm of x. If x < 0, returns NaN.
     *
     * @example
     * ```
     * Math.log2(doubleNaN) = -nan
     * Math.log2(doubleInf) = inf
     * Math.log2(doubleNegInf) = -nan
     * ```
     */
    public static log2(x: float): float {
        return log2(x);
    }

    /**
     * Largest value of `u` and `v`
     *
     * @param u arbitrary number (of type `Int`)
     * @param v arbitrary number (of type `Int`)
     *
     * @returns Largest value of `u` and `v`
     */
    public static max(): double {
        return -Infinity;
    }

    public static max(val: double): double {
        return val;
    }

    public static max(val1: double, val2: double): double {
        if (isNaN(val1) || isNaN(val2)) {
            return NaN
        }
        if (val1 > val2) {
            return val1
        }
        return val2;
    }

    public static max(...values: double[]): double {
        if (values.length == 0) {
            return -Infinity;
        }
        let max = values[0];
        for (let i = 1; i < values.length; i++) {
            if (isNaN(values[i])) return NaN;
            if (values[i] > max) {
                max = values[i];
            }
        }
        return max;
    }

    /**
     * Smallest value of `u` and `v`
     *
     * @param u arbitrary number (of type `Int`)
     * @param v arbitrary number (of type `Int`)
     *
     * @returns Smallest value of `u` and `v`
     */
    public static min(): double {
        return Infinity;
    }

    public static min(val: double): double {
        return val;
    }

    public static min(val1: double, val2: double): double {
        if (isNaN(val1) || isNaN(val2)) {
            return NaN
        }
        if (val1 < val2) {
            return val1
        }
        return val2;
    }

    public static min(...values: double[]): double {
        if (values.length == 0) {
            return Infinity;
        }
        let min = values[0];
        for (let i = 1; i < values.length; i++) {
            if (isNaN(values[i])) return NaN;
            if (values[i] < min) {
                min = values[i];
            }
        }
        return min;
    }

    /**
     * `u` raised to power of `v`
     *
     * @param u: base value
     * @param v: power value
     *
     * @returns The value of a base `u` raised to a power `v`
     *
     * @example
     * ```
     * Math.pow(doubleNaN, doubleInf) = -NaN
     * Math.pow(doubleInf, doubleInf) = Inf
     * ```
     */
    public static pow(u: double, v: double): double {
        return power(u, v);
    }

    /**
     * Pseudo-random number in the range [0.0, 1.0)
     *
     * @returns A floating-point, pseudo-random number that's greater than or equal to 0 and less than 1,
     * with approximately uniform distribution over that range — which you can then scale to your desired range.
     * Initial seed to the random number generator algorithm can be given using {@link seedRandom()} function.
     *
     */
    public static native random(): double

    /**
     * `v` rounded to nearest integer
     *
     * @param v arbitrary value
     *
     * @returns `v` rounded to nearest integer
     *
     * @example
     * ```
     * Math.round(doubleNaN) = -NaN
     * Math.round(doubleInf) = Inf
     * Math.round(doubleNegInf) = -Inf
     * ```
     */
    public static round(x: double): double {
        return round(x);
    }

    /**
     * @param x arbitrary number
     *
     * @returns -1 if `x` is negative, 1 if `x` is positive, 0 if `x` is close to zero (epsilon is 1e-13)
     *
     */
    public static sign(x: double): double {
        if (isNaN(x)) {
            return x;
        }
        return sign(x).toDouble();
    }

    /**
     * Sine of `v`
     *
     * @param v angle in radians
     *
     * @returns Sine of angle in radians
     *
     * @example
     * ```
     * Math.sin(doubleNaN) = -nan
     * Math.sin(doubleInf) = -nan
     * Math.sin(doubleNegInf) = -nan
     * ```
     */
    public static sin(x: double): double {
        return sin(x);
    }

    /**
     * Hyperbolic sine of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Hyperbolic sine of angle `v`
     *
     * @example
     * ```
     * Math.sinh(doubleNaN) = -nan
     * Math.sinh(doubleInf) = inf
     * Math.sinh(doubleNegInf) = -inf
     * ```
     */
    public static sinh(x: double): double {
        return sinh(x);
    }

    /**
     * Hyperbolic sine of angle `v`
     *
     * @param v angle in radians
     *
     * @returns Hyperbolic sine of angle `v`
     *
     * @example
     * ```
     * Math.sinh(doubleNaN) = -nan
     * Math.sinh(doubleInf) = inf
     * Math.sinh(doubleNegInf) = -inf
     * ```
     */
    public static sinh(x: float): float {
        return sinh(x);
    }

    /**
     * Square root of `x`
     *
     * @param x number greater or equal to zero.
     *
     * @returns The square root of `x`, a non-negative number.
     * If `x` is less than zero, returns `NaN`.
     *
     * @example
     * ```
     * Math.sqrt(doubleNaN) = -nan
     * Math.sqrt(doubleInf) = inf
     * Math.sqrt(doubleNegInf) = -nan
     * ```
     */
    public static sqrt(x: double): double {
        return sqrt(x);
    }

    /**
     * Tangent of angle `x`
     *
     * @param x angle in radians
     *
     * @returns Tangent of angle `x`
     *
     * @example
     * ```
     * Math.tan(doubleNaN) = -nan
     * Math.tan(doubleInf) = -nan
     * Math.tan(doubleNegInf) = -nan
     * ```
     */
    public static tan(x: double): double {
        return tan(x);
    }

    /**
     * Hyperbolic arctangent of angle `x`
     *
     * @param x angle in radians
     *
     * @returns Hyperbolic arctangent of angle `x`
     *
     * @example
     * ```
     * Math.atanh(doubleNaN) = -nan
     * Math.atanh(doubleInf) = -nan
     * Math.atanh(doubleNegInf) = -nan
     * ```
     */
    public static tanh(x: double): double {
        if (isNaN(x)) return NaN;
        return tanh(x);
    }

    /**
     * Hyperbolic arctangent of angle `x`
     *
     * @param x angle in radians
     *
     * @returns Hyperbolic arctangent of angle `x`
     *
     * @example
     * ```
     * Math.atanh(doubleNaN) = -nan
     * Math.atanh(doubleInf) = -nan
     * Math.atanh(doubleNegInf) = -nan
     * ```
     */
    public static tanh(x: float): float {
        if (isNaN(x)) return Float.NaN;
        return tanh(x);
    }

    /**
     * Integer part of `v`
     *
     * @param v number to be truncated.
     *
     * @returns The integer part of a number by removing any fractional digits.
     *
     * @notes
     * If arg is +Infinity or -Infinity, it is returned unmodified.
     * If arg is +0 or -0, it is returned unmodified.
     * If arg is NaN, NaN is returned
     *
     */
    public static trunc(x: double): double {
        return trunc(x);
    }
}
