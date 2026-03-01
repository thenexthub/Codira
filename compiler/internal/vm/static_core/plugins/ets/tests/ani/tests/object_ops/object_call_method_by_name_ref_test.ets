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

    public getName(value1: int, value2: int): string
    {
        if (value1 == 10 && value2 == 20) {
            return "Equality";
        } else {
            return "Inequality";
        }
    }
}

interface ji {
    jf(value: int): string;
}

class B implements ji {
    constructor(data: int) {
        this.data = data;
    }

    data: int;

    public refMethod(value1: int, value2: int): string {
        if (value1 === value2) {
            return "Equality";
        } else {
            return "Inequality";
        }
    }

    public refMethod(value: int): string {
        if (this.data > value) {
            return "Greater";
        } else {
            return "Less";
        }
    }

    protected protectedMethod(value: int): string {
        if (this.data > value) {
            return "Greater";
        } else {
            return "Less";
        }
    }

    private privateMethod(value: int): string {
        if (this.data > value) {
            return "Greater";
        } else {
            return "Less";
        }
    }

    public nestedMethod(value: int): string {
        let lambdaMethod: (p1: int, p2: int) => string =
            (param1: int, param2: int): string => {
                if (this.data > value) {
                    return "Greater";
                } else {
                    return "Less";
                }
        };
        return lambdaMethod(this.data, value);
    }

    public recursiveMethod(value: int): string {
        if (value <= 0) {
            return "Less";
        }
        if (value === 1) {
            return "Greater";
        }
        return this.recursiveMethod(value - 1);
    }

    calculateSum(value1: int, value2: char, value3: double): string {
        let aChar: char = c'A';
        if (value2 == aChar) {
            return "CharEquality";
        }
        if (value3 == 1.0) {
            return "NumEquality";
        }
        return "Inequality";
    }

    public jf(value: int): string {
        if (this.data > value) {
            return "Greater";
        } else {
            return "Less";
        }
    }
}

final class C extends B {
    constructor(data1: int) {
        super(data1);
        this.data1 = data1;
    }

    data1: int;

    public refCompareMethod(value1: int, value2: int): string {
        if (value1 > value2) {
            return "Greater";
        } else {
            return "Less";
        }
    }

    public override refMethod(value1: int, value2: int): string {
        if (value1 == value2) {
            return "Equality";
        } else {
            return "Inequality";
        }
    }

    public callProtected(value: int): string {
        return this.protectedMethod(value) as string;
    }

    public jf(value: int): string {
        if (this.data1 === value) {
            return "Equality";
        } else {
            return "Inequality";
        }
    }
}

class CheckWrongSignature {
    public method(a: String): String {
        return "a";
    }
}
