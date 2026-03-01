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
    Mandatory call to a superconstructor if a class has an extension clause
    on all execution paths of the constructor body.
---*/

class Point {
    x: number;
    y: number;
    constructor(x: number, y: number) {
        this.x = x;
        this.y = y;
    }
}

class ColoredPoint extends Point {
    static readonly WHITE = 0;
    static readonly BLACK = 1;
    color: number;
    constructor(x: number, y: number, color: number) {
        super(x, y); // calls base class constructor
        this.color = color
    }
}

class BWPoint extends ColoredPoint {
    constructor(x: number, y: number, black: boolean) {
        if (black) {
            super(x, y, ColoredPoint.BLACK);
        } else {
            super(x, y, ColoredPoint.WHITE);
        }
    }
}

function main() {
    let black = new BWPoint(1.0, 11.0, true)
    arktest.assertTrue(black instanceof BWPoint)
    arktest.assertEQ(black.color, 1)
    arktest.assertEQ(black.x, 1.0)
    arktest.assertEQ(black.y, 11.0)

    let white = new BWPoint(2.0, 22.0, false)
    arktest.assertTrue(white instanceof BWPoint)
    arktest.assertEQ(white.color, 0)
    arktest.assertEQ(white.x, 2.0)
    arktest.assertEQ(white.y, 22.0)
}
