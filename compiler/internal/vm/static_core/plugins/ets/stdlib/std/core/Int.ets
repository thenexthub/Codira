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
 * Represents boxed int value and related operations
 */
export final class Int extends Integral implements Comparable<Int> {
    private value: int;

    /**
     * Constructs a new Int instance with initial value zero
     */
    public constructor() {
        this.value = 0;
    }

    /**
     * Constructs a new Int instance with provided initial value
     *
     * @param value the initial value
     */
    public constructor(value: int) {
        this.value = value;
    }

    /**
     * Returns boxed representation of the primitive
     *
     * @param value value to box
     *
     * @returns boxed value
     */
    public static valueOf(value: int): Int {
        // TODO(ivan-tyulyandin): caching is possible
        return new Int(value);
    }

    /**
     * Minimal value that this type can have as an integral
     */
    public static readonly MIN_VALUE: int = -2147483648;

    /**
     * Maximal value that this type can have as an integral
     */
    public static readonly MAX_VALUE: int = 2147483647;

    /**
     * Size of this type in bits
     */
    public static readonly BIT_SIZE: byte = 32;

    /**
     * Size of this type in bytes
     */
    public static readonly BYTE_SIZE: byte = 4;

    /**
     * Returns value of this instance
     *
     * @returns value as int
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
        return this.value;
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
    public static native toByte(value: int): byte;

    /**
     * Returns the primitive as short value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toShort(value: int): short;

    /**
     * Returns the primitive as long value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toLong(value: int): long;

    /**
     * Returns the primitive as float value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toFloat(value: int): float;

    /**
     * Returns the primitive as double value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toDouble(value: int): double;

    /**
     * Returns the primitive as char value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static native toChar(value: int): char;

    /**
     * Returns the primitive as int value
     *
     * @param value value to cast
     *
     * @returns casted value
     */
    public static toInt(value: int): int {
        return value;
    }

    /**
     * Compares this instance to other Int object
     * The result is less than 0 if this instance lesser than provided object
     * 0 if they are equal
     * and greater than 0 otherwise.
     *
     * @param other Int object to compare with
     *
     * @returns result of the comparison, -1 if this instance is less than provided object, 0 if equal, 1 if greater
     */
    public override compareTo(other: Int): int {
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
    public static toString(v: int): string {
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

    equals(other: Any): boolean {
        if (this === other) {
            return true
        }

        if (!(other instanceof Int)) {
            return false
        }

        return this.value == (other as Int).value
    }

    public toLocaleString(): string {
        return new Intl.NumberFormat().format(this.value)
    }

    /**
     * Performs integral addition of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the addition
     *
     * @returns Result of the addition
     */
    public add(other: Int): Int {
        return Int.valueOf((this.value + other.toInt()) as int)
    }

    /**
     * Performs integral subtraction of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the subtraction
     *
     * @returns Result of the subtraction
     */
    public sub(other: Int): Int {
        return Int.valueOf((this.value - other.toInt()) as int)
    }

    /**
     * Performs integral multiplication of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the multiplication
     *
     * @returns Result of the multiplication
     */
    public mul(other: Int): Int {
        return Int.valueOf((this.value * other.toInt()) as int)
    }

    /**
     * Performs integral division of this instance with provided one, returns the result as new instance
     *
     * @param other Right hand side of the division
     *
     * @returns Result of the division
     */
    public div(other: Int): Int {
        return Int.valueOf((this.value / other.toInt()) as int)
    }

    /**
     * Checks if this instance value is less than value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is less than provided, false otherwise
     */
    public isLessThan(other: Int): boolean {
        return this.value < other.toInt();
    }

    /**
     * Checks if this instance value is less than or equal to value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is less than or equal to provided, false otherwise
     */
    public isLessEqualThan(other: Int): boolean {
        return this.value <= other.toInt();
    }

    /**
     * Checks if this instance value is greater than value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is greater than provided, false otherwise
     */
    public isGreaterThan(other: Int): boolean {
        return this.value > other.toInt();
    }

    /**
     * Checks if this instance value is greater than or equal to value of provided instance
     *
     * @param other Right hand side of the comparison
     *
     * @returns true if this value is greater than or equal to provided, false otherwise
     */
    public isGreaterEqualThan(other: Int): boolean {
        return this.value >= other.toInt();
    }

    /**
     * parseInt(String, int) parses from String an integer of specified radix
     *
     * @param s the string to convert
     * @param r the radix of conversion; should be [2, 36]; 0 assumed to be 10
     *
     * @returns the result of parsing
    */
    public static native parseInt(s: String, r: int): int;
}
