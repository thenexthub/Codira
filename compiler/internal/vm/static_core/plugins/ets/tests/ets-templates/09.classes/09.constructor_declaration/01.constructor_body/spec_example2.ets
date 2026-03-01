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
    x: number = -1.0;
    y: number = -1.0;
    constructor(x: number, y: number) {
        this.x = x;
        this.y = y;
    }
}

class ColoredPoint extends Point {
    static readonly WHITE = 0;
    static readonly BLACK = 1;
    color: number;
    // primary constructor:
    constructor(x: number, y: number, color: number) {
        super(x, y); // calls base class constructor as class has 'extends'
        this.color = color;
    }
    // secondary constructor:
    constructor zero(color: number) {
        this(0, 0, color);
    }
}

function main() {
    let black = new ColoredPoint.zero(ColoredPoint.BLACK);
    arktest.assertTrue(black instanceof ColoredPoint);
    arktest.assertEQ(black.color, 1);
    arktest.assertEQ(black.x, 0.0);
    arktest.assertEQ(black.y, 0.0);

    let white = new ColoredPoint.zero(ColoredPoint.WHITE);
    arktest.assertTrue(white instanceof ColoredPoint);
    arktest.assertEQ(white.color, 0);
    arktest.assertEQ(white.x, 0.0);
    arktest.assertEQ(white.y, 0.0);
}
