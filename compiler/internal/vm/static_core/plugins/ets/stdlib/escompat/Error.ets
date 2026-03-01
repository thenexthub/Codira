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
 * @file Defines the Error for ArkTS
 * @kit ArkTS
 */

package escompat;

/**
 * Defines an interface for Error options
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export interface ErrorOptions {
    /**
     * Specifies cause of an Error
     *
     * @type { Object | undefined }
     * @default undefined
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    cause?: Object
}

/**
 * Implementation of ErrorOptions
 *
 * @implements ErrorOptions
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class ErrorOptionsImpl implements ErrorOptions {
    /**
     * Specifies cause of an Error
     *
     * @type { Object | undefined }
     * @default undefined
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    cause_?: Object

    /**
     * Getter for Error cause
     *
     * @returns { Object | undefined } - A cause of the Error
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    override get cause(): Object | undefined {
        return this.cause_
    }

    /**
     * Setter for Error cause
     *
     * @param { Object | undefined } cause - A cause of the Error
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    set cause(cause: Object | undefined) {
        this.cause_ = cause
    }

    /**
     * Constructor for ErrorOptions
     *
     * @param { Object | undefined } cause - A cause of the Error
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(cause?: Object) {
        this.cause_ = cause
    }
}

/**
 * Stores information about stacktrace and cause in case of an error.
 * Serves as a base class for all error classes.
 * @syscap SystemCapability.Utils.Lang
 * @since 24 static
 */
export class Error {
    private name_: string
    private message_: string
    private stackLines: FixedArray<StackTraceElement> = []
    private stack_?: String
    private cause_?: Object
    private code_: int

    /**
     * Constructs a new error instance with provided code, message and cause
     *
     * @param { int } code - Error code
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(code: int, message?: String, options?: ErrorOptions) {
        this("Error", code, message, options)
    }

    /**
     * Constructs a new error instance with provided message and cause
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(message?: String, options?: ErrorOptions) {
        this("Error", 0, message, options)
    }

    /**
     * Constructs a new error instance with provided name, code, message and options
     *
     * @param { String } name - Error name
     * @param { int } code - Error code
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(name: String, code: int, message?: String, options?: ErrorOptions) {
        this.code_ = code
        this.message_ = (message == undefined) ? "" : message
        this.cause_ = (options == undefined) ? undefined : options.cause
        this.name_ = name
        this.stackLines = StackTrace.provisionStackTrace()
    }

    /**
     * Constructs a new error instance with provided name, message and options
     *
     * @param { String } name - Error name
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    constructor(name: String, message: String | undefined, options?: ErrorOptions) {
        this(name, 0, message, options)
    }

    /**
     * Gets the name of the error.
     *
     * @returns { String } - Current error name
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    get name(): string {
        return this.name_
    }

    /**
     * Sets the name of the error.
     *
     * @param { String } val - New name for the Error
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    set name(val: string) {
        this.name_ = val
    }

    /**
     * Gets the code of the error.
     *
     * @returns { int } - Current error code
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    get code(): int {
        return this.code_
    }

    /**
     * Sets the code of the error.
     *
     * @param { int } val - New code for the Error
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    set code(val: int) {
        this.code_ = val
    }

    /**
     * Gets the message of the error.
     *
     * @returns { String } - Current error message
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    get message(): string {
        return this.message_
    }

    /**
     * Sets the message of the error.
     *
     * @param { String } val - New message for the Error
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    set message(val: string) {
        this.message_ = val
    }

    /**
     * Gets the cause of the error.
     *
     * @returns { Object | undefined } - Current error cause
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    get cause(): Object | undefined {
        return this.cause_
    }

    /**
     * Sets the cause of the error.
     *
     * @param { Object | undefined } val - New cause for the Error
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    set cause(val: Object | undefined) {
        this.cause_ = val
    }

    /**
     * Constructs a new error instance with provided message and options
     *
     * @param { String | undefined } message - Error text
     * @param { ErrorOptions | undefined } options - Error options
     * @returns { Error } - Newly created Error instance
     * @static
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    static $_invoke(message?: String, options?: ErrorOptions): Error {
        return new Error(message, options)
    }

    /**
     * Converts this error to a string
     *
     * @returns { string } - String representation of the Error
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    override toString(): String {
        return this.name + ((this.name !== "" && this.message) ? ": " + this.message : this.message);
    }

    /**
     * Forms stack and returns it
     *
     * @returns { String | undefined } - Current error stacktrace
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    get stack(): String | undefined {
        this.formStack()
        return this.stack_
    }

    /**
     * Does stack replacement and clears previously collected stack info
     *
     * @param { String | undefined } newStack - New stacktrace for the error
     * @syscap SystemCapability.Utils.Lang
     * @since 24 static
     */
    set stack(newStack: String | undefined) {
        this.stack_ = newStack
        this.stackLines = []
    }

    private formStack() {
        if (this.stack_ != undefined) {
            return
        }
        if (this.stackLines.length == 0) {
            return
        }
        let builder = new StringBuilder("")

        // NOTE(kparshukov): find a better way to erase Error's ctors lines
        const provisionStackTraceLevel = 2
        const realStackStart = (this.stackLines.length > provisionStackTraceLevel ? provisionStackTraceLevel : 0)
        for (let i: int = realStackStart; i < this.stackLines.length; i++) {
            builder.append(this.stackLines[i].toString() + '\n')
        }

        this.stack_ = builder.toString()
        this.stackLines = []
    }
}

