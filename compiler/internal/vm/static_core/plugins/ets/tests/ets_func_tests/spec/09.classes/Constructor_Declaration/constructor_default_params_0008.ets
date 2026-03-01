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
desc: 09.9 Constructor Declaration
tags: []
---*/

class Y {
    data1 : int;
    data2 : int;

    constructor(p1: int = 0, p2: int = 1) {
        this.data1 = p1;
        this.data2 = p2;
    }
}

class X {
    data : Y;
    constructor (y:Y = new Y()) {
        this.data = y
    }
}

function main() : int {
    let x : X = new X();
    arktest.assertEQ( x.data.data1, 0)
    arktest.assertEQ( x.data.data2, 1)
    return 0;
}
