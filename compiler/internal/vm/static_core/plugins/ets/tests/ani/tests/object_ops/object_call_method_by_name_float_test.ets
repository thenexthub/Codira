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

	public floatMethod(value1: float, value2: float): float
    {
        return value1 + value2;
    }
}

interface ji {
    jf(value: float): float;
}

class B implements ji {
    constructor(data: float) {
        this.data = data;
    }

    data: float;

    public floatMethod(value1: float, value2: float): float {
        if (this.data > value1 + value2) {
            return this.data;
        } else {
            return value1 + value2 as float;
        }
    }

    public floatMethod(value: float): float {
        if (this.data > value) {
            return this.data;
        } else {
            return value;
        }
    }

    protected protectedMethod(value: float): float {
        return this.data + value as float;
    }

    private privateMethod(value: float): float {
        return this.data - value as float;
    }

    public nestedMethod(value: float): float {
        let lambdaMethod: (p1: float, p2: float) => float =
            (param1: float, param2: float): float => {
                return param1 + param2 as float;
        };
        return lambdaMethod(this.data, value);
    }

    public recursiveMethod(value: int): float {
        if (value <= 0) {
            return 1;
        } else {
            return value * this.recursiveMethod(value - 1) as float;
        }
    }

    calculateSum(value1: float, value2: char, value3: int): float {
        let aChar: char = c'A';
        if (value2 == aChar) {
            return this.data - value1 as float;
        }
        if (value3 == 1) {
            return value1;
        }
        return this.data + value1 as float;
    }

    public jf(value: float): float {
        return this.data - value as float;
    }
}

final class C extends B {
    constructor(data1: float) {
        super(data1);
        this.data1 = data1;
    }

    data1: float;

    public floatAddMethod(value1: float, value2: float): float {
        return this.data1 + value1 + value2 as float;
    }

    public override floatMethod(value1: float, value2: float): float {
        return this.data1 - value1 - value2 as float;
    }

    public callProtected(value: float): float {
        return this.protectedMethod(value) as float;
    }

    public jf(value: float): float {
        return this.data1 + value as float;
    }
}

class CheckWrongSignature {
    public method(a: String): float {
        return Double.toFloat(1.1);
    }
}
