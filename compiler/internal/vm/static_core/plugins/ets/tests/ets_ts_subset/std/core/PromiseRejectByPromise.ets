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

let reject: ((err: Error) => void) | null = null;
let error = new Error('error')
let p1 = new Promise<number>((_resolve: (value: number) => void, _reject: (err: Error) => void): void => {
    reject = _reject;
});
let p2 = new Promise<number>((_resolve: (value: number | PromiseLike<number>)
        => void, _reject: (err: Error) => void): void => {
    _resolve(p1);
    _reject(new Error('NotMe'));
    _resolve(7);
});
p2.then((v:number): void => {
    console.log('Test failed. The promise should not be fulfilled.');
}, (err: Error): void => {
    if(err == error) {
        console.log('Test passed.')
    } else {
        console.log('Test failed. The promise should be rejected with the provided value.')
    }
})
reject!(error);
