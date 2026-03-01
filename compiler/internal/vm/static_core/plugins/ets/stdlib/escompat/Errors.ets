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

/**
 * @file Defines the commonly used Errors for ArkTS
 * @kit ArkTS
 */

package escompat;

/**
 * Represents an evaluation error
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class EvalError extends Error {
    /**
     * Constructs a new EvalError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("EvalError", message, options)
    }

    /**
     * Constructs a new EvalError instance with provided message and options
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @returns { EvalError } - Newly created EvalError instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    static $_invoke(message?: String, options?: ErrorOptions): EvalError {
        return new EvalError(message, options)
    }
}

/**
 * Represents an error that occurs when a numeric types conversion is performed on an index expression, and the fractional part differs from 0.
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class NonIntegralIndexError extends Error {
    /**
     * Constructs a new NonIntegralIndexError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("NonIntegralIndexError", message, options)
    }

    /**
     * Constructs a new NonIntegralIndexError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @returns { NonIntegralIndexError } - Newly created NonIntegralIndexError instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    static $_invoke(message?: String, options?: ErrorOptions): NonIntegralIndexError {
        return new NonIntegralIndexError(message, options)
    }
}

/**
 * Represents an error that occurs when an operation could not be performed, typically (but not exclusively) when a value is not of the expected type
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class TypeError extends Error {
    /**
     * Constructs a new TypeError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("TypeError", message, options)
    }

    /**
     * Constructs a new TypeError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @returns { TypeError } - Newly created TypeError instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    static $_invoke(message?: String, options?: ErrorOptions): TypeError {
        return new TypeError(message, options)
    }
}

/**
 * Represents an error that occurs when a variable that doesn't exist (or hasn't yet been initialized) in the current scope is referenced
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class ReferenceError extends Error {
    /**
     * Constructs a new ReferenceError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("ReferenceError", message, options)
    }

    /**
     * Constructs a new ReferenceError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @returns { ReferenceError } - Newly created ReferenceError instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    static $_invoke(message?: String, options?: ErrorOptions): ReferenceError {
        return new ReferenceError(message, options)
    }
}

/**
 * Represents an error that occurs when a global URI handling function was used in a wrong way
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class URIError extends Error {
    /**
     * Constructs a new URIError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("URIError", message, options)
    }

    /**
     * Constructs a new URIError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @returns { URIError } - Newly created URIError instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    static $_invoke(message?: String, options?: ErrorOptions): URIError {
        return new URIError(message, options)
    }
}

/**
 * AggregateError object represents an error when several errors
 * need to be wrapped in a single error.
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class AggregateError extends Error {
    /**
     * Defines an array with aggregated errors
     *
     * @type { Array<Error> }
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    errors: Array<Error>

    /**
     * Constructs a new AggregateError instance with provided errors, message and error specific information
     *
     * @param { Iterable<Error> } errors - Errors to be aggregated
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(errors: Iterable<Error>, message?: String, options?: ErrorOptions) {
        super("AggregateError", message, options)
        this.errors = Array.from<Error>(errors)
    }

    /**
     * Constructs a new AggregateError instance with provided errors, message and error specific information
     *
     * @param { Error[] } errors - Errors to be aggregated
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(errors: Error[], message?: String, options?: ErrorOptions) {
        super("AggregateError", message, options)
        this.errors = Array.from<Error>(errors)
    }

    /**
     * Constructs a new AggregateError instance with provided message and error specific information
     *
     * @param { Iterable<Error> } errors - Errors to be aggregated
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @returns { AggregateError } - Newly created AggregateError instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    static $_invoke(errors: Iterable<Error>, message?: String, options?: ErrorOptions): AggregateError {
        return new AggregateError(errors, message, options)
    }

    /**
     * Constructs a new AggregateError instance with provided message and error specific information
     *
     * @param { Error[] } errors - Errors to be aggregated
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @returns { AggregateError } - Newly created AggregateError instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    static $_invoke(errors: Error[], message?: String, options?: ErrorOptions): AggregateError {
        return new AggregateError(errors, message, options)
    }

}

/**
 * Represents an error that occurs when an input string contains invalid or incorrectly formatted data.
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class FormatError extends Error {
    /**
     * Constructs a new FormatError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("FormatError", message, options)
    }

    /**
     * Constructs a new FormatError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @returns { FormatError } - Newly created FormatError instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    static $_invoke(message?: String, options?: ErrorOptions): FormatError {
        return new FormatError(message, options)
    }
}
