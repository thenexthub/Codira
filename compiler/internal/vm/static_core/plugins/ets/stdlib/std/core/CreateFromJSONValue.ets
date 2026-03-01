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
 * Creates a Boolean instance based on JSONValue
 *
 * @param json: JSONValue - a JSON representation
 *
 * @throws JSONTypeError if json does not encode a valid boolean literal
 *
 * @returns Boolean - true if JSON encodes true literal, false if JSON encodes false literal
 */
export function createFromJSONValue(this: Boolean, json: JSONValue): Boolean {
    if (json instanceof JSONFalse) {
        return Boolean.FALSE
    } else if (json instanceof JSONTrue) {
        return Boolean.TRUE
    }
    throw new JSONTypeError('Cannot create Boolean from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Byte instance based on JSONValue
 *
 * @param json: JSONValue - a JSON representation
 *
 * @throws JSONTypeError if json does not encode a valid byte
 *
 * @returns Byte - byte value decoded from JSON
 */
export function createFromJSONValue(this: Byte, json: JSONValue): Byte {
    if (json instanceof JSONNumber) {
        let num = (json as JSONNumber).value
        if (Double.isInteger(num) && Byte.MIN_VALUE <= num && num <= Byte.MAX_VALUE) {
            return num.toByte()
        }
    }
    throw new JSONTypeError('Cannot create Byte from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Char instance based on JSONValue
 *
 * @param json: JSONValue - a JSON representation
 *
 * @throws JSONTypeError if json does not encode a valid char
 *
 * @returns char - char value decoded from JSON
 */
export function createFromJSONValue(this: Char, json: JSONValue): Char {
    if (json instanceof JSONString) {
        let str = (json as JSONString).value
        if (str.getLength() == 1) {
            return Char.valueOf(str.charAt(0))
        }
    }
    throw new JSONTypeError('Cannot create Char from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Short instance based on JSONValue
 *
 * @param json: JSONValue - a JSON representation
 *
 * @throws JSONTypeError if json does not encode a valid short
 *
 * @returns Short - short value decoded from JSON
 */
export function createFromJSONValue(this: Short, json: JSONValue): Short {
    if (json instanceof JSONNumber) {
        let num = (json as JSONNumber).value
        if (Double.isInteger(num) && Short.MIN_VALUE <= num && num <= Short.MAX_VALUE) {
            return Short.valueOf(num.toShort())
        }
    }
    throw new JSONTypeError('Cannot create Short from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Int instance based on JSONValue
 *
 * @param json: JSONValue - a JSON representation
 *
 * @throws JSONTypeError if json does not encode a valid int
 *
 * @returns Int - int value decoded from JSON
 */
export function createFromJSONValue(this: Int, json: JSONValue): Int {
    if (json instanceof JSONNumber) {
        let num = (json as JSONNumber).value
        if (Double.isInteger(num) && Int.MIN_VALUE <= num && num <= Int.MAX_VALUE) {
            return Int.valueOf(Double.toInt(num))
        }
    }
    throw new JSONTypeError('Cannot create Int from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Long instance based on JSONValue
 *
 * @param json: JSONValue - a JSON representation
 *
 * @throws JSONTypeError if json does not encode a valid long
 *
 * @returns Long - long value decoded from JSON
 */
export function createFromJSONValue(this: Long, json: JSONValue): Long {
    if (json instanceof JSONNumber) {
        let num = (json as JSONNumber).value
        if (Double.isInteger(num) && Long.MIN_VALUE <= num && num <= Long.MAX_VALUE) {
            return Long.valueOf(num.toLong())
        }
    }
    throw new JSONTypeError('Cannot create Long from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Float instance based on JSONValue
 *
 * @param json: JSONValue - a JSON representation
 *
 * @throws JSONTypeError if json does not encode a valid float
 *
 * @returns Float - float value decoded from JSON
 */
export function createFromJSONValue(this: Float, json: JSONValue): Float {
    if (json instanceof JSONNumber) {
        let val = (json as JSONNumber).value
        if (-Float.MAX_VALUE <= val && val <= Float.MAX_VALUE) {
            return Float.valueOf(((json as JSONNumber).value).toFloat())
        }
    }
    throw new JSONTypeError('Cannot create Double from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a Double instance based on JSONValue
 *
 * @param json: JSONValue - a JSON representation
 *
 * @throws JSONTypeError if json does not encode a valid double
 *
 * @returns Double - double value decoded from JSON
 * @tag arkts
 */
export function createFromJSONValue(this: Double, json: JSONValue): Double {
    if (json instanceof JSONNumber) {
        return Double.valueOf((json as JSONNumber).value)
    }
    throw new JSONTypeError('Cannot create Double from JSON', {cause: json as Object} as ErrorOptions)
}

/**
 * Creates a String instance based on JSONValue
 *
 * @param json: JSONValue - a JSON representation
 *
 * @throws JSONTypeError if json does not encode a valid String
 *
 * @returns String - string value decoded from JSON
 */
export function createFromJSONValue(this: String, json: JSONValue): String {
    if (json instanceof JSONString) {
        return (json as JSONString).value
    }
    throw new JSONTypeError('Cannot create String from JSON', {cause: json as Object} as ErrorOptions)
}
