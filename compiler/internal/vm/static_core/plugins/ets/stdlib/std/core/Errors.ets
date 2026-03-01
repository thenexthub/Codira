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

package std.core;

/**
 * Represents an error that occurs when assertion fails.
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class AssertionError extends Error {
    /**
     * Constructs a new AssertionError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("AssertionError", message, options)
        console.error(message ?? "Assertion failed.");
    }
}

/**
 * Represents an error that occurs when division by zero is performed.
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class DivideByZeroError extends Error {
    /**
     * Constructs a new DivideByZeroError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("DivideByZeroError", message, options)
    }
}

/**
 * Represents an error that occurs when null pointer is dereferenced.
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class NullPointerError extends Error {
    /**
     * Constructs a new NullPointerError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("NullPointerError", message, options)
    }
}

/**
 * Represents an error that occurs when exception is thrown and not caught
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class UncaughtExceptionError extends Error {
    /**
     * Constructs a new FormatError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("UncaughtExceptionError", message, options)
    }
}

/**
 * Represents an error that occurs when trying to interpret syntactically invalid code
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class SyntaxError extends Error {
    /**
     * Constructs a new SyntaxError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("SyntaxError", message, options)
    }

    /**
     * Constructs a new SyntaxError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @returns { SyntaxError }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    static $_invoke(message?: String, options?: ErrorOptions): SyntaxError {
        return new SyntaxError(message, options)
    }
}

/**
 * Represents an error that occurs when JSONValue can not be assigned to a type
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class JSONTypeError extends Error {
    /**
     * Constructs a new JSONTypeError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("JSONTypeError", message, options)
    }
}

/**
 * Represents an error that occurs when provided collection index is out of range
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class RangeError extends Error {
    /**
     * Constructs a new RangeError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("RangeError", message, options)
    }

    /**
     * Constructs a new RangeError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @returns { RangeError }
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    static $_invoke(message?: String, options?: ErrorOptions): RangeError {
        return new RangeError(message, options)
    }
}

/**
 * Represents error that is thrown when negative array size is supplied
 * @extends RangeError
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export final class NegativeArraySizeError extends RangeError {
    /**
     * Constructs a new NegativeArraySizeError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super(message, options)
        this.name = "NegativeArraySizeError"
    }
}

/**
 * Represents an error that occurs when array is oging to be indexed out of its bounds
 * @extends RangeError
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export final class ArrayIndexOutOfBoundsError extends RangeError {
    /**
     * Constructs a new ArrayIndexOutOfBoundsError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super(message, options)
        this.name = "ArrayIndexOutOfBoundsError"
    }
}

/**
 * Represents error that is thrown when illegal arithmetic operation is occured (e.g. division by zero)
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export final class ArithmeticError extends Error {
    /**
     * Constructs a new ArithmeticError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("ArithmeticError", message, options)
    }
}

/**
 * Represents an error that occurs when memory allocation fails
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class OutOfMemoryError extends Error {
    /**
     * Constructs a new OutOfMemoryError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("OutOfMemoryError", message, options)
    }
}

/**
 * Represents an error that occurs when the available memory is not sufficient to create the activation frame
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class StackOverflowError extends Error {
    /**
     * Constructs a new StackOverflowError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("StackOverflowError", message, options)
    }
}

/**
 * Represents an error that occurs when attempting to instantiate abstract class or an interface
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class InstantiationError extends Error {
    /**
     * Constructs a new InstantiationError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("InstantiationError", message, options)
    }
}

/**
 * Represents an error that occurs when an internal error occured
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class InternalError extends Error {
    /**
     * Constructs a new InternalError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("InternalError", message, options)
    }
}

/**
 * Represents an error that is thrown when attempting to wait, notify or notifyAll on object, that hasn't been synchronised
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class IllegalMonitorStateError extends Error {
    /**
     * Constructs a new IllegalMonitorStateError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("IllegalMonitorStateError", message, options)
    }
}

/**
 * Represents error that is thrown when provided collection index is out of bounds
 * @extends RangeError
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class IndexOutOfBoundsError extends RangeError {
    /**
     * Constructs a new IndexOutOfBoundsError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super(message, options)
        this.name = "IndexOutOfBoundsError"
    }
}

/**
 * Represents error that is thrown when provided string index is out of bounds
 * @extends RangeError
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export final class StringIndexOutOfBoundsError extends RangeError {
    /**
     * Constructs a new StringIndexOutOfBoundsError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super(message, options)
        this.name = "StringIndexOutOfBoundsError"
    }
}

/**
 * Represents error that is thrown in case of illegal class casting
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export final class ClassCastError extends Error {
    /**
     * Constructs a new ClassCastError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("ClassCastError", message, options)
    }
}

/**
 * Represents error that is thrown when attempting to store an object of different type in array of type-erased objects
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export final class ArrayStoreError extends Error {
    /**
     * Constructs a new ArrayStoreError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("ArrayStoreError", message, options)
    }
}

/**
 * Represents error that is thrown when there is an error in initializer
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class ExceptionInInitializerError extends Error {
    /**
     * Constructs a new ExceptionInInitializerError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("ExceptionInInitializerError", message, options)
    }
}

/**
 * Represents error that is thrown when coroutines limit is reached
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class CoroutinesLimitExceedError extends Error {
    /**
     * Constructs a new CoroutinesLimitExceedError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("CoroutinesLimitExceedError", message, options)
    }
}

/**
 * Represents error that is thrown when lock is in an illegal state
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class IllegalLockStateError extends Error {
    /**
     * Constructs a new IllegalLockStateError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("IllegalLockStateError", message, options)
    }
}

/**
 * Represents the error that is thrown when invalid operation is called on coroutine
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class InvalidCoroutineOperationError extends Error {
    /**
     * Constructs a new InvalidCoroutineOperationError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("InvalidCoroutineOperationError", message, options)
    }
}

/**
 * Represents the error that is thrown when there is a wrong unsafe call to Function
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class ArgumentsUnderapplicationError extends Error {
    /**
     * Constructs a new ArgumentsUnderapplicationError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("ArgumentsUnderapplicationError", message, options)
    }
}

/**
 * Represents the error that is thrown when there is store access error
 * @extends Error
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class InvalidStoreAccessError extends Error {
    /**
     * Constructs a new InvalidStoreAccessError instance with provided message and error specific information
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        super("InvalidStoreAccessError", message, options)
    }
}
