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
 * Represents an error that occurs when using a function under the API package in an incorrect manner
 */
export class BusinessError<T = void> extends Error {
    data?: T;

    constructor() {
        super();
        this.code = 0;
    }

    constructor(code: int, error: Error) {
        super(error.name, error.message, {cause: error.cause} as ErrorOptions);
        this.code = code;
    }

    constructor(code: int, data: T, error: Error) {
        super(error.name, error.message, {cause: error.cause} as ErrorOptions);
        this.code = code;
        this.data = data;
    }

    constructor(code: int, message: string, data?: T) {
        super(code, message);
        this.data = data;
    }
}

export type RecordData = undefined | null | Object | Record<string, RecordData> | Array<RecordData>;
