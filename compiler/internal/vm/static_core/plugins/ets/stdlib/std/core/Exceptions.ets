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

// TODO(orlovskiymaxim): what cause?
/**
 * Represents error that is thrown in case of ?
 */
export final class NoDataError extends Error {
    // NOTE(kparshukov): #28318 move to std/core/Errors.ets
    constructor(message?: String, options?: ErrorOptions) {
        super("NoDataError", message, options)
    }
}

/**
 * Represents exception that is thrown when provided argument have value outside the allowable range
 */
export final class ArgumentOutOfRangeError extends Error {
    // NOTE(kparshukov): #28318 move to std/core/Errors.ets
    constructor(message?: String, options?: ErrorOptions) {
        super("ArgumentOutOfRangeError", message, options)
    }
}

/**
 * Represents error that is thrown when a method has been invoked at an illegal or inappropriate time.
 */
export class IllegalStateError extends Error {
    // NOTE(kparshukov): #28318 move to std/core/Errors.ets
    constructor(message?: String, options?: ErrorOptions) {
        super("IllegalStateError", message, options)
    }
}

/**
 * Represents error that is thrown when the requested operation is not supported.
 */
export final class UnsupportedOperationError extends Error {
    // NOTE(kparshukov): #28318 move to std/core/Errors.ets
    constructor(message?: String, options?: ErrorOptions) {
        super("UnsupportedOperationError", message, options)
    }
}

/**
 * @class Represents error that is thrown when
 * a method has been passed an illegal argument
 */
export final class IllegalArgumentError extends Error {
    // NOTE(kparshukov): #28318 move to std/core/Errors.ets
    constructor(message?: String, options?: ErrorOptions) {
        super("IllegalArgumentError", message, options)
    }
}

/**
 * @class Represents runtime error 
 */
export final class RuntimeError extends Error {
    // NOTE(kparshukov): #28318 move to std/core/Errors.ets
    constructor(message?: String, options?: ErrorOptions) {
        super("RuntimeError", message, options)
    }
}