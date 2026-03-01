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
desc: >-
    A return statement may be used in the body of a constructor if it
    does not include an expression.
---*/

enum Color {
    Red,
    Green,
    Blue,
}

class A {
    x: int

    constructor(p: Color) {
        switch (p) {
            case Color.Red:
                this.x = 11
                return
            case Color.Green:
                this.x = 22
            case Color.Blue:
                return
        }
    }
}

function main() {
    arktest.assertEQ(new A(Color.Red).x, 11)
    arktest.assertEQ(new A(Color.Green).x, 22)
    arktest.assertEQ(new A(Color.Blue).x, 0)
}
