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

let r1 = Promise.reject<string>(new Error('rejected 0'))
let r2 = Promise.reject<string>(new Error('rejected 1'));
let r3 = Promise.reject<string>(new Error('rejected 2'));
Promise.allSettled([r1, r2, r3]).then<void, void>((values): void => {
    if (values.length != 3) {
        console.log('Test failed. Expected a string array of length 3 but got length ' + values.length + '.');
        return;
    }
    if (values[0].status != 'rejected' || values[1].status != 'rejected' || values[2].status != 'rejected') {
        console.log('Test failed. Expected elements 0, 2, 4 have status \'rejected\'.');
        return;
    }
    if ((values[0] as PromiseRejectedResult).reason.message != 'rejected 0') {
        console.log('Test failed. Unexpected value of the element 0.');
        return;
    }
    if ((values[1] as PromiseRejectedResult).reason.message != 'rejected 1') {
        console.log('Test failed. Unexpected value of the element 1.');
        return;
    }
    if ((values[2] as PromiseRejectedResult).reason.message != 'rejected 2') {
        console.log('Test failed. Unexpected value of the element 2.');
        return;
    }
    console.log('Test passed.');
}, (error: Error): void => {
    console.log('Test failed. The promise should not be rejected.');
}).catch((e: Error): void => {
    console.log('Test failed: ' + e);
});
