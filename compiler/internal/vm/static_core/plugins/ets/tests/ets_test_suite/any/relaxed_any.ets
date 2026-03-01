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

function xassert(v: boolean) { arktest.assertTrue(v) }

function expectError(action: () => void) {
    try { action() } catch (e) { return }
    throw new Error("Error expected, but nothing was thrown")
}

class X {
    prop: string = "ooooooo"
    foo() { return this.prop }
    bar(a: string) { return this.prop + a }
}

function testldstname() {
    let x = new X();
    ((a: any) => a.prop = "ttt")(x);
    xassert(((a: any) => a.prop)(x) == "ttt");

    // expectError(() => { ((a: any) => a.prop = 1)(x) }); // exception is not thrown
    expectError(() => { ((a: any) => a.nope)(x) });
    expectError(() => { ((a: any) => a.nope = 1)(x) });
}

function testldstval() {
    let x = new X();
    ((a: any) => a["prop"] = "uuu")(x);
    xassert(((a: any) => a["prop"])(x) == "uuu");

    // expectError(() => { ((a: any) => a["prop"] = 1)(x) }); // exception is not thrown
    expectError(() => { ((a: any) => a["nope"])(x) });
    expectError(() => { ((a: any) => a["nope"] = 1)(x) });
}

function testldstidx() {
    let x: Array<Any> = [undefined, 21, "ay"];
    ((a: any) => a[1] = "aaa")(x);
    xassert(((a: any) => a[1])(x) == "aaa");

    // expectError(() => { ((a: any) => a[-1])(x) }); // nothing happens
    // expectError(() => { ((a: any) => a[-1] = "aaa")(x) }); // nothing happens
    // expectError(() => { ((a: any) => a[1.5])(x) }); // nothing happens
}

function testcallthis() {
    let x = new X();
    ((a: any) => a["prop"] = "ya")(x);
    xassert(((a: any) => a.foo())(x) == "ya");
    xassert(((a: any) => a.bar("ee"))(x) == "yaee");
}

function testcall() {
    xassert(((a: any) => a())(() => "aaa") == "aaa");
    xassert(((a: any) => a("bbb"))((v: string) => v) == "bbb");
    xassert(((a: any) => a(123.0))((v: number) => v) == 123.0);

    expectError(() => { ((a: any) => a(1))((s: string) => s) });
}

function testcasts() {
    let x = new X()
    xassert(((a: any) => a as X)(x) == x)
    xassert(((a: any) => a instanceof X)(x) == true)
}

function test() {
    testldstval();
    testldstidx();
    testldstname();
    testcall();
    // testcallthis(); // does not work
    testcasts();
}
test()
