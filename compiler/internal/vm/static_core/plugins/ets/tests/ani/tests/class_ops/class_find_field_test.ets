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

class Point {
    x: int;
    y: int;
    getRadius: (p: Point) => double = (p: Point) => Math.sqrt(p.x * p.x + p.y * p.y);
}

class ExtendedPoint extends Point {
    public z: int;
}

class Vector {
    private p1: Point | ExtendedPoint | null = null;
    private p2: Point | ExtendedPoint | null = null;
    readonly length: number;
}

interface IFigure {
    points: ExtendedPoint[];
    vectors: Vector[];
}

class Circle implements IFigure {
    points: ExtendedPoint[] = [];
    vectors: Vector[] = [];
    protected radius: Vector = new Vector();
    protected center: Point | ExtendedPoint = new Point();
}

class Sphere extends Circle {}

class Space {
    objects: [Circle, Sphere, Vector, ExtendedPoint, Point];
    private dimension: int;
    constructor(objects: [Circle, Sphere, Vector, ExtendedPoint, Point]) {
        this.objects = objects
    }
    name?: string;
}

class UniversalFigureHandler {
    figure: Partial<IFigure>;
    constructor(figure: Partial<IFigure>) {
        this.figure = figure;
    }
}

class NamedSpaceHandler {
    space: Required<Space>;
    constructor(space: Required<Space>) {
        this.space = space;
    }
}

class Parent {
    parentField: int;
    overridedField: int;
}
class Child extends Parent {
    childField: int;
    overridedField: int;
}