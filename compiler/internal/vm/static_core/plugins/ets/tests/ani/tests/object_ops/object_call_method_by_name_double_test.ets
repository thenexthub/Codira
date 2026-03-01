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

	public doubleMethod(value1: double, value2: double): double
    {
        return value1 + value2;
    }
}

interface ji {
    jf(value: double): double;
}

class B implements ji {
    constructor(data: double) {
        this.data = data;
    }

    data: double;

    public doubleMethod(value1: double, value2: double): double {
        if (this.data > value1 + value2) {
            return this.data;
        } else {
            return value1 + value2 as double;
        }
    }

    public doubleMethod(value: double): double {
        if (this.data > value) {
            return this.data;
        } else {
            return value;
        }
    }

    protected protectedMethod(value: double): double {
        return this.data + value as double;
    }

    private privateMethod(value: double): double {
        return this.data - value as double;
    }

    public nestedMethod(value: double): double {
        let lambdaMethod: (p1: double, p2: double) => double =
            (param1: double, param2: double): double => {
                return param1 + param2 as double;
        };
        return lambdaMethod(this.data, value);
    }

    public recursiveMethod(value: int): double {
        if (value <= 0) {
            return 1;
        } else {
            return value * this.recursiveMethod(value - 1) as double;
        }
    }

    calculateSum(value1: double, value2: char, value3: int): double {
        let aChar: char = c'A';
        if (value2 == aChar) {
            return this.data - value1 as double;
        }
        if (value3 == 1) {
            return value1;
        }
        return this.data + value1 as double;
    }

    public jf(value: double): double {
        return this.data - value as double;
    }
}

final class C extends B {
    constructor(data1: double) {
        super(data1);
        this.data1 = data1;
    }

    data1: double;

    public doubleAddMethod(value1: double, value2: double): double {
        return this.data1 + value1 + value2 as double;
    }

    public override doubleMethod(value1: double, value2: double): double {
        return this.data1 - value1 - value2 as double;
    }

    public callProtected(value: double): double {
        return this.protectedMethod(value) as double;
    }

    public jf(value: double): double {
        return this.data1 + value as double;
    }
}

class CheckWrongSignature {
    public method(a: String): double {
        return 1.1;
    }
}
