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

/**
 * Represents boxed double value and related operations
 */
export final class Double extends Floating implements Comparable<Double> {
    private value: double;

    /**
     * Constructs a new Double instance with initial value zero
     *
     */
    public constructor() {
        this.value = 0.0;
    }

    /**
     * Constructs a new Double instance with provided initial value
     *
     * @param value the initial value
     *
     */
    public constructor(value: double) {
        this.value = value;
    }

    public constructor(value: BigInt) {
        this.value = value.doubleValue()
    }

    /**
     * Constructs a new Double instance from string
     *
     * @param value string that may contain a number
     */
    public constructor(value: String) {
        this.value = Double.numberFromString(value);
    }

    /**
     * Creates a new instance of a Double
     *
     * @returns A new Double instance
     */
    static $_invoke(): Number {
        return new Double();
    }

    /**
     * Creates a new instance of a Double
     *
     * @param value The value to be converted to a number. Can be a string, number, or BigInt (optional).
     *
     * @returns A new Double instance
     */
    static $_invoke(value: String | Number | BigInt | undefined | null): Number {
        if (value instanceof String) {
            return new Double(value as String);
        } else if (value instanceof Number) {
            return new Double(value as Number);
        } else if (value instanceof BigInt) {
            return new Double(value as BigInt);
        }
        if (value === null) {
            return new Double();
        }
        return new Double(NaN);
    }

    /**
     * Returns boxed representation of the primitive
     *
     * @param value value to box
     *
     * @returns boxed value
     * @tag arkts
     */
    public static valueOf(value: double): Double {
        // TODO(ivan-tyulyandin): caching is possible
        return new Double(value);
    }

    /**
     * Returns boxed representation of the primitive
     *
     * @param value value to box
     *
     * @returns boxed value
     */
    public valueOf(): number {
        return this.value as number;
    }

    /**
     * Minimal value that this type can have as a double
     * the workarond for libc's double literal parsing bug
     *
     */
    public static readonly MIN_VALUE: double = 4.9e-300 / 1.e+24;

    /**
     * Maximal value that this type can have as a double
     *
     */
    public static readonly MAX_VALUE: double = 1.7976931348623157e+308;

    /**
     * Maximal integer value that can be used as a double without loss of precision
     *
     */
    public static readonly MAX_SAFE_INTEGER: double = 9007199254740991;

    /**
     * Minimal integer value that can be used as a double without loss of precision
     *
     */
    public static readonly MIN_SAFE_INTEGER: double = -9007199254740991;

    /**
     * Size of this type in bits
     * @tag arkts
     */
    public static readonly BIT_SIZE: byte = 64;

    /**
     * Size of this type in bytes
     * @tag arkts
     */
    public static readonly BYTE_SIZE: byte = 8;

    /**
     * Represents the NaN value according to IEEE-754 specification
     *
     */
    public static readonly NaN: double = 0.0 / 0.0;

    /**
     * Represents the +Infinity value according to IEEE-754 specification
     *
     */
    public static readonly POSITIVE_INFINITY: double = 1.0 / 0.0;

    /**
     * Represents the -Infinity value according to IEEE-754 specification
     *
     */
    public static readonly NEGATIVE_INFINITY: double = -1.0 / 0.0;

    /**
     * Number of significant precision bits in this floating type
     * @tag arkts
     */
    public static readonly PRECISION: byte = 53;

    /**
     * Minimal possible difference between two double values.
     * For double (IEEE-754 binary64) it is 2^(-52) and its bit representation is 0x3cb0000000000000.
     * @tag arkts
     */
    public static readonly DELTA: double = Double.bitCastFromLong(0x3cb0000000000000);

    /**
     * Minimal possible difference between two double values
     *
     */
    public static readonly EPSILON: double = Double.DELTA;


