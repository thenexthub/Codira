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

class A {}
class B {}
class C extends B {}
class D1 {
    foo(a: A | C) {
        return 1;
    }
    foo(a: A | B) {
        return 2;
    }
}
class E1 extends D1 {
    override foo(a: A | B) {
        return 3;
    }
}

interface I {}
class Base {}
class Derv extends Base implements I {}
class D2 {
    foo(a: Derv) {
        return 1;
    }
}
class E2 extends D2 {
    foo(a: Base | I) {
        return 2;
    }
}
class II implements I {}

class Derv1 extends Base {}
class Derv2 extends Derv1 {}
class Derv3 extends Derv2 {}
class Derv4 extends Derv3 {}
class Derv11 extends Derv1 {}
class D3 {
    foo(a: Derv11 | Derv4) {
        return 1;
    }
}
class E3 extends D3 {
    foo(a: Derv1) {
        return 3;
    }
}

class IBase extends Base implements I {}
class ID extends D1 implements I {}
class D4 {
    foo(a: IBase) {
        return 1;
    }
}
class E4 extends D4 {
    foo(a: I | D4) {
        return 2;
    }
}

function main(): void {
    let d1 = new D1()
    let e1 = new E1()
    arktest.assertEQ(e1.foo(new B()), 3)
    arktest.assertEQ(e1.foo(new C()), 3)
    arktest.assertEQ(d1.foo(new B()), 2)
    arktest.assertEQ(d1.foo(new C()), 1)

    let e2 = new E2()
    arktest.assertEQ(e2.foo(new Base()), 2)
    arktest.assertEQ(e2.foo(new II()), 2)
    arktest.assertEQ(e2.foo(new Derv()), 2)

    let e3 = new E3()
    arktest.assertEQ(e3.foo(new Derv1()), 3)
    arktest.assertEQ(e3.foo(new Derv11()), 3)
    arktest.assertEQ(e3.foo(new Derv4()), 3)

    let e4 = new E4()
    arktest.assertEQ(e4.foo(new ID()), 2)
    arktest.assertEQ(e4.foo(new IBase()), 2)
}