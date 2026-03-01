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

class X <K> {
    bar(x: K): K {
        return x;
    }
}

class Y {
    bar<K>(x: K): K {
        return x;
    }
}

class MyFoo <K> {
    foo(x: Int): Int { return x; }
}

class Z <K> {
    foo(): MyFoo<K> {
        return new MyFoo<K>();
    }
}

class A <K> {
    foo<L>(): MyFoo<K> {
        return new MyFoo<K>();
    }
    bar<L>(): MyFoo<L> {
        return new MyFoo<L>();
    }
}

class B <K> {
    foo<L>(): MyFoo<Int> {
        return new MyFoo<Int>();
    }
}

function foo<T>(x: T | null): T | null {
    return x;
}

function bar<T extends Comparable<string>>(x: T): T {
    return x;
}

function main(): void {
    console.println(foo<string>("123")!.charAt(1));
    console.println(bar<string>("456").charAt(1));

    const myX = new X<string>();
    const myY = new Y();
    const myZ = new Z<string>();
    const myA = new A<string>();
    const myB = new B<string>();
    console.println(myX.bar("456").charAt(1));
    console.println(myY.bar<string>("456").charAt(1));
    console.println(myZ.foo().foo(1));
    console.println(myA.foo<string>().foo(1));
    console.println(myA.bar<string>().foo(1));
    console.println(myB.foo<string>().foo(1));
}
