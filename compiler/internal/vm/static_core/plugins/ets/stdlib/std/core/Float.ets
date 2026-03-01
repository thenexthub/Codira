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

package std.core;

/**
 * Represents boxed float value and related operations
 */
export final class Float extends Floating implements Comparable<Float> {
    private value: float;

    /**
     * Constructs a new Float instance with initial value zero
     */
    public constructor() {
        this.value = 0.0f;
    }

    /**
     * Constructs a new Float instance with provided initial value
     *
     * @param value the initial value
     */
    public constructor(value: float) {
        this.value = value;
    }

    /**
     * Constructs a new Float instance with provided initial value (`double` type literal)
     *
     * @param value the initial value
     */
    public constructor(value: double) {
        this.value = Double.toFloat(value);
    }

    /**
     * Returns boxed representation of the primitive
     *
     * @param value value to box
     *
     * @returns boxed value
     */
    public static valueOf(value: float): Float {
        // TODO(ivan-tyulyandin): caching is possible
        return new Float(value);
    }

    /**
     * Minimal value that this type can have as a float
     */
    public static readonly MIN_VALUE: float = 1.4e-45f;

    /**
     * Maximal value that this type can have as a float
     */
    public static readonly MAX_VALUE: float = 3.40282346638528860e+38f;

    /**
     * Maximal integer value that can be used as a float without loss of precision
     */
    public static readonly MAX_SAFE_INTEGER = 16777215;

    /**
     * Size of this type in bits
     */
    public static readonly BIT_SIZE: byte = 32;

    /**
     * Size of this type in bytes
     */
    public static readonly BYTE_SIZE: byte = 4;


    /**
     * Represents the NaN value according to IEEE-754 specification
     */
    public static readonly NaN: float = Double.toFloat(0.0 / 0.0);

    /**
     * Represents the +Infinity value according to IEEE-754 specification
     */
    public static readonly POSITIVE_INFINITY: float = Double.toFloat(1.0 / 0.0);

    /**
     * Represents the -Infinity value according to IEEE-754 specification
     */
    public static readonly NEGATIVE_INFINITY: float = Double.toFloat(-1.0 / 0.0);

    /**
     * Number of significant precision bits in this floating type
     */
    public static readonly PRECISION: byte = 24;

    /**
     * Minimal possible difference between two float values
     * For float (IEEE-754 binary32) it is 2^(-23) and its bit representation is 0x34000000.
     */
    public static readonly DELTA: float = Float.bitCastFromInt(0x34000000);

    /**
     * Minimal possible difference between two float values
     */
    public static readonly EPSILON: float = Float.DELTA;

