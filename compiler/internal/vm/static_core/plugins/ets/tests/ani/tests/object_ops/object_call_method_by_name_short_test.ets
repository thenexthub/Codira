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

    shortByNameMethod(a0: short, a1: short): short {
        return (a0 + a1).toShort();
    }
}

interface ji {
    jf(value: short): short;
}

class B implements ji {
    constructor(data: short) {
        this.data = data;
    }

    data: short;

    public shortMethod(value1: short, value2: short): short {
        if (this.data > value1 + value2) {
            return this.data;
        } else {
            return (value1 + value2).toShort();
        }
    }

    public shortMethod(value: short): short {
        if (this.data > value) {
            return this.data;
        } else {
            return value;
        }
    }

    protected protectedMethod(value: short): short {
        return (this.data + value).toShort();
    }

    private privateMethod(value: short): short {
        return (this.data - value).toShort();
    }

    public nestedMethod(value: short): short {
        let lambdaMethod: (p1: short, p2: short) => short =
            (param1: short, param2: short): short => {
                return (param1 + param2).toShort();
        };
        return lambdaMethod(this.data, value);
    }

    public recursiveMethod(value: int): short {
        if (value <= 0) {
            return 1;
        } else {
            return (value * this.recursiveMethod(value - 1)).toShort();
        }
    }

    calculateSum(value1: short, value2: char, value3: double): short {
        let aChar: char = c'A';
        if (value2 == aChar) {
            return (this.data - value1).toShort();
        }
        if (value3 == 1.0) {
            return value1;
        }
        return (this.data + value1).toShort();
    }

    public jf(value: short): short {
        return (this.data - value).toShort();
    }
}

final class C extends B {
    constructor(data1: short) {
        super(data1);
        this.data1 = data1;
    }

    data1: short;

    public shortAddMethod(value1: short, value2: short): short {
        return (this.data1 + value1 + value2).toShort();
    }

    public override shortMethod(value1: short, value2: short): short {
        return (this.data1 - value1 - value2).toShort();
    }

    public callProtected(value: short): short {
        return this.protectedMethod(value) as short;
    }

    public jf(value: short): short {
        return (this.data1 + value).toShort();
    }
}

class CheckWrongSignature {
    public method(a: String): short {
        return 1;
    }
}
