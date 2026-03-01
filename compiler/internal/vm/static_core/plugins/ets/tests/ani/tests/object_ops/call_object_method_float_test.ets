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

    public floatMethod(a0: float, a1: float): float
    {
        return a0 + a1;
    }

    public floatMethodVoidParam(): float
    {
        return 5.0f;
    }

    public floatMethodMultipleParam(a0: short, a1: boolean, a2:float, a3:short, a4:float): float
    {
        if (a1) {
            return (a2 + a4) as float;
        } else if (a0 > a3) {
            return (a2 - a4) as float;
        } else {
            return a2;
        }
    }

    public nestedMethod(data1: float, data2: float): float
    {
        let method = (value1: float, value2: float): float => {
            return (value1 + value2) as float;
        }

        return method(data1, data2);
    }

    public recursionMethod(a0: float): float
    {
        if (a0 > 100) {
            return a0;
        } else {
            return this.recursionMethod((a0 + 10.0).toFloat());
        }
    }
}

final class B extends A {

    public subFloatMethodVoidParam(): float
    {
        return 8.0f;
    }
}

abstract class C {
    public abstract func(a: float, b: float): float;
}

class D extends C {
    public override func(a: float, b: float): float {
        return (a + b) as float;
    }
}

class E extends D {
    public override func(a: float, b: float): float {
        return (a - b) as float;
    }
}
