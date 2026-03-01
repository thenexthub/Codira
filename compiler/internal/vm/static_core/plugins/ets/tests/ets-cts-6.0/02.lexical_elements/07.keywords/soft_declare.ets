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
desc: The following soft keywords have special meaning in certain contexts but are
  valid identifiers elsewhere
name: soft_0
params: declare
---*/

let declare = 0;

class A {
    declare = 1
}

class B {
    readonly declare = 1
}

interface I{
    test(declare: String): String; 
}

class C implements I{
    test(declare: String): String{
        return declare;
    }
}

abstract class AD {
    readonly declare = 1
    abstract test(declare: String): String;
}

class D extends AD{
    test(declare: String): String{
        return declare;
    }
}

function main(): void{
    arktest.assertEQ(declare, 0)
    let ca = new A()
    ca.declare = 2
    arktest.assertEQ(ca.declare, 2)
    let cb = new B() 
    arktest.assertEQ(cb.declare, 1)
    let cc = new C()
    arktest.assertEQ(cc.test("test"), "test")
    let cd = new D()
    arktest.assertEQ(cd.test("test"), "test")
    arktest.assertEQ(cd.declare, 1)
}
