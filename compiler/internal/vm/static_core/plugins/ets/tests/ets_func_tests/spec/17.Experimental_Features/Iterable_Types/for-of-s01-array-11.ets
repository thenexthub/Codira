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
    let index : int = 0;
    let tmp : Object;
    let initialArray : Object[]  = [new Int(1), new Int(3), new Int(5)];
    let expectedArray : Object[]= [new Int(1), new Int(3), new Int(5)];
    let result : int = 0;


    index = 0;
    for (tmp of initialArray) {
        if (tmp != expectedArray[index]) {
            return 1;
        }
        index++;
    }

    arktest.assertEQ(result, 0)
    return 0;
}