    /**
     * Returns value of this instance
     *
     * @returns value as byte
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
     */
    public override byteValue(): byte {
        return this.toByte();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as short
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
     */
    public override shortValue(): short {
        return this.toShort();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as int
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
     */
    public override intValue(): int {
        return this.toInt();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as long
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
     */
    public override longValue(): long {
        return this.toLong();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as float
     */
    public override toFloat(): float {
        return this.value;
    }

    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns value as float
     */
    public override floatValue(): float {
        return this.toFloat();
    }

    /**
     * Returns value of this instance
     *
     * @returns value as double
     */
    public override toDouble(): double {
        return this.value.toDouble();
    }


    /**
     * Returns value of this instance
     *
     * Supportive interface
     *
     * @returns value as double
     */
    public override doubleValue(): double {
        return this.toDouble();
    }

    /**
     * Returns the primitive as byte value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toByte(value: float): byte;

    /**
     * Returns the primitive as short value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toShort(value: float): short;

    /**
     * Returns the primitive as int value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toInt(value: float): int;

    /**
     * Returns the primitive as long value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toLong(value: float): long;

    /**
     * Returns the primitive as double value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static toFloat(value: float): float {
        return value;
    }

    /**
     * Returns the primitive as double value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toDouble(value: float): double;

    /**
     * Compares this instance to other Float object
     * The result is less than 0 if this instance lesser than provided object
     * 0 if they are equal
     * and greater than 0 otherwise.
     *
     * @param other Float object to compare with
     *
     * @returns result of the comparison, -1 if this instance is less than provided object, 0 if equal, 1 if greater
     */
    public override compareTo(other: Float): int {
        if ((this.isNaN() && other.isNaN()) || (Math.abs(this.value - other) < Float.EPSILON)) {
            return 0;
        }
        if (this.isNaN() || this.value > other) {
            return 1;
        }
        return -1;
    }

    /**
     * toString(f: float, r: int): String -- returns a string representation of f by radix r
     */
    public static native toString(f: float, r: int): String;

    /**
     * toString(f: float): String -- returns a string representation of f by radix 10
     */
    public static toString(f: float): String {
        return Float.toString(f, 10);
    }

    /**
     * Converts this object to a string
     *
     * @returns result of the conversion by radix r
     */
    public toString(r: int): String {
        return Float.toString(this.value, r);
    }

    /**
     * Converts this object to a string
     *
     * @returns result of the conversion by radix 10
     */
    public override toString(): String {
        return Float.toString(this.value, 10);
    }
    
    /**
     * Converts this object to a locale-specific string representation
     *
     * @returns result of the locale-specific conversion
     */
    public toLocaleString(): String {
        return new Intl.NumberFormat().format(this.value)
    }

    /**
     * compare(float, float) checks if two floats are differs no more than by Float.DELTA
     *
     * @param lhs left-hand side float for comparison
     *
     * @param rhs right-hand side float for comparison
     *
     * @returns true if lhs and rhs are equal with respect to Float.DELTA
     */
    public static compare(lhs: float, rhs: float): boolean {
        return (abs(lhs - rhs) <= Float.DELTA)
    }

    /**
     * Checks for equality this instance with provided object, treated as a Float
     *
     * @param other object to be checked against
     *
     * @returns true if provided object and this instance have same value, false otherwise
     * Returns false if type of provided object is not the same as this type
     */
    public equals(other: Any): boolean {
        if (this === other) {
            return true
        }

        if (!(other instanceof Float)) {
            return false
        }

        return this.value == (other as Float).value
    }

    /**
     * Checks if float is NaN (not a number)
     *
     * @param v the float to test
     *
     * @returns true if the argument is NaN
     */
    public static isNaN(v: float): boolean {
        // IEEE-754 feature
        return v != v;
    }

    /**
     * Checks if the underlying float is NaN (not a number)
     *
     * @returns true if the underlying float is NaN
     */
    public isNaN(): boolean {
        return Float.isNaN(this.value);
    }

    /**
     * Checks if float is a floating point value (not a NaN or infinity)
     *
     * @param v the float to test
     *
     * @returns true if the argument is a floating point value
     */
    public static isFinite(v: float): boolean {
        return !(Float.isNaN(v) || (v == Float.POSITIVE_INFINITY) || (v == Float.NEGATIVE_INFINITY));
    }

    /**
     * Checks if the underlying float is a floating point value (not a NaN or infinity)
     *
     * @returns true if the underlying float is a floating point value
     */
    public isFinite(): boolean {
        return Float.isFinite(this.value);
    }

    /**
     * Checks if float is similar to an integer value
     *
     * @param v the float to test
     *
     * @returns true if the argument is similar to an integer value
     */
    public static isInteger(v: float): boolean {
        // In the language % works as C fmod that differs with IEEE-754 % definition
        return Float.compare(v % 1.0f, 0.0f);
    }

    /**
     * Checks if the underlying float is similar to an integer value
     *
     * @returns true if the underlying float is similar to an integer value
     */
    public isInteger(): boolean {
        return Float.isInteger(this.value);
    }

    /**
     * Checks if float is a safe integer value
     *
     * @param v the float to test
     *
     * @returns true if the argument is integer ans less than MAX_SAFE_INTEGER
     */
    public static isSafeInteger(v: float): boolean {
        return Float.isInteger(v) && (abs(v) <= Float.MAX_SAFE_INTEGER);
    }

    /*
     * Checks if float is a safe integer value
     *
     * @returns true if the underlying float is a safe integer
     */
    public isSafeInteger(): boolean {
        return Float.isSafeInteger(this.value);
    }

    /**
     * Performs floating point addition of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the addition
     *
     * @returns Result of the addition
     */
    public add(other: Float): Float {
        return Float.valueOf((this.value + other.toFloat()) as float)
    }

    /**
     * Performs floating point subtraction of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the subtraction
     *
     * @returns Result of the subtraction
     */
    public sub(other: Float): Float {
        return Float.valueOf((this.value - other.toFloat()) as float)
    }

    /**
     * Performs floating point multiplication of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the multiplication
     *
     * @returns Result of the multiplication
     */
    public mul(other: Float): Float {
        return Float.valueOf((this.value * other.toFloat()) as float)
    }

    /**
     * Performs floating point division of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the division
     *
     * @returns Result of the division
     */
    public div(other: Float): Float {
        return Float.valueOf((this.value / other.toFloat()) as float)
    }

    /**
     * Checks if this instance value is less than value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is less than provided, false otherwise
     */
    public isLessThan(other: Float): boolean {
        return this.value < other.toFloat();
    }

    /**
     * Checks if this instance value is less than or equal to value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is less than or equal to provided, false otherwise
     */
    public isLessEqualThan(other: Float): boolean {
        return this.value <= other.toFloat();
    }

    /**
     * Checks if this instance value is greater than value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is greater than provided, false otherwise
     */
    public isGreaterThan(other: Float): boolean {
        return this.value > other.toFloat();
    }

    /**
     * Checks if this instance value is greater than or equal to value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is greater than or equal to provided, false otherwise
     */
    public isGreaterEqualThan(other: Float): boolean {
        return this.value >= other.toFloat();
    }

    /**
     * Converts bit representation to corresponding IEEE-754 floating point representation
     * @param bits bits to convert
     *
     * @returns float - converted value
     */
    public static native bitCastFromInt(bits: int): float

    /**
     * Converts IEEE-754 floating point representation to corresponding bit representation
     * @param val value to convert
     *
     * @returns int - bit representation
     */
    public static native bitCastToInt(val: float): int
}
