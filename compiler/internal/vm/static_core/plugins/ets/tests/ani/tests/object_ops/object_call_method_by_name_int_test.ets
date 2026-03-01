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

    intByNameMethod(a0: int, a1: int): int {
        return a0 + a1 as int;
    }
}

interface ji {
    jf(value: int): int;
}

class B implements ji {
    constructor(data: int) {
        this.data = data;
    }

    data: int;

    public intMethod(value1: int, value2: int): int {
        if (this.data > value1 + value2) {
            return this.data;
        } else {
            return value1 + value2 as int;
        }
    }

    public intMethod(value: int): int {
        if (this.data > value) {
            return this.data;
        } else {
            return value;
        }
    }

    protected protectedMethod(value: int): int {
        return this.data + value as int;
    }

    private privateMethod(value: int): int {
        return this.data - value as int;
    }

    public nestedMethod(value: int): int {
        let lambdaMethod: (p1: int, p2: int) => int =
            (param1: int, param2: int): int => {
                return param1 + param2 as int;
        };
        return lambdaMethod(this.data, value);
    }

    public recursiveMethod(value: int): int {
        if (value <= 0) {
            return 1;
        } else {
            return value * this.recursiveMethod(value - 1) as int;
        }
    }

    calculateSum(value1: int, value2: char, value3: double): int {
        let aChar: char = c'A';
        if (value2 == aChar) {
            return this.data - value1 as int;
        }
        if (value3 == 1.0) {
            return value1;
        }
        return this.data + value1 as int;
    }

    public jf(value: int): int {
        return this.data - value as int;
    }
}

final class C extends B {
    constructor(data1: int) {
        super(data1);
        this.data1 = data1;
    }

    data1: int;

    public intAddMethod(value1: int, value2: int): int {
        return this.data1 + value1 + value2 as int;
    }

    public override intMethod(value1: int, value2: int): int {
        return this.data1 - value1 - value2 as int;
    }

    public callProtected(value: int): int {
        return this.protectedMethod(value) as int;
    }

    public jf(value: int): int {
        return this.data1 + value as int;
    }
}

class CheckWrongSignature {
    public method(a: String): int {
        return 1;
    }
}
