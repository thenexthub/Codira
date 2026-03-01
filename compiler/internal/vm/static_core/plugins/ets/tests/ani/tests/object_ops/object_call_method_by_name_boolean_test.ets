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

    booleanByNameMethod(a0: int, a1: int): boolean {
        if (a0 + a1 > 10) {
            return true;
        } else {
            return false;
        }
    }
}

interface ji {
    jf(value: int): boolean;
}

class B implements ji {
    constructor(data: int) {
        this.data = data;
    }

    data: int;

    public booleanMethod(value1: int, value2: int): boolean {
        if (value1 === value2) {
            return true;
        } else {
            return false;
        }
    }

    public booleanMethod(value: int): boolean {
        if (this.data > value) {
            return true;
        } else {
            return false;
        }
    }

    protected protectedMethod(value: int): boolean {
        if (this.data == value) {
            return true;
        } else {
            return false;
        }
    }

    private privateMethod(value: int): boolean {
        if (this.data > value) {
            return true;
        } else {
            return false;
        }
    }

    public nestedMethod(value: int): boolean {
        let lambdaMethod: (p1: int, p2: int) => boolean =
            (param1: int, param2: int): boolean => {
                if (this.data > value) {
                    return true;
                } else {
                    return false;
                }
        };
        return lambdaMethod(this.data, value);
    }

    public recursiveMethod(value: int): boolean {
        if (value <= 0) {
            return false;
        }
        if (value === 1) {
            return true;
        }
        return this.recursiveMethod(value - 1);
    }

    calculateSum(value1: int, value2: char, value3: double): boolean {
        let aChar: char = c'A';
        if (value2 == aChar) {
            return true;
        }
        if (value3 == 1.0) {
            return false;
        }
        return value1 > 1 ? true : false;
    }

    public jf(value: int): boolean {
        if (this.data > value) {
            return true;
        } else {
            return false;
        }
    }
}

final class C extends B {
    constructor(data1: int) {
        super(data1);
        this.data1 = data1;
    }

    data1: int;

    public booleanCompareMethod(value1: int, value2: int): boolean {
        if (value1 > value2) {
            return true;
        } else {
            return false;
        }
    }

    public override booleanMethod(value1: int, value2: int): boolean {
        if (value1 == value2) {
            return true;
        } else {
            return false;
        }
    }

    public callProtected(value: int): boolean {
        return this.protectedMethod(value) as boolean;
    }

    public jf(value: int): boolean {
        if (this.data1 === value) {
            return true;
        } else {
            return false;
        }
    }
}

class CheckWrongSignature {
    public method(a: String): boolean {
        return true;
    }
}
