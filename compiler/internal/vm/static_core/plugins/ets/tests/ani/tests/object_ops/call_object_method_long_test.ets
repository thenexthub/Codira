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
    public static newA()
    {
        return new A();
    }

    public longMethod(a: long, b: long): long {
        return a + b;
    }

    public longMethodVoidParam(): long
    {
        return 5 as long;
    }

    public longMethodMultipleParam(a0: long, a1: boolean, a2:float, a3:long, a4:float): long
    {
        if (a1) {
            return a0 + a3;
        } else if (a4 > a2) {
            return a0;
        } else {
            return a0 - a3;
        }
    }

    public nestedMethod(data1: long, data2: long): long
    {
        let method = (value1: long, value2: long): long => {
            return value1 + value2;
        }

        return method(data1, data2);
    }

    public recursionMethod(a0: long): long
    {
        if (a0 > 100) {
            return a0;
        } else {
            return this.recursionMethod(a0 + 10);
        }
    }
}

final class B extends A {

    public subLongMethodVoidParam(): long
    {
        return 8 as int;
    }

    public subMethodMultipleParam(a0: long, a1: boolean, a2:float, a3:long, a4:float): long
    {
        if (a1) {
            return a0 + a3;
        } else if (a4 > a2) {
            return a0;
        } else {
            return a0 - a3;
        }
    }
}

abstract class C {
    public abstract func(a: long, b: long): long;
}

class D extends C {
    public override func(a: long, b: long): long {
        return (a + b) as long;
    }
}

class E extends D {
    public override func(a: long, b: long): long {
        return (a - b) as long;
    }
}