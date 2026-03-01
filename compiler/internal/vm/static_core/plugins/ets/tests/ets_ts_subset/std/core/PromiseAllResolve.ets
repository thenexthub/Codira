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

let v1 = new Object();
let v2 = new Object();
let v3 = new Object();
Promise.all([v1, v2, v3]).then<void, void>((values: Array<Object>): void => {
    if (values.length != 3) {
        console.log('Test failed. Expected object array of length 3 but get length ' + values.length + '.');
        return;
    }
    if (values[0] != v1) {
        console.log('Test failed. Unexpected value at index 0.');
        return;
    }
    if (values[1] != v2) {
        console.log('Test failed. Unexpected value at index 0.');
        return;
    }
    if (values[2] != v3) {
        console.log('Test failed. Unexpected value at index 0.');
        return;
    }
    console.log('Test passed.');
}, (error: Error): void => {
    console.log('Test failed. The promise should not be rejected.');
});
