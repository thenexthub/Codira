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

class ExportClass {
    get memA(): string {
        return 'memAStr';
    }
    memB(): string {
        return 'memBStr';
    }

    memC: string = '';
}

let inst = new ExportClass();
inst.memC = 'memCStr';
console.log(`ret ${inst.memA}, ${inst.memB()}, ${inst.memC}`);


class Base {
    get field(): Base {
        console.log('Base get')
        return new Base
    }
    set field(a_field: Derived) {
        console.log('Base set')
    }
}
class Derived extends Base {
    override get field(): Derived {
        console.log('Derived get')
        return new Derived
    }
    override set field(a_field: Base) {
        console.log('Derived set')
    }
}
function foo (base: Base) {
    base.field = new Derived // setter is called
    let b: Base = base.field // getter is called
}
foo (new Derived)


class Point {
    x: number
    y: number
    constructor(x: number, y: number) {
        this.x = x
        this.y = y
    }
}
class ColoredPoint extends Point {
    static readonly WHITE = 0
    static readonly BLACK = 1
    color: number

    constructor(x: number, y: number, color: number) {
        super(x, y) // calls base class constructor
        this.color = color
    }

    constructor zero(color: number) {
        this(0, 0, color)
    }
}
class BWPoint extends ColoredPoint {
    constructor(x: number, y: number, black: boolean) {
        super (x, y, black ? ColoredPoint.BLACK : ColoredPoint.WHITE)
    }
}
let pt1 = new BWPoint(2, 3, false);
let pt2 = new ColoredPoint(3);
pt1.x = 0;
pt2.y = 1;
console.log('BWPoint', pt1.color + pt2.color);

export abstract class AbstractClass {};

function getUnion(x: int): Point | Base {
     if (x > 1) {
         return new Point(1, 1);
     }
     return new Base();
}