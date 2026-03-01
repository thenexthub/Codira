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

let error = new Error();
class Thenable implements PromiseLike<Error> {
    then<U, E>(_?: (value: Error) => U | PromiseLike<U>, onRejected?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<Awaited<U|E>> {
        return Promise.resolve().then<E>((): E | PromiseLike<E> => {
            return onRejected!(error);
        }) as PromiseLike<Awaited<U|E>>;
    }
};

Promise.any<Error>([new Thenable()]).then<void, void>((values): void => {
    console.error('Test failed. The promise should not be resolved.');
}, (err): void => {
    if ((err as AggregateError).errors.length != 1) {
        console.error('Test failed. Expected errors count 1 but got ' + (err as AggregateError).errors.length + '.');
        return;
    }
    if ((err as AggregateError).errors[0] != error) {
        console.error('Test failed. The promise should be rejected by the specified value.');
        return;
    }
    console.log('Test passed.');
});
