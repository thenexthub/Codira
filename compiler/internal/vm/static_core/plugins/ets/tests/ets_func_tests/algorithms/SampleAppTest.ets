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


/*
    Sample Program which demonstrate and test ArkTS features
        as they described in ArkTS Tutorial
*/

class Point {
    public x: double;
    public y: double;
    constructor() {
        this.x = Double.NaN;
        this.y = Double.NaN;
    }
    constructor(x: double, y: double) {
        this.x = x;
        this.y = y;
    }
}


interface Figure {
    // currently not usable
    private fakeId(): int { return 13; }
    getOrigin(): Point;
    getWidth(): double;
    getHeight(): double;
    // default realisation to test override
    id(): int { return 13; };
    intersects(other: Figure | null): boolean { return false; };
}


class Rectangle implements Figure {
    static total: int = 0;
    start: Point | null;  // bottom-left
    end: Point | null;  // top-right
    constructor() {
        this.start = null;
        this.end = null;
        Rectangle.total++;
    }
    constructor(start: Point, end: Point) {
        this.start = start;
        this.end = end;
        Rectangle.total++;
    }
    override getOrigin(): Point {
        return this.start ?? new Point();
    }
    override getWidth(): double {
        let s = this.start ?? new Point();
        let e = this.end ?? new Point();
        return abs(e.x - s.x);
    }
    override getHeight(): double {
        let s = this.start ?? new Point();
        let e = this.end ?? new Point();
        return abs(e.y - s.y);
    }
}


final class Square extends Rectangle {
    constructor() {
        super();
    }
    constructor(start: Point, side: double) {
        // TODO: start?.x
        super(start, new Point(start.x + side, start.y + side));
    }
    // Figure::intersects(other: Figure | null): boolean
    override intersects(other: Rectangle | null): boolean {
        if (null === other) {
            return false;
        }
        if (this.end.y < other.start.y
            || this.start.y > other.end.y) {
            return false;
        }
        if (this.end.x < other.start.x 
            || this.start.x > other.end.x) {
            return false;
        }
        return true;
    }
}

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name);
        return 0;
    }
    console.println("FAILED: " + name);
    return 1;
}

function default_ctor(): boolean {
    let total: int = Rectangle.total;
    let r = new Rectangle();
    return (Rectangle.total == total + 1)
        && (Double.isNaN(r.getOrigin().x))
        && (Double.isNaN(r.getOrigin().y))
        && (Double.isNaN(r.getWidth()))
        && (Double.isNaN(r.getHeight()))
        && (r.id() == 13);
}

function rect_ctor(): boolean {
    let r = new Rectangle(new Point(5, 10), new Point(6, 12));
    return (5 == r.getOrigin().x)
        && (10 == r.getOrigin().y)
        && (1 == r.getWidth())
        && (2 == r.getHeight())
        && (r.id() == 13);
}

function default_iface_method(): boolean {
    let r1 = new Rectangle();
    let r2 = new Rectangle();
    return !r1.intersects(r2);
}

function square_ctor(): boolean {
    let r = new Square(new Point(5, 10), 8);
    return (5 == r.getOrigin().x)
        && (10 == r.getOrigin().y)
        && (8 == r.getWidth())
        && (8 == r.getHeight())
        && (r.id() == 13);
}

function square_override(): boolean {
    let s1 = new Square(new Point(0, 0), 10);
    let s2 = new Square(new Point(-5, -5), 100);
    let r1 = new Rectangle(new Point(1, 2), new Point(2, 4));
    let r2 = new Rectangle(new Point(-2, -2), new Point(-1, -1));
    return (s1.intersects(s2))
        && (s1.intersects(r1))
        && (!s1.intersects(r2));
}

function array_of_squares(): boolean {
    let a: Square[] = new Square[32];
    let ok: boolean = true;
    for (let i = 0; i < a.length; i++) {
        a[i] = new Square(new Point(2.0*i, 2.0*i), 3);
    }
    for (let i = 0; i < a.length - 1; i++) {
        ok = ok && a[i].intersects(a[i+1]);
    }
    return ok;
}

function main(): int {
    let failures: int = 0;
    failures += test(default_ctor(), "Sample App default ctor");
    failures += test(rect_ctor(), "Sample App super ctor");
    failures += test(default_iface_method(), "Sample App default iface method");
    failures += test(square_ctor(), "Sample App extend ctor");
    failures += test(square_override(), "Sample App override method");
    failures += test(array_of_squares(), "Sample App array of user types");
    if (failures == 0) {
        console.println("PASSED: All tests run");
    } else {
        console.println("FAILED: All tests run");
    }
    return failures;
}
