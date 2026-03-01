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

class Phone {
    static get_button_names(): string {
        return "up";
    }

    static get_num_string(a0: int, a1: int): string {
        if (a0 == 5 && a1 == 6) {
            return "INT5"
        }
        return "NO1";
    }
}

class A {
    static funcA(a0: int, a1: int): string {
        if (a0 == 5 && a1 == 6) {
            return "INT5"
        }
        return "NO1";
    }

    static funcA(a0: double, a1: double): double {
        return a0 + a1 as double;
    }

    static funcB(a0: int, a1: int): string {
        return A.funcA(a0, a1);
    }
}
class B extends A{
    static funcB(a0: int, a1: int): string {
        if (a0 == 5 && a1 == 6) {
            return "up"
        }
        return "NO1";
    }
}

class C extends A{
}

class D extends A{
    static funcA(a0: int, a1: int): string {
        if (a0 == 5 && a1 == 6) {
            return "up"
        }
        return "NO1";
    }
}

class E extends A{
    static funcA(a0: int, a1: int): string {
        return A.funcA(a0, a1);
    }
}

class F {
    static increment(a0: int, a1: int): void {
        if (a0 == 5 && a1 == 6) {
            F.count = "INT5"
        } else {
            F.count = "NO1";
        }
    }

    static getCount(): string {
        return F.count;
    }

    static count: string = "";
}

class G {
    public static publicMethod(a0: int, a1: int): string {
        if (a0 == 5 && a1 == 6) {
            return "INT5"
        }
        return "NO1";
    }

    private static privateMethod(a0: int, a1: int): string {
        if (a0 == 5 && a1 == 6) {
            return "up"
        }
        return "NO1";
    }

    static callPrivateMethod(a0: int, a1: int): string {
        return G.privateMethod(a0, a1);
    }
}