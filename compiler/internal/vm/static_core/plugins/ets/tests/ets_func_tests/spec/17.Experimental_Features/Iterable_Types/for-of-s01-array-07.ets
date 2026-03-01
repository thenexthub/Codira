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

/*---
desc: 8.9 for-of Statements. 17.4,17.5.1 - iterable
tags: []
---*/

function main() : int {
    let tmp : int[];
    let index : int = 0;
    let initialArray : int[][]= [[1,2],[3,4],[5,6]];
    let expectedArray : int[][]= [[1,2],[3,4],[5,6]];
    let result : int = 0;


    index = 0;
    for (tmp of initialArray) {
        if (!compareArray(tmp, expectedArray[index])) {
            console.log("Incorrect content at index " + index)
            result++;
        }

        index++;
    }

    arktest.assertEQ(result, 0)
    return 0;
}

function compareArray(left : int [], right : int []) : boolean {
    if (left.length != right.length) {
        console.log("Wrong size " + " left " + left.length + " right " + right.length);
        return false;
    }

    for (let i = 0; i < left.length; i++) {
        if (left[i] != right[i]) {
            return false;
        }
    }

    return true;
}

