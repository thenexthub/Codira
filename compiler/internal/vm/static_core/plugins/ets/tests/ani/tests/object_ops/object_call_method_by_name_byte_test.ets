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

    byteByNameMethod(a0: byte, a1: byte): byte {
        return (a0 + a1).toByte();
    }
}

interface ji {
    jf(value: byte): byte;
}

class B implements ji {
    constructor(data: byte) {
        this.data = data;
    }

    data: byte;

    public byteMethod(value1: byte, value2: byte): byte {
        if (this.data > value1 + value2) {
            return this.data;
        } else {
            return (value1 + value2).toByte();
        }
    }

    public byteMethod(value: byte): byte {
        if (this.data > value) {
            return this.data;
        } else {
            return value;
        }
    }

    protected protectedMethod(value: byte): byte {
        return (this.data + value).toByte();
    }

    private privateMethod(value: byte): byte {
        return (this.data - value).toByte();
    }

    public nestedMethod(value: byte): byte {
        let lambdaMethod: (p1: byte, p2: byte) => byte =
            (param1: byte, param2: byte): byte => {
                return (param1 + param2).toByte();
        };
        return lambdaMethod(this.data, value);
    }

    public recursiveMethod(value: int): byte {
        if (value <= 0) {
            return 1;
        } else {
            return (value * this.recursiveMethod(value - 1)).toByte();
        }
    }

    calculateSum(value1: byte, value2: char, value3: int): byte {
        let aChar: char = c'A';
        if (value2 == aChar) {
            return (this.data - value1).toByte();
        }
        if (value3 == 1) {
            return value1;
        }
        return (this.data + value1).toByte();
    }

    public jf(value: byte): byte {
        return (this.data - value).toByte();
    }
}

final class C extends B {
    constructor(data1: byte) {
        super(data1);
        this.data1 = data1;
    }

    data1: byte;

    public byteAddMethod(value1: byte, value2: byte): byte {
        return (this.data1 + value1 + value2).toByte();
    }

    public override byteMethod(value1: byte, value2: byte): byte {
        return (this.data1 - value1 - value2).toByte();
    }

    public callProtected(value: byte): byte {
        return this.protectedMethod(value) as byte;
    }

    public jf(value: byte): byte {
        return (this.data1 + value).toByte();
    }
}

class CheckWrongSignature {
    public method(a: String): byte {
        return 1;
    }
}
