/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

class H {};
class F extends H {};

type Ton = Object | null;
type Tou = Object | undefined;
type Tonu = Object | null | undefined;
type Tin = Int | null;
type Tiu = Int | undefined;
type Tfn = F | null;
type Tfu = F | undefined;
type Tnu = null | undefined;
type Tall = Object | Int | F | null | undefined;

function isObject(a: Tall) {
    return a instanceof Object;
}

function isTon(a: Tonu) {
    return a instanceof Ton;
}

function isTou(a: Tonu) {
    return a instanceof Tou;
}

function isTonu(a: Tonu) {
    return a instanceof Tonu;
}

function isTnu(a: Tonu) {
    return a instanceof Tnu;
}

function isTfn(a: Tonu) {
    return a instanceof Tfn;
}

function isTfu(a: Tfn | undefined) {
    return a instanceof Tfu;
}

function isTin(a: Tiu | null) {
    return a instanceof Tin;
}

function main() {
    // CC-OFFNXT(G.DCL.06) for test purposes
    let o = new Object();
    let i = new Int(123);
    let s = new String('123');
    let f = new F();
    let h = new H();
    let n = null;
    let u = undefined;

    arktest.assertEQ(isObject(o), true);
    arktest.assertEQ(isObject(i), true);
    arktest.assertEQ(isObject(s), true);
    arktest.assertEQ(isObject(f), true);
    arktest.assertEQ(isObject(h), true);
    arktest.assertEQ(isObject(n), false);
    arktest.assertEQ(isObject(u), false);

    arktest.assertEQ(isTon(o), true);
    arktest.assertEQ(isTon(i), true);
    arktest.assertEQ(isTon(s), true);
    arktest.assertEQ(isTon(f), true);
    arktest.assertEQ(isTon(h), true);
    arktest.assertEQ(isTon(n), true);
    arktest.assertEQ(isTon(u), false);

    arktest.assertEQ(isTou(o), true);
    arktest.assertEQ(isTou(i), true);
    arktest.assertEQ(isTou(s), true);
    arktest.assertEQ(isTou(f), true);
    arktest.assertEQ(isTou(h), true);
    arktest.assertEQ(isTou(n), false);
    arktest.assertEQ(isTou(u), true);

    arktest.assertEQ(isTonu(o), true);
    arktest.assertEQ(isTonu(i), true);
    arktest.assertEQ(isTonu(s), true);
    arktest.assertEQ(isTonu(f), true);
    arktest.assertEQ(isTonu(h), true);
    arktest.assertEQ(isTonu(n), true);
    arktest.assertEQ(isTonu(u), true);

    arktest.assertEQ(isTnu(o), false);
    arktest.assertEQ(isTnu(i), false);
    arktest.assertEQ(isTnu(s), false);
    arktest.assertEQ(isTnu(f), false);
    arktest.assertEQ(isTnu(h), false);
    arktest.assertEQ(isTnu(n), true);
    arktest.assertEQ(isTnu(u), true);

    arktest.assertEQ(isTfn(o), false);
    arktest.assertEQ(isTfn(i), false);
    arktest.assertEQ(isTfn(s), false);
    arktest.assertEQ(isTfn(f), true);
    arktest.assertEQ(isTfn(h), false);
    arktest.assertEQ(isTfn(n), true);
    arktest.assertEQ(isTfn(u), false);

    arktest.assertEQ(isTfu(f), true);
    arktest.assertEQ(isTfu(n), false);
    arktest.assertEQ(isTfu(u), true);

    arktest.assertEQ(isTin(i), true);
    arktest.assertEQ(isTin(n), true);
    arktest.assertEQ(isTin(u), false);
}
