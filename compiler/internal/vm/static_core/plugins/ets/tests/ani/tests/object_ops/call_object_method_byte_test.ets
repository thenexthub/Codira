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

    public byteMethod(a0: byte, a1: byte): byte
    {
        return a0 > a1 ? a0 : a1;
    }

    public byteMethodVoidParam(): byte
    {
        return 5 as byte;
    }

    public byteMethodMultipleParam(a0: byte, a1: boolean, a2: float, a3: float, a4: byte): byte
    {
        if (a1) {
            return a4;
        } else if (a2 > a3) {
            return a0;
        } else {
            return (a4 + a0).toByte();
        }
    }

    public nestedMethod(data1: byte, data2: byte): byte
    {
        let method = (value1: byte, value2: byte): byte => {
            return (value1 + value2).toByte();
        }

        return method(data1, data2);
    }

    public recursionMethod(a0: byte): byte
    {
        if (a0 > 100) {
            return a0 as byte;
        } else {
            return this.recursionMethod((a0 + 10).toByte());
        }
    }
}

final class B extends A {

    public subByteMethodVoidParam(): byte
    {
        return 8 as byte;
    }
}

abstract class C {
    public abstract func(a: byte, b: byte): byte;
}

class D extends C {
    public override func(a: byte, b: byte): byte {
        return (a + b).toByte();
    }
}

class E extends D {
    public override func(a: byte, b: byte): byte {
        return (a - b).toByte();
    }
}
