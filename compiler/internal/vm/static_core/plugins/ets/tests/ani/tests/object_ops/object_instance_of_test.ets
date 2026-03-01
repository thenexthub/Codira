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

class A {
    public static new_A(): A {
        return new A();
    }

    public static new_A_array(): Array<A> {
        return new Array<A>();
    }

    boolean_method(a0: int, a1: int): boolean {
        if (a0 + a1 > 10) {
            return true;
        } else {
            return false;
        }
    }
}

class B extends A {
    public static new_B(): B {
        return new B();
    }

    boolean_method(a0: int, a1: int): boolean {
        if (a0*a1 > 10) {
            return true;
        } else {
            return false;
        }
    }
}

class C extends B {
    public static new_C(): C {
        return new C();
    }

    boolean_method(a0: int, a1: int): boolean {
        if (a0 - a1 > 10) {
            return true;
        } else {
            return false;
        }
    }
}

class D {
    public static new_D(): D {
        return new D();
    }

    boolean_method(a0: int): boolean {
        if (a0 > 10) {
            return true;
        } else {
            return false;
        }
    }

    public static new_Union(): D | F {
        return new D();
    }
}

enum Color {
    RED,
    GREEN,
    BLUE
}

enum Items {
    ONE,
    TWO
}

interface I {}

class F implements I {
    public static new_I(): I {
        return new F();
    }

    public static new_F(): F {
        return new F();
    }

    public static new_Union(): D | F {
        return new F();
    }

    public static new_Color(): Color {
        let c: Color = Color.BLUE
        return c;
    }

    public static new_Items(): Items {
        let i: Items = Items.ONE
        return i;
    }

    public static new_Object(): Object {
        return new Object();
    }

    /// @brief Returns boxed primitive, union is required for boxing of int
    public static new_Boxed_Primitive(): Int | String {
        return 5;
    }
}
