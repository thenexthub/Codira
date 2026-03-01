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

name: issue-13330_3.ets
tags: []
---*/

let accessCounter = 0;

class Base {

    set base(value: int) {
        accessCounter++;
    }
}

class Derive extends Base {

    set derive(value: int) {
        this.base = value
        accessCounter++;
    }
}


function main(): void {
    accessCounter = 0;
    let vv = new Derive();
    vv.base = 42;
    arktest.assertEQ( accessCounter, 1)

    vv.derive = 43;
    arktest.assertEQ( accessCounter, 3)

}
