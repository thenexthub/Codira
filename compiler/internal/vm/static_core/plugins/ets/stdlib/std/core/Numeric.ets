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
 * Can be implemented by any type that represents a numeric value.
 */
export abstract class Numeric extends Object {

    /**
     * Returns the value of an object as a byte
     *
     * @returns object's value
     */
    public abstract toByte(): byte;

    /**
     * Returns the value of an object as an int
     *
     * @returns object's value
     */
    public abstract toInt(): int;

    /**
     * Returns the value of an object as a short
     *
     * @returns object's value
     */
    public abstract toShort(): short;

    /**
     * Returns the value of an object as a long
     *
     * @returns object's value
     */
    public abstract toLong(): long;

    /**
     * Returns the value of an object as a float
     *
     * @returns object's value
     */
    public abstract toFloat(): float;

    /**
     * Returns the value of an object as a double
     *
     * @returns object's value
     */
    public abstract toDouble(): double;

    /**
     * Returns the value of an object as a byte
     *
     * Supportive interface
     *
     * @returns object's value
     */
    public abstract byteValue(): byte;

    /**
     * Returns the value of an object as an int
     *
     * Supportive interface
     *
     * @returns object's value
     */
    public abstract intValue(): int;

    /**
     * Returns the value of an object as a short
     *
     * Supportive interface
     *
     * @returns object's value
     */
    public abstract shortValue(): short;

    /**
     * Returns the value of an object as a long
     *
     * Supportive interface
     *
     * @returns object's value
     */
    public abstract longValue(): long;

    /**
     * Returns the value of an object as a float
     *
     * Supportive interface
     *
     * @returns object's value
     */
    public abstract floatValue(): float;

    /**
     * Returns the value of an object as a double
     *
     * Supportive interface
     *
     * @returns object's value
     */
    public abstract doubleValue(): double;
}

/**
 * Common class for all integral types
 */
export abstract class Integral extends Numeric {}

/**
 * Common class for all real number
 */
export abstract class Floating extends Numeric {}
