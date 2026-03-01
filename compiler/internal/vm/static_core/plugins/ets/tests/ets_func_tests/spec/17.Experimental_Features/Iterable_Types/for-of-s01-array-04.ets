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
desc: 8.9 for-of Statements. 17.5 - iterable
tags: []
---*/

class Bar {
    public index: int = 0;
    constructor(i : int) {
        this.index = i;
    }
}

function main() : int {
    let idx : int = 0;
    let result : int = 0;
    // TODO clarify why not
    // let bar : Bar = new Bar(100000);
    let barArray : Bar[] = [new Bar(2), new Bar(3), new Bar(4), new Bar(5), new Bar(6), new Bar(7)];
    let expectedArray : int[] = [2, 3, 4, 5, 6, 7];
    for (let bar : Bar of barArray) {
        if (bar.index != expectedArray[idx]) {
            console.log("Incorrect element at index : " + idx);
            result++;
        }
        idx++;
    }

    arktest.assertEQ(idx, expectedArray.length)
    arktest.assertEQ(result, 0)
    return 0;
}

