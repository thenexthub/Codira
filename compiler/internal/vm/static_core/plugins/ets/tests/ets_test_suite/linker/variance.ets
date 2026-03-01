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

class StubError extends Error { }

function stub() {
    throw new StubError();
}
function stubt<T>(): T {
    stub();
    return new Object() as T;
}
function assert_stub(f: () => void) {
    try {
        f();
    } catch (e) {
        if (e instanceof StubError) {
            return;
        }
    }
    arktest.assertTrue(false, "error expected");
}

class T0 { }
class T1 extends T0 { }
class T2 extends T1 { }

// NOTE(vpukhov): cover the rest subtyping cases
class A1 {
    p1(a: T2): void { stub() }
    n1(a: T1): void { stub() }
    p2(): T1 { return stubt<T1>() }
    // n2(): T2 { return stubt<T2>() }
}
class A2 extends A1 {
    static create(): A1 { return new A2() }

    n1(a: T2): void { }
    p1(a: T1): void { }
    p2(): T2 { return new T2() }
    // n2(): T1 { return new T1() }
}
function simple() {
    let a: A1 = A2.create();
    let t1 = new T1();
    let t2 = new T2();
    assert_stub(() => { stub() })

    a.p1(t2); assert_stub(() => { a.n1(t1) });
    a.p2();
}

class B1 {
    p1(a: B2): void { stub() }
    n1(a: B1): void { stub() }
    p2(): B1 { return stubt<B1>() }
}
class BInterm extends B1 { }
class B2 extends BInterm {
    static create(): B1 { return new B2() }

    n1(a: B2): void { }
    p1(a: B1): void { }
    p2(): B2 { return new B2() }
}
function loops() {
    let b: B1 = B2.create();
    let b1 = new B1();
    let b2 = new B2();
    assert_stub(() => { stub() })

    b.p1(b2); assert_stub(() => { b.n1(b1) });
    b.p2();
}

class C1 {
    m(a: T2): void { stub() }
    m(a: T1): void { stub() }
    m(a: T0): void { stub() }
}
class C2 extends C1 {
    static create(): C1 { return new C2() }

    m(a: T1): void { }
}
function overrideMultiple() {
    let c: C1 = C2.create();

    assert_stub(() => { c.m(new T0()) });
    c.m(new T1());
    c.m(new T2());
}

function main() {
    simple();
    loops();
    overrideMultiple();
}
