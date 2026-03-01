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
desc: 05 Generics
name: 05.generics/genericTypeArgumentWithDefaultParameterValue_06
tags: []
---*/

class UserType {
    name: string
}

class UserName {
    name : string
}

interface I<T = UserType, T1 = UserName> {
    foo(p: T) : T
}

class A : implements I {
    data: T
    data1: T1
    constructor(p: T, p1: T1) {
        this.data = p.type
        this.data1 = p1.name
    }

    foo(p : T) : T {
        return data;
    }

    bar(p : T1) : T1 {
        return data1;
    }
}

class B : extends A {}


function main(): void {
    let a1 = new A();
    let a2 = new A<UserType, UserName>();
    let a3 = new A<string, string>;

    let b1 = new B();
    let b2 = new B<UserType>();
    let b3 = new B<string>;


    let ra1 = a1.foo(new UserType(), new UserName());
    let ra2 = a2.foo(new UserType(), new UserName());
    let ra3 = a3.foo("hello", "world")

    let ra11 = a1.bar(new UserType(), new UserName());
    let ra21 = a2.bar(new UserType(), new UserName());
    let ra31 = a3.bar("hello", "world")

    arktest.assertTrue( ra1 instanceof UserType)
    arktest.assertTrue( ra2 instanceof UserType)
    arktest.assertTrue( ra3 instanceof string)

    arktest.assertTrue( ra11 instanceof UserName)
    arktest.assertTrue( ra21 instanceof UserName)
    arktest.assertTrue( ra31 instanceof string)
}

