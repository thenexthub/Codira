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

let resolve: ((value: Object) => void) | null = null;
let value = new Object();
let p1 = new Promise<Object>((_resolve: (v: Object) => void) => {
    resolve = _resolve;
});
let p2 = Promise.resolve<Object>(p1);
if (p1 !== p2) {
    throw new Error('Test failed. Expected the same promise.');
}

p2.then<void, void>((v: Object): void => {
    if (v !== value) {
        console.log('Test failed. The promise should be resolved by the provided value.');
        return;
    }
    console.log('Test passed.');
}, (err: Error): void => {
    console.log('Test failed. The promise should be rejected.');
});

resolve!(value);
