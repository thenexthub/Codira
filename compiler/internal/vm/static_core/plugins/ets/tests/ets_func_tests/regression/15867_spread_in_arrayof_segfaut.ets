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
desc: Spread in Array.of segfault
tags: []
---*/

class MyObj {
}

interface Branch {
}

class Leaf implements Branch {
}

function checkLength(a: Object, expected: number): boolean {
    let test = a as Array<object>;
    let cr = test.length == expected;
    return cr
}

function main(): void {
    let a1 = Array.of(...([1, 2, 3] as Int[]));
    arktest.assertTrue(checkLength(a1, 3));

    let a1_1 = Array.of<Int>(...([1, 2, 3] as Int[]));
    arktest.assertTrue(checkLength(a1_1, 3));

    let a2 = Array.of(...(["1", "2", "3"] as String[]));
    arktest.assertTrue(checkLength(a2, 3));

    let a2_1 = Array.of<String>(...(["1", "2", "3"] as String[]));
    arktest.assertTrue(checkLength(a2_1, 3));

    let a3 = Array.of(...([new MyObj(), new MyObj(), new MyObj()] as MyObj[]));
    arktest.assertTrue(checkLength(a3, 3));

    let a3_1 = Array.of<MyObj>(...([new MyObj(), new MyObj(), new MyObj()] as MyObj[]));
    arktest.assertTrue(checkLength(a3_1, 3));

    let a4 = Array.of(...([new Leaf(), new Leaf(), new Leaf()] as Branch[]));
    arktest.assertTrue(checkLength(a4, 3));

    let a4_1 = Array.of<Branch>(...([new Leaf(), new Leaf(), new Leaf()] as Branch[]));
    arktest.assertTrue(checkLength(a4_1, 3));
}