    /**
     * Returns value of this instance
     *
     * @returns value as byte
     * @tag arkts
     */
    public override toByte(): byte {
        return this.value.toByte();
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns value as byte
     * @tag arkts
     */
    public override byteValue(): byte {
        return this.toByte();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as short
     * @tag arkts
     */
    public override toShort(): short {
        return this.value.toShort();
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns value as short
     * @tag arkts
     */
    public override shortValue(): short {
        return this.toShort();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as int
     * @tag arkts
     */
    public override toInt(): int {
        return this.value.toInt();
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns value as int
     * @tag arkts
     */
    public override intValue(): int {
        return this.toInt();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as long
     * @tag arkts
     */
    public override toLong(): long {
        return this.value.toLong();
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns value as long
     * @tag arkts
     */
    public override longValue(): long {
        return this.toLong();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as float
     * @tag arkts
     */
    public override toFloat(): float {
        return this.value.toFloat();
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns value as float
     * @tag arkts
     */
    public override floatValue(): float {
        return this.toFloat();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as double
     * @tag arkts
     */
    public override toDouble(): double {
        return this.value;
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns value as double
     * @tag arkts
     */
    public override doubleValue(): double {
        return this.toDouble();
    }

    /**
     * Returns the primitive as double value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static toDouble(value: double): double {
        return value;
    }

    /**
     * Returns the primitive as byte value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toByte(value: double): byte;

    /**
     * Returns the primitive as short value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toShort(value: double): short;

    /**
     * Returns the primitive as int value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toInt(value: double): int;

    /**
     * Returns the primitive as long value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toLong(value: double): long;

    /**
     * Returns the primitive as float value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toFloat(value: double): float;

    /**
     * Compares this instance to other Double object
     * The result is less than 0 if this instance lesser than provided object
     * 0 if they are equal
     * and greater than 0 otherwise.
     *
     * @param other Double object to compare with
     *
     * @returns result of the comparison, -1 if this instance is less than provided object, 0 if equal, 1 if greater
     * @tag arkts
     */
    public override compareTo(other: Double): int {
        if ((this.isNaN() && other.isNaN()) || (Math.abs(this.value - other) < Double.EPSILON)) {
            return 0;
        }
        if (this.isNaN() || this.value > other) {
            return 1;
        }
        return -1;
    }

    /**
     * toString(d: double, r: int): String -- returns a string representation of d by radix r
     * @tag arkts
     */
    public static native toString(d: double, r: int): String;

    /**
     * @tag arkts
     */
    public static toString(d: double): String {
        return Double.toString(d, 10);
    }

    /**
     * @tag arkts
     */
    public toString(r: int): String {
        return Double.toString(this.value, r);
    }

    public toString(d: Number): String {
        return Double.toString(this.value, d.toInt());
    }

    /**
     * Converts this object to a string
     *
     * @returns result of the conversion
     *
     */
    public override toString(): String {
        return Double.toString(this.value, 10);
    }

    /**
     * Without an argument method returns just toString value
     *
     * @returns result of the conversion
     *
     */
    public toLocaleString(): String {
        return new Intl.NumberFormat().format(this.value)
    }

    public toLocaleString(locales?: string | Intl.Locale | (string | Intl.Locale)[], options?: Intl.NumberFormatOptions): string {
        const formatter = new Intl.NumberFormat(Intl.intlLocalesToLanguageTags(locales), options)
        return formatter.format(this.value)
    }

    /**
     * compare(double, double) checks if two doubles are differs no more than by Double.DELTA
     *
     * @param lhs left-hand side double for comparison
     *
     * @param rhs right-hand side double for comparison
     *
     * @returns true if lhs and rhs are equal with respect to Double.DELTA
     * @tag arkts
     */
    public static compare(lhs: double, rhs: double): boolean {
        return (abs(lhs - rhs) <= Double.DELTA)
    }

    /**
    * Checks for equality this instance with provided object, treated as a Double
    *
    * @param other object to be checked against
    *
    * @returns true if provided object and this instance have same value, false otherwise
    * Returns false if type of provided object is not the same as this type
    * @tag arkts
    */
    public equals(other: Any): boolean {
        if (this === other) {
            return true
        }

        if (!(other instanceof Double)) {
            return false
        }

        return this.value == (other as Double).value
    }

    /**
     * isNaN(double) checks if double is NaN (not a number)
     *
     * @param v the double to test
     *
     * @returns true if the argument is NaN
     *
     */
    public static isNaN(v: double): boolean {
        // IEEE-754 feature
        return v != v;
    }

    /**
     * isNaN() checks if the underlying double is NaN (not a number)
     *
     * @returns true if the underlying double is NaN
     * @tag arkts
     */
    public isNaN(): boolean {
        return Double.isNaN(this.value);
    }

    /**
     * isFinite(double) checks if double is a floating point value (not a NaN or infinity)
     *
     * @param v the double to test
     *
     * @returns true if the argument is a floating point value
     *
     */
    public static isFinite(v: double): boolean {
        return !(Double.isNaN(v) || (v == Double.POSITIVE_INFINITY) || (v == Double.NEGATIVE_INFINITY));
    }

    /**
     * isFinite() checks if the underlying double is a floating point value (not a NaN or infinity)
     *
     * @returns true if the underlying double is a floating point value
     * @tag arkts
     */
    public isFinite(): boolean {
        return Double.isFinite(this.value);
    }

    /**
     * Checks if double is similar to an integer value
     *
     * @param v the double to test
     *
     * @returns true if the argument is similar to an integer value
     *
     */
    public static isInteger(v: double): boolean {
        // In the language % works as C fmod that differs with IEEE-754 % definition
        return (abs(v % (1.0 as double)) == 0.0);
    }

    /**
     * Checks if the underlying double is similar to an integer value
     *
     * @returns true if the underlying double is similar to an integer value
     * @tag arkts
     */
    public isInteger(): boolean {
        return Double.isInteger(this.value);
    }

    /**
     * Checks if double is a safe integer value
     *
     * @param v the double to test
     *
     * @returns true if the argument is integer ans less than MAX_SAFE_INTEGER
     *
     */
    public static isSafeInteger(v: double): boolean {
        return Double.isInteger(v) && (abs(v) <= Double.MAX_SAFE_INTEGER);
    }

    /**
     * Checks if double is a safe integer value
     *
     * @returns true if the underlying double is a safe integer
     * @tag arkts
     */
    public isSafeInteger(): boolean {
        return Double.isSafeInteger(this.value);
    }


    /**
     * Performs floating point addition of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the addition
     *
     * @returns Result of the addition
     * @tag arkts
     */
    public add(other: Double): Double {
        return Double.valueOf((this.value + other.toDouble()) as double)
    }

    /**
     * Performs floating point subtraction of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the subtraction
     *
     * @returns Result of the subtraction
     * @tag arkts
     */
    public sub(other: Double): Double {
        return Double.valueOf((this.value - other.toDouble()) as double)
    }

    /**
     * Performs floating point multiplication of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the multiplication
     *
     * @returns Result of the multiplication
     * @tag arkts
     */
    public mul(other: Double): Double {
        return Double.valueOf((this.value * other.toDouble()) as double)
    }

    /**
     * Performs floating point division of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the division
     *
     * @returns Result of the division
     * @tag arkts
     */
    public div(other: Double): Double {
        return Double.valueOf((this.value / other.toDouble()) as double)
    }

    /**
     * Checks if this instance value is less than value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is less than provided, false otherwise
     * @tag arkts
     */
    public isLessThan(other: Double): boolean {
        return this.value < other.toDouble();
    }

    /**
     * Checks if this instance value is less than or equal to value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is less than or equal to provided, false otherwise
     * @tag arkts
     */
    public isLessEqualThan(other: Double): boolean {
        return this.value <= other.toDouble();
    }

    /**
     * Checks if this instance value is greater than value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is greater than provided, false otherwise
     * @tag arkts
     */
    public isGreaterThan(other: Double): boolean {
        return this.value > other.toDouble();
    }

    /**
     * Checks if this instance value is greater than or equal to value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is greater than or equal to provided, false otherwise
     * @tag arkts
     */
    public isGreaterEqualThan(other: Double): boolean {
        return this.value >= other.toDouble();
    }

    /**
     * parseFloat(String) converts std.core.String to double
     *
     * @param s the string to convert
     *
     * @returns the result of conversion
     *
     * @note
     * If arg is '+Infinity', 'Infinity' or '-Infinity', return value is `inf` or `-inf` respectively.
     * If arg is '+0' or '-0', return value is 0 or -0.
     * If arg has leading zeroes, it's ignored: '0001.5' -> 1.5, '-0001.5' -> -1.5
     * If arg starts from '.', leading zero is implied: '.5' -> 0.5, '-.5' -> -0.5
     * If arg successfully parsed, trailing non-digits ignored: '-.6ffg' -> -0.6
     * If arg can not be parsed into a number, NaN is returned
     *
     * @remark
     * Implemented as native function,  @see `parseFloat()` intrinsic.
     *
     * ECMA reference: https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number.parsefloat
     *
    */
    public static native parseFloat(s: String): double;

    /**
     * parseInt(String, int) parses from String an integer of specified radix
     *
     * @param s the string to convert
     * @param r the radix of conversion; should be [2, 36]; 0 assumed to be 10
     *
     * @returns the result of parsing
     *
     * @note
     * If args ('10', 1) -> thrown ArgumentOutOfRangeError, ('10', 37) -> thrown ArgumentOutOfRangeError
     * If args ('10', 2) -> 2
     * If args ('10', 10) -> 10, ('10', 0) -> 10
     * If args ('ff', 16) -> 255
     * etc.
     *
     * @remark
     * Implemented as native function,  @see `parseInt()` intrinsic.
     *
     * ECMA reference: https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number.parseint
     *
    */
    public static native parseIntCore(s: String, r: int): double;

    /**
     * parseInt(String) parses from String an integer of radix 10
     *
     * @param s the string to convert
     *
     * @returns the result of parsing
     *
    */
    public static parseInt(s: String): double {
        return Double.parseIntCore(s, 0);
    }
    
    /**
     * parseInt(String, int) parses from String an integer of specified radix
     *
     * @param s the string to convert
     * @param r the radix of conversion; should be [2, 36]; 0 assumed to be 10
     *
     * @returns the result of parsing
     *
     * @note
     * If args ('10', 1) -> thrown ArgumentOutOfRangeError, ('10', 37) -> thrown ArgumentOutOfRangeError
     * If args ('10', 2) -> 2
     * If args ('10', 10) -> 10, ('10', 0) -> 10
     * If args ('ff', 16) -> 255
     * etc.
     *
     * @remark
     * Implemented as native function,  @see `parseInt()` intrinsic.
     *
     * ECMA reference: https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number.parseint
     *
    */
    public static parseInt(s: String, r: int): double {
        if ((r < 2 || r > 36) && r != 0.0) {
            return NaN;
        } else {
            return Double.parseIntCore(s, r);
        }
    }

    /**
     * parseInt(String, double) parses from String an integer of specified radix
     *
     * @param s the string to convert
     * @param r the radix of conversion; should be [2, 36]; 0 assumed to be 10
     *
     * @returns the result of parsing
     *
     * @note
     * If args ('10', 1) -> thrown ArgumentOutOfRangeError, ('10', 37) -> thrown ArgumentOutOfRangeError
     * If args ('10', 2) -> 2
     * If args ('10', 10) -> 10, ('10', 0) -> 10
     * If args ('ff', 16) -> 255
     * etc.
     *
     * @remark
     * Implemented as native function,  @see `parseInt()` intrinsic.
     *
     * ECMA reference: https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number.parseint
     *
    */
    public static parseInt(s: String, r: double): double {
        if (!Double.isFinite(r)) {
            r = 0.0;
        }
        if ((r < 2.0 || r > 36.0) && r != 0.0) {
            return NaN;
        } else {
            return Double.parseIntCore(s.trim(), Double.toInt(r));
        }
    }

    /**
     * toExponential(double) returns std.core.String representing the underlying double in exponential notation
     *
     * @param d the exponent (rounded to nearest integer); must be in [0, 100]
     *
     * @returns the result of conversion
     *
     * @note
     * If d = new Double(0.25); d.toExponential(2) -> '2.50e-1'
     * If d = new Double(0.25); d.toExponential(2.5) -> '2.50e-1'
     * If d = new Double(0.25); d.toExponential(1) -> '2.5e-1'
     * If d = new Double(12345.01); d.toExponential(10) -> '1.2345010000e+4'
     * If d = new Double(NaN); d.toExponential(10) -> 'NaN';
     * If d = new Double(Double.POSITIVE_INFINITY); d.toExponential(10) -> 'Infinity';
     *                                                                     '-Infinity' for negative
     *
     * @remark
     * Implemented as native function,  @see `toExponential()` intrinsic.
     *
     * ECMA reference: https://tc39.es/ecma262/multipage/doubles-and-dates.html#sec-double.prototype.toexponential
     *
    */
    private native toExponentialImpl(d: double): String;
    public native toExponentialWithNoDigit(): String;

    /**
     * toExponential() returns std.core.String representing the underlying double in exponential notation
     *
     * @returns the result of conversion
     *
    */
    public toExponential(): String {
        return this.toExponentialWithNoDigit();
    }

    /**
     * toExponential() returns std.core.String representing the underlying double in exponential notation
     *
     * @returns the result of conversion
     *
    */
    public toExponential(d?: double): String {
        if (d == undefined) {
            return this.toExponentialWithNoDigit()
        }
        return this.toExponentialImpl(d)
    }

    /**
     * toPrecision(double) returns std.core.String representing the underlying double on the specified precision
     *
     * @param d precision (rounded to nearest integer); must be in [1, 100]
     *
     * @returns the result of conversion
     *
     * @note
     * If d = new Double(0.25); d.toPrecision(4) -> '0.2500'
     * If d = new Double(1.01); d.toPrecision(4.7) -> '1.010'
     * If d = new Double(0.25); d.toPrecision(0) -> thrown ArgumentOutOfRangeError
     * If d = new Double(12345.123455); d.toPrecision(10) -> '12345.12346'
     *
     * @remark
     * Implemented as native function,  @see `toPrecision()` intrinsic.
     *
     * ECMA reference: https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number.prototype.toprecision
     *
    */
    public native toPrecision(d: double): String;

    /**
     * toPrecision() returns std.core.String representing the underlying double in exponential notation
     *                       basically, if toPrecision called with no argument it's just toString according
     *                       to spec
     *
     * @returns the result of conversion
     *
    */
    public toPrecision(): String {
        return this.toString();
    }

    /**
     * toFixed(double) returns std.core.String representing the underlying double using fixed-point notation
     *
     * @param d fixed size (integer part); must be in [0, 100]
     *
     * @returns the result of conversion
     *
     * @note
     * If d = new Double(0.1); d.toFixed(0) -> '0'
     * If d = new Double(0.7); d.toFixed(0) -> '1'
     * If d = new Double(0.12345); d.toFixed(1) -> '0.1'
     * If d = new Double(0.12345); d.toFixed(3) -> '0.123'
     * If d = new Double(Double.POSITIVE_INFINITY); d.toFixed(3) -> 'Infinity'
     * If d = new Double(Double.NaN); d.toFixed(3) -> 'NaN'
     * If d = new Double(0.25); d.toFixed(200) -> thrown ArgumentOutOfRangeError
     *
     * @remark
     * Implemented as native function,  @see `toFixed()` intrinsic.
     *
     * ECMA reference: https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number.prototype.tofixed
     *
    */
    public native toFixedImpl(d: double): String;

    /**
     * toFixed(double) returns std.core.String representing the underlying double using fixed-point notation
     *
     * @returns the result of conversion
     *
    */
    public toFixed(): String {
        return this.toFixed(0);
    }

    /**
     * toFixed(double) returns std.core.String representing the underlying double using fixed-point notation
     *
     * @returns the result of conversion
     *
    */
    public toFixed(d?: double): String {
        if (d == undefined) {
            return this.toFixedImpl(0)
        }
        return this.toFixedImpl(d)
    }

    /**
     * Converts bit representation to corresponding IEEE-754 floating point representation
     * @param bits bits to convert
     *
     * @returns double - converted value
     * @tag arkts
     */
    public static native bitCastFromLong(bits: long): double

    /**
     * Converts IEEE-754 floating point representation to corresponding bit representation
     * @param val value to convert
     *
     * @returns long - bit representation
     * @tag arkts
     */
    public static native bitCastToLong(val: double): long

    /**
     * numberFromString(String) converts std.core.String to double
     *
     * @param s the string to convert
     *
     * @returns the result of conversion
     *
     * @note
     * If arg is '+Infinity', 'Infinity' or '-Infinity', return value is `inf` or `-inf` respectively.
     * If arg is '+0' or '-0', return value is 0 or -0.
     * If arg has leading zeroes, it's ignored: '0001.5' -> 1.5, '-0001.5' -> -1.5
     * If arg starts from '.', leading zero is implied: '.5' -> 0.5, '-.5' -> -0.5
     * If arg successfully parsed, trailing non-digits cause return value is NaN: '-.6ffg' -> -NaN
     * If arg can not be parsed into a number, NaN is returned
     *
     *
     * ECMA reference: https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-number-constructor-number-value
     */
    private static native numberFromString(s: String): double;
}

/**
 * `Number' values represent floating-point numbers like 6 or -3.14
 */
export type Number = Double;

/**
 Number is a synonym for type number
 */
export type number = double;
