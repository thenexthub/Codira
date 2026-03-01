/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: Postfix increment expression
---*/


class A {
    n: number
    d: double
    f: float
    l: long
    i: int
    s: short
    b: byte
}

class B {
    private _n: number = 1.0;
    get n(): number { return this._n }
    set n(v: number) { this._n = v }

    private _d: double = 2.0;
    get d(): double { return this._d }
    set d(v: double) { this._d = v }

    private _f: float = 3.0f;
    get f(): float { return this._f }
    set f(v: float) { this._f = v }

    private _l: long = 4;
    get l(): long { return this._l }
    set l(v: long) { this._l = v }

    private _i: int = 5;
    get i(): int { return this._i }
    set i(v: int) { this._i = v }

    private _s: short = 6;
    get s(): short { return this._s }
    set s(v: short) { this._s = v }

    private _b: byte = 7;
    get b(): byte { return this._b }
    set b(v: byte) { this._b = v }

}

function main(): void {

    let a = new A
    arktest.assertEQ(a.n++, 0.0)
    arktest.assertEQ(a.n, 1.0)

    arktest.assertEQ(a.d++, 0.0)
    arktest.assertEQ(a.d, 1.0)

    arktest.assertEQ(a.f++, 0.0f)
    arktest.assertEQ(a.f, 1.0f)

    arktest.assertEQ(a.l++, 0)
    arktest.assertEQ(a.l, 1)

    arktest.assertEQ(a.i++, 0)
    arktest.assertEQ(a.i, 1)

    arktest.assertEQ(a.s++, 0)
    arktest.assertEQ(a.s, 1)

    arktest.assertEQ(a.b++, 0)
    arktest.assertEQ(a.b, 1)
  
    // class field via get/set
    let b = new B
    arktest.assertEQ(b.n++, 1.0)
    arktest.assertEQ(b.n, 2.0)

    arktest.assertEQ(b.d++, 2.0)
    arktest.assertEQ(b.d, 3.0)

    arktest.assertEQ(b.f++, 3.0f)
    arktest.assertEQ(b.f, 4.0f)

    arktest.assertEQ(b.l++, 4)
    arktest.assertEQ(b.l, 5)

    arktest.assertEQ(b.i++, 5)
    arktest.assertEQ(b.i, 6)

    arktest.assertEQ(b.s++, 6)
    arktest.assertEQ(b.s, 7)

    arktest.assertEQ(b.b++, 7)
    arktest.assertEQ(b.b, 8)

}
