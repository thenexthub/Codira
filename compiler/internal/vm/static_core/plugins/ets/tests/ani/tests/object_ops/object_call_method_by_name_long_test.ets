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

	public longMethod(value1: long, value2: long): long
    {
        return value1 + value2;
    }
}

interface ji {
    jf(value: long): long;
}

class B implements ji {
    constructor(data: long) {
        this.data = data;
    }

    data: long;

    public longMethod(value1: long, value2: long): long {
        if (this.data > value1 + value2) {
            return this.data;
        } else {
            return value1 + value2 as long;
        }
    }

    public longMethod(value: long): long {
        if (this.data > value) {
            return this.data;
        } else {
            return value;
        }
    }

    protected protectedMethod(value: long): long {
        return this.data + value as long;
    }

    private privateMethod(value: long): long {
        return this.data - value as long;
    }

    public nestedMethod(value: long): long {
        let lambdaMethod: (p1: long, p2: long) => long =
            (param1: long, param2: long): long => {
                return param1 + param2 as long;
        };
        return lambdaMethod(this.data, value);
    }

    public recursiveMethod(value: int): long {
        if (value <= 0) {
            return 1;
        } else {
            return value * this.recursiveMethod(value - 1) as long;
        }
    }

    calculateSum(value1: long, value2: char, value3: int): long {
        let aChar: char = c'A';
        if (value2 == aChar) {
            return this.data - value1 as long;
        }
        if (value3 == 1) {
            return value1;
        }
        return this.data + value1 as long;
    }

    public jf(value: long): long {
        return this.data - value as long;
    }
}

final class C extends B {
    constructor(data1: long) {
        super(data1);
        this.data1 = data1;
    }

    data1: long;

    public longAddMethod(value1: long, value2: long): long {
        return this.data1 + value1 + value2 as long;
    }

    public override longMethod(value1: long, value2: long): long {
        return this.data1 - value1 - value2 as long;
    }

    public callProtected(value: long): long {
        return this.protectedMethod(value) as long;
    }

    public jf(value: long): long {
        return this.data1 + value as long;
    }
}

class CheckWrongSignature {
    public method(a: String): long {
        return 1;
    }
}
