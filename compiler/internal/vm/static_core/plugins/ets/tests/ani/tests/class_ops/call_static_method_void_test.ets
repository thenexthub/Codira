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

class Operations {
    static voidMethod(a0: int, a1: int): void {
        Operations.count += a0;
        Operations.count += a1;
    }

    static getCount(): int {
        return Operations.count;
    }

    static count: int = 0;
}

class A {
    static funcA(a0: int, a1: int): void {
        A.count += a0;
        A.count += a1;
    }

    static funcA(a0: double, a1: double): double {
        return a0 + a1 as double;
    }

    static funcB(a0: int, a1: int): void {
        return A.funcA(a0, a1);
    }

    static getCount(): int {
        return A.count;
    }

    static count: int = 0;
}

class B extends A{
    static funcB(a0: int, a1: int): void {
        B.count += a1;
        B.count -= a0;
    }

    static getCount(): int {
        return B.count;
    }

    static count: int = 0;
}

class C extends A{
}

class D extends A{
    static funcA(a0: int, a1: int): void {
        A.count = 0;
        A.count += a1;
        A.count -= a0;
    }
}

class E extends A{
    static funcA(a0: int, a1: int): void {
        A.funcA(a0, a1);
    }
}

class F {
    static increment(a0: int, a1: int): void {
        F.count = 0;
        F.count += a0;
        F.count += a1;
    }

    static getCount(): int {
        return F.count;
    }

    static count: int = 0;
}

class G {
    public static publicMethod(a0: int, a1: int): void {
        G.count = 0;
        G.count += a1;
        G.count += a0;
    }

    private static privateMethod(a0: int, a1: int): void {
        G.count = 0;
        G.count += a1;
        G.count -= a0;
    }

    static callPrivateMethod(a0: int, a1: int): void {
        G.privateMethod(a0, a1);
    }

    static getCount(): int {
        return G.count;
    }

    static count: int = 0;
}