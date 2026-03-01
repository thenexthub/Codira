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
 * Represents boxed short value and related operations
 */
export final class Short extends Integral implements Comparable<Short> {
    private value: short;

    /**
     * Constructs a new Short instance with initial value zero
     */
    public constructor() {
        this.value = 0;
    }

    /**
     * Constructs a new Short instance with provided initial value
     *
     * @param value the initial value
     */
    public constructor(value: short) {
        this.value = value;
    }

    /**
     * Returns boxed representation of the primitive
     *
     * @param value value to box
     *
     * @returns boxed value
     */
    public static valueOf(value: short): Short {
        // TODO(ivan-tyulyandin): caching is possible
        return new Short(value);
    }

    /**
     * Minimal value that this type can have as an shortegral
     */
    public static MIN_VALUE: short = -32768;

    /**
     * Maximal value that this type can have as an shortegral
     */
    public static MAX_VALUE: short = 32767;

    /**
     * Size of this type in bits
     */
    public static BIT_SIZE: byte = 16;

    /**
     * Size of this type in bytes
     */
    public static BYTE_SIZE: byte = 2;

    /**
     * Returns value of this instance
     *
     * @returns value as short
     */
    public override toByte(): byte {
        return (this.value).toByte();
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
        return this.value;
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
        return Short.toInt(this.value);
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
        return this.value.toFloat();
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
        return Short.toDouble(this.value);
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
     * Returns value of this instance
     *
     * @returns value as char
     */
    public toChar(): char {
        return this.value.toChar();
    }

    /**
     * Returns the primitive as byte value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toByte(value: short): byte;

    /**
     * Returns the primitive as int value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toInt(value: short): int;

    /**
     * Returns the primitive as long value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toLong(value: short): long;

    /**
     * Returns the primitive as float value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toFloat(value: short): float;

    /**
     * Returns the primitive as double value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toDouble(value: short): double;

    /**
     * Returns the primitive as char value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toChar(value: short): char;

    /**
     * Returns the primitive as short value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static toShort(value: short): short {
        return value;
    }

    /**
     * Compares this instance to other Short object
     * The result is less than 0 if this instance lesser than provided object
     * 0 if they are equal
     * and greater than 0 otherwise.
     *
     * @param other Short object to compare with
     *
     * @returns result of the comparison, -1 if this instance is less than provided object, 0 if equal, 1 if greater
     */
    public override compareTo(other: Short): int {
        if (this.value < other) {
            return -1;
        }
        if (this.value == other) {
            return 0;
        }
        return 1;
    }

    /**
     * Converts the primitive to a string
     *
     * @param v value to be converted
     *
     * @returns result of the conversion
     */
    public static toString(v: short): string {
        return StringBuilder.toString(v);
    }

    /**
     * Converts this object to a string
     *
     * @returns result of the conversion
     */
    public override toString(): String {
        return StringBuilder.toString(this.value);
    }

    /**
     * Converts this object to a string
     *
     * @returns result of the conversion
     */
    public toString(radix: number): string {
        return (new Long(this.value)).toString(radix);
    }

    /**
    * Checks for equality this instance with provided object, treated as a Short
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

        if (!(other instanceof Short)) {
            return false
        }

        return this.value == (other as Short).value
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
     * Performs shortegral addition of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the addition
     *
     * @returns Result of the addition
     */
    public add(other: Short): Short {
        return Short.valueOf((this.value + other.toShort()).toShort())
    }

    /**
     * Performs shortegral subtraction of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the subtraction
     *
     * @returns Result of the subtraction
     */
    public sub(other: Short): Short {
        return Short.valueOf((this.value - other.toShort()).toShort())
    }

    /**
     * Performs shortegral multiplication of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the multiplication
     *
     * @returns Result of the multiplication
     */
    public mul(other: Short): Short {
        return Short.valueOf((this.value * other.toShort()).toShort())
    }

    /**
     * Performs shortegral division of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the division
     *
     * @returns Result of the division
     */
    public div(other: Short): Short {
        return Short.valueOf((this.value / other.toShort()).toShort())
    }

    /**
     * Checks if this instance value is less than value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is less than provided, false otherwise
     */
    public isLessThan(other: Short): boolean {
        return this.value < other.toShort();
    }

    /**
     * Checks if this instance value is less than or equal to value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is less than or equal to provided, false otherwise
     */
    public isLessEqualThan(other: Short): boolean {
        return this.value <= other.toShort();
    }

    /**
     * Checks if this instance value is greater than value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is greater than provided, false otherwise
     */
    public isGreaterThan(other: Short): boolean {
        return this.value > other.toShort();
    }

    /**
     * Checks if this instance value is greater than or equal to value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is greater than or equal to provided, false otherwise
     */
    public isGreaterEqualThan(other: Short): boolean {
        return this.value >= other.toShort();
    }

    /**
     * parseInt(String, int) parses from String an integer of specified radix
     *
     * @param s the string to convert
     * @param r the radix of conversion; should be [2, 36]; 0 assumed to be 10
     *
     * @returns the result of parsing
    */
    public static native parseInt(s: String, r: int): short;
}
