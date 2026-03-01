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

/* Balls implementation */

export class Ball {
    constructor(x: float, y: float, speed: float, radius: float) {
        this.radius = radius;

        this.x = x;
        this.y = y;
        this.vx = sqrt(2).toFloat() * speed;
        this.vy = sqrt(2).toFloat() * speed;
        this.vx = Double.toFloat(sqrt(2)) * speed;
        this.vy = Double.toFloat(sqrt(2)) * speed;
    }

    move(): void {
        this.x += this.vx;
        this.y += this.vy;
    }
    left(): float { return this.x - this.radius; }
    top(): float { return this.y - this.radius; }
    right(): float { return this.x + this.radius; }
    bottom(): float { return this.y + this.radius; }

    public x: float;
    public y: float;
    public vx: float;
    public vy: float;
    public radius: float;
}

export function recomputeFrameBall(balls: Ball[], frameWidth: float, frameHeight: float): Ball[] {
    for (let i = 0; i < balls.length; i++) {
        balls[i].move();
    }

    // Compute collisions between balls
    for (let i = 0; i < balls.length; ++i) {
        for (let j = 0; j < balls.length; ++j) {
            if (i == j) continue;
            let b1 = balls[i];
            let b2 = balls[j];

            let nx: float = b2.x - b1.x;
            let ny: float = b2.y - b1.y;

            let dist: float = Double.toFloat(sqrt(nx * nx + ny * ny));
            if (dist > (b1.radius + b2.radius)) continue;

            nx = nx / dist;
            ny = ny / dist;

            let dot: float = b1.vx * nx + b1.vy * ny;
            b1.vx -= nx * 2 * dot;
            b1.vy -= ny * 2 * dot;

            let d: float = Double.toFloat(b1.radius + b2.radius - dist - 0.1);
            b1.x -= nx * d;
            b1.y -= ny * d;
        }
    }
    // Compute collisions with walls
    for (let i = 0; i < balls.length; i++) {
        let b = balls[i];
        if (b.left() <= 0 || b.right() >= frameWidth) {
            b.vx = -b.vx;
            b.x = b.left() <= 0 ? b.radius : frameWidth - b.radius;
        }
        if (b.top() <= 0 || b.bottom() >= frameHeight) {
            b.vy = -b.vy;
            b.y = b.top() <= 0 ? b.radius : frameHeight - b.radius;
        }
    }
    return balls;
}

/******************************************************************************/
/* SBody implementation */

export class Vec2 {
    constructor(x: float, y: float) {
        this.x = x;
        this.y = y;
    }
    public x: float;
    public y: float;
};

function v2add(a: Vec2, b: Vec2): Vec2 {
    return new Vec2(a.x + b.x, a.y + b.y);
}

export function v2sub(a: Vec2, b: Vec2): Vec2 {
    return new Vec2(a.x - b.x, a.y - b.y);
}

function v2dot(a: Vec2, b: Vec2): float {
    return a.x * b.x + a.y * b.y;
}

function v2scale(a: float, b: Vec2): Vec2 {
    return new Vec2(a * b.x, a * b.y);
}

export function v2len(a: Vec2): float {
    return Double.toFloat(sqrt(a.x * a.x + a.y * a.y));
}

export class SBody {
    public r: Vec2 = new Vec2(0, 0);
    public v: Vec2 = new Vec2(0, 0);
    public rad: float = 0;
    public m: float = 0;
};

function simProcessCollision(a: SBody, b: SBody): void {
    let r: Vec2 = v2sub(b.r, a.r);
    let rlen: float = v2len(r);
    let shift: float = a.rad + b.rad - rlen as float;
    if (shift < 0)
        return; // too far
    let n = v2scale(1.0f / rlen as float, r);
    let mratio: float = b.m / a.m;
    let msum: float = 1.0f + mratio;
    let dvlen: float = (2.0f * v2dot(n, v2sub(a.v, b.v)) as float / msum) as float;
    a.v = v2add(a.v, v2scale(-mratio as float * dvlen as float, n));
    b.v = v2add(b.v, v2scale(dvlen, n));
    a.r = v2add(a.r, v2scale((-mratio * shift) as float / msum as float, n));
    b.r = v2add(b.r, v2scale(shift as float / msum as float, n));
}

class Pair {
    constructor(a: float, b: float) {
        this.a = a;
        this.b = b;
    }
    public a: float = 0;
    public b: float = 0;
}

function simHitWall(ri: float, vi: float, rad: float, border: float): Pair {
    if (ri - rad < 0) {
        ri = rad;
        vi = -vi;
    }
    if (ri + rad > border) {
        ri = border - rad;
        vi = -vi;
    }
    const r = new Pair(ri, vi);
    return r;
}

export function recomputeFrameSBody(arr: SBody[], borders: Vec2): SBody[] {
    for (let i = 0; i < arr.length; ++i) {
        let b = arr[i];
        b.r = v2add(b.r, b.v);

        let refl = simHitWall(b.r.x, b.v.x, b.rad, borders.x);
        b.r.x = refl.a;
        b.v.x = refl.b;
        refl = simHitWall(b.r.y, b.v.y, b.rad, borders.y);
        b.r.y = refl.a;
        b.v.y = refl.b;

        for (let j = 0; j < i; ++j) {
            simProcessCollision(b, arr[j]);
        }
    }

    return arr;
}

function main(): int {
    let vec = new Vec2(1200, 800);

    let countSB: int = 512;
    let arrSBody = new SBody[countSB];
    let step: int = 13;
    for (let i=0; i < countSB; ++i) {
        arrSBody[i] = new SBody;
        arrSBody[i].rad = 11;
        arrSBody[i].m = 11*11;
        arrSBody[i].r.x = step *i % 1200;
        arrSBody[i].r.y = step *((step*i / 1200) + 1) as int;
        arrSBody[i].v.x = 5;
        arrSBody[i].v.y = 5;

    }

    let resArr = recomputeFrameSBody(arrSBody, vec);
    let result_hash: float = 0;
    for (let i=0; i < countSB; ++i) {
        result_hash += resArr[i].r.x + resArr[i].r.y + resArr[i].v.x + resArr[i].v.y + resArr[i].rad + resArr[i].m;
    }

    if (Float.toInt(result_hash) != 392623)
    {
        console.print("Wrong result hash: ");
        console.print(Float.toInt(result_hash));
        console.println();
        return 1;
    }
    return 0;
}
