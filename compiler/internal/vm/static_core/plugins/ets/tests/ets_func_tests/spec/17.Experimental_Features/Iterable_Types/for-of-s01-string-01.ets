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

function main() {
    let ch : string = 'z';
    let index : int = 0;
    let result : int = 0;
    let testStr : String = "a string object";
    let testArray : string[] = ['a', ' ', 's', 't', 'r', 'i', 'n', 'g', ' ', 'o', 'b', 'j', 'e', 'c', 't'];
    for (ch of testStr) {
        if (ch != testArray[index]) {
            console.log("Incorrect element at index : " + index);
            result++;
        }
        index++;
    }

    arktest.assertEQ(index, testStr.length)
    arktest.assertEQ(result, 0)
}

