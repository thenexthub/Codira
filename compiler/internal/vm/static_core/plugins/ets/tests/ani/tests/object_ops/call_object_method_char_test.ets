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

    public charMethod(a0: char, a1: char): char
    {
        return a0.compareTo(a1) > 0 ? a0 : a1;
    }

    public charMethodVoidParam(): char
    {
        return c'A';
    }

    public charMethodMultipleParam(a0: byte, a1: boolean, a2: float, a3: float, a4: byte): char
    {
        if (a1) {
            return c'A';
        } else if (a2 > a3) {
            return c'B';
        } else {
            return c'C';
        }
    }

    public nestedMethod(data1: char, data2: int): char
    {
        let method = (value1: char, value2: int): char => {
            return (value1.toInt() + value2).toChar();
        }

        return method(data1, data2);
    }

    public recursionMethod(a0: char): char
    {
        if (a0.toInt() > 100) {
            return a0 as char;
        } else {
            return this.recursionMethod((a0.toInt() + 5).toChar());
        }
    }
}

final class B extends A {

    public subCharMethodVoidParam(): char
    {
        return c'W';
    }
}

abstract class C {
    public abstract func(a: char, b: byte): char;
}

class D extends C {
    public override func(a: char, b: byte): char {
        return (a.toInt() + b.toInt()).toChar();
    }
}

class E extends D {
    public override func(a: char, b: byte): char {
        return (a.toInt() - b.toInt()).toChar();
    }
}
