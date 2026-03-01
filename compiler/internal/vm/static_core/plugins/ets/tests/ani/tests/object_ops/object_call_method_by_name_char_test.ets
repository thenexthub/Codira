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

	public charByNameMethod(value1: char, value2: char): char {
        return value1.compareTo(value2) > 0 ? value1 : value2;
    }
}

interface ji {
    jf(value: char): char;
}

class B implements ji {
    constructor(data: char) {
        this.data = data;
    }

    data: char;

    public charMethod(value1: char, value2: char): char {
        return value1.compareTo(value2) > 0 ? value1 : value2;
    }

    public charMethod(value: char): char {
        if (this.data.compareTo(value) > 0) {
            return this.data;
        } else {
            return value;
        }
    }

    protected protectedMethod(value: char): char {
        if (this.data.compareTo(value) > 0) {
            return this.data;
        } else {
            return value;
        }
    }

    private privateMethod(value1: char, value2: char): char {
        return value1.compareTo(value2) > 0 ? value1 : value2;
    }

    public nestedMethod(value: char): char {
        let lambdaMethod: (p1: char, p2: char) => char =
            (param1: char, param2: char): char => {
                return param1.compareTo(param2) > 0 ? param1 : param2;
        };
        return lambdaMethod(this.data, value);
    }

    public recursiveMethod(value: int): char {
        if (value <= 0) {
            return c'A';
        }
        if (value === 1) {
            return c'Z';
        }
        return this.recursiveMethod(value - 1);
    }

    calculateSum(value1: char, value2: char, value3: double, value4: int): char {
        if (value3 === 1.0) {
            return this.data;
        }
        if (value4 === 1) {
            return value1;
        }
        return value2;
    }

    public jf(value: char): char {
        return this.data.compareTo(value) > 0 ? this.data : value;
    }
}

final class C extends B {
    constructor(data1: char) {
        super(data1);
        this.data1 = data1;
    }

    data1: char;

    public charComparisonMethod(value1: char, value2: char): char {
        return value1.compareTo(value2) > 0 ? value1 : value2;
    }

    public override charMethod(value1: char, value2: char): char {
        return value1.compareTo(value2) > 0 ? value2 : value1;
    }

    public callProtected(value: char): char {
        return this.protectedMethod(value) as char;
    }

    public jf(value: char): char {
        return this.data.compareTo(value) > 0 ? value : this.data;
    }
}

class CheckWrongSignature {
    public method(a: String): char {
        return c'a';
    }
}
