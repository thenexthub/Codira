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
    static or(a0: boolean, a1: boolean): boolean {
        return a0 || a1;
    }
}

class A {
    static funcA(a0: boolean, a1: boolean): boolean {
        return a0 || a1;
    }

    static funcA(a0: int, a1: int): int {
        return a0 + a1 as int;
    }

    static funcB(a0: boolean, a1: boolean): boolean {
        return A.funcA(a0, a1);
    }
}

class B extends A{
    static funcB(a0: boolean, a1: boolean): boolean {
        return a0 && a1;
    }
}

class C extends A{
}

class D extends A{
    static funcA(a0: boolean, a1: boolean): boolean {
        return a0 && a1;
    }
}

class E extends A{
    static funcA(a0: boolean, a1: boolean): boolean {
        return A.funcA(a0, a1);
    }
}

class F {
    static increment(a0: boolean, a1: boolean): void {
        F.count &= a0;
        F.count &= a1;
    }

    static getCount(): boolean {
        return F.count;
    }

    static count: boolean = false;
}

class G {
    public static publicMethod(a0: boolean, a1: boolean): boolean {
        return a0 || a1;
    }

    private static privateMethod(a0: boolean, a1: boolean): boolean {
        return a0 && a1;
    }

    static callPrivateMethod(a0: boolean, a1: boolean): boolean {
        return G.privateMethod(a0, a1);
    }
}

let parentMethodInParentWasCalled: int = 1;
let childMethodInChildWasCalled: int = 2;
let overridedMethodInChildWasCalled: int = 3;
let overridedMethodInParentWasCalled: int = 4;

class Parent {
    static methodChecker: int = 0;
    static getCheckerValue(): int {
        return Parent.methodChecker;
    }

    static parentStaticMethod(): boolean {
        Parent.methodChecker = parentMethodInParentWasCalled;
        return true;
    }
    static shadowingStaticMethod(): boolean {
        Parent.methodChecker = overridedMethodInParentWasCalled;
        return true;
    }
}
class Child extends Parent {
    static childStaticMethod(): boolean {
        Child.methodChecker = childMethodInChildWasCalled;
        return true;
    }
    static shadowingStaticMethod(): boolean {
        Child.methodChecker = overridedMethodInChildWasCalled;
        return true;
    }
}