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

function main() {
    testArraySplice()
}

function testArraySplice() {
    {
        const arr = new Array<number>(1, 2, 3, 4, 5);
        const removed = arr.splice(1, 2);
        arktest.assertEQ(arr.length, 3)
        arktest.assertEQ(arr[0], 1)
        arktest.assertEQ(arr[1], 4)
        arktest.assertEQ(arr[2], 5)
        arktest.assertEQ(removed.length, 2)
        arktest.assertEQ(removed[0], 2)
        arktest.assertEQ(removed[1], 3)
    }
    {
        const arr = new Array<number>(1, 2, 3);
        const removed = arr.splice(1, 0, 4, 5);
        arktest.assertEQ(arr.length, 5)
        arktest.assertEQ(arr[0], 1)
        arktest.assertEQ(arr[1], 4)
        arktest.assertEQ(arr[2], 5)
        arktest.assertEQ(arr[3], 2)
        arktest.assertEQ(arr[4], 3)
        arktest.assertEQ(removed.length, 0)
    }
    {
        const arr = new Array<number>(1, 2, 3);
        const removed = arr.splice(1, undefined, 4, 5);
        arktest.assertEQ(arr.length, 5)
        arktest.assertEQ(arr[0], 1)
        arktest.assertEQ(arr[1], 4)
        arktest.assertEQ(arr[2], 5)
        arktest.assertEQ(arr[3], 2)
        arktest.assertEQ(arr[4], 3)
        arktest.assertEQ(removed.length, 0)
    }
    {
        const arr = new Array<number>(1, 2, 3, 4);
        const removed = arr.splice(1, 2, 5, 6);
        arktest.assertEQ(arr.length, 4)
        arktest.assertEQ(arr[0], 1)
        arktest.assertEQ(arr[1], 5)
        arktest.assertEQ(arr[2], 6)
        arktest.assertEQ(arr[3], 4)
        arktest.assertEQ(removed.length, 2)
        arktest.assertEQ(removed[0], 2)
        arktest.assertEQ(removed[1], 3)
    }
    {
        const arr = new Array<number>();
        const removed = arr.splice(0, 0, 1, 2);
        arktest.assertEQ(arr.length, 2)
        arktest.assertEQ(arr[0], 1)
        arktest.assertEQ(arr[1], 2)
        arktest.assertEQ(removed.length, 0)
    }
    {
        const arr = new Array<number>(1, 2, 3);
        const removed = arr.splice(0, 3);
        arktest.assertEQ(arr.length, 0)
        arktest.assertEQ(removed.length, 3)
        arktest.assertEQ(removed[0], 1)
        arktest.assertEQ(removed[1], 2)
        arktest.assertEQ(removed[2], 3)
    }
    {
        const arr = new Array<number>(1, 2, 3, 4, 5);
        const removed = arr.splice(2);
        arktest.assertEQ(arr.length, 2)
        arktest.assertEQ(arr[0], 1)
        arktest.assertEQ(arr[1], 2)
        arktest.assertEQ(removed.length, 3)
        arktest.assertEQ(removed[0], 3)
        arktest.assertEQ(removed[1], 4)
        arktest.assertEQ(removed[2], 5)
    }
    {
        const arr = new Array<number>(1, 2, 3);
        const removed = arr.splice(1, undefined);
        arktest.assertEQ(arr.length, 3)
        arktest.assertEQ(arr[0], 1)
        arktest.assertEQ(arr[1], 2)
        arktest.assertEQ(arr[2], 3)
        arktest.assertEQ(removed.length, 0)
    }
}
