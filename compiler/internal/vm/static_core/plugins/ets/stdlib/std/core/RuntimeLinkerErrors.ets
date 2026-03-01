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

class LinkerError extends Error {
    constructor(name: String, message: String | undefined, options: ErrorOptions | undefined) {
        super(name, message, options)
    }

    constructor(message?: String, options?: ErrorOptions) {
        super("LinkerError", message, options)
    }
}

export class LinkerVerificationError extends LinkerError {
    constructor(message?: String, options?: ErrorOptions) {
        super("LinkerVerificationError", message, options)
    }
}

export class LinkerUnresolvedClassError extends LinkerError {
    constructor(message?: String, options?: ErrorOptions) {
        super("LinkerUnresolvedClassError", message, options)
    }
}

export class LinkerUnresolvedFieldError extends LinkerError {
    constructor(message?: String, options?: ErrorOptions) {
        super("LinkerUnresolvedFieldError", message, options)
    }
}

export class LinkerUnresolvedMethodError extends LinkerError {
    constructor(message?: String, options?: ErrorOptions) {
        super("LinkerUnresolvedMethodError", message, options)
    }
}

export class LinkerBadSupertypeError extends LinkerError {
    constructor(message?: String, options?: ErrorOptions) {
        super("LinkerBadSupertypeError", message, options)
    }
}

export class LinkerTypeCircularityError extends LinkerError {
    constructor(message?: String, options?: ErrorOptions) {
        super("LinkerTypeCircularityError", message, options)
    }
}

export class LinkerMethodConflictError extends LinkerError {
    constructor(message?: String, options?: ErrorOptions) {
        super("LinkerMethodConflictError", message, options)
    }
}

export class LinkerAbstractMethodError extends LinkerError {
    constructor(message?: String, options?: ErrorOptions) {
        super("LinkerAbstractMethodError", message, options)
    }
}

export class LinkerClassNotFoundError extends LinkerError {
    constructor(message?: String, options?: ErrorOptions) {
        super("LinkerClassNotFoundError", message, options)
    }
}
