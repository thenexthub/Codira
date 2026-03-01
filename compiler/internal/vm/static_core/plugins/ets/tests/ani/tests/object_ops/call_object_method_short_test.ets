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

    public shortMethod(a0: short, a1: short): short
    {
        return a0 > a1 ? a0 : a1;
    }

    public shortMethodVoidParam(): short
    {
        return 5 as short;
    }

    public shortMethodMultipleParam(a0: int, a1: boolean, a2: float, a3: short, a4: short): short
    {
        if (a1) {
            return (a3 + a4).toShort();
        } else if (a0 > a3) {
            return a3;
        } else {
            return (a3 - a4).toShort();
        }
    }

    public nestedMethod(data1: short, data2: short): short
    {
        let method = (value1: short, value2: short): short => {
            return (value1 + value2).toShort();
        }

        return method(data1, data2);
    }

    public recursionMethod(a0: short): short
    {
        if (a0 > 100) {
            return a0;
        } else {
            return this.recursionMethod((a0 + 10).toShort());
        }
    }
}

final class B extends A {

    public subIntMethodVoidParam(): short
    {
        return 8 as short;
    }

    public subMethodMultipleParam(a0: short, a1: boolean, a2:float, a3:short, a4:short): short
    {
        if (a1) {
            return (a3 + a4).toShort();
        } else if (a0 > a3) {
            return a3;
        } else {
            return (a3 - a4).toShort();
        }
    }
}

abstract class C {
    public abstract func(a: short, b: short): short;
}

class D extends C {
    public override func(a: short, b: short): short {
        return (a + b).toShort();
    }
}

class E extends D {
    public override func(a: short, b: short): short {
        return (a - b).toShort();
    }
}