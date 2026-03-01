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

    public booleanMethod(a0: int, a1: int): boolean {
        if (a0 + a1 > 10) {
            return true;
        } else {
            return false;
        }
    }

    public booleanMethodVoidParam(): boolean
    {
        return true;
    }

    public booleanMethodMultipleParam(a0: byte, a1: float, a2:float, a3:byte): boolean
    {
        if (a0 > a3) {
            return true;
        } else if (a1 > a2) {
            return false;
        } else {
            return false;
        }
    }

    public nestedMethod(data1: byte, data2: byte): boolean
    {
        let method = (value1: byte, value2: byte): boolean => {
            if (value1 > value2) {
                return true;
            } else {
                return false;
            }
        }

        return method(data1, data2);
    }

    public recursionMethod(a0: int): boolean
    {
        if (a0 > 100) {
            return true;
        } else {
            return this.recursionMethod(a0 + 10);
        }
    }
}

final class B extends A {

    public subBooleanMethodVoidParam(): boolean
    {
        return false;
    }
}

abstract class C {
    public abstract func(a: int, b: int): boolean;
}

class D extends C {
    public override func(a: int, b: int): boolean {
        if (a > b) {
            return true;
        } else {
            return false;
        }
    }
}

class E extends D {
    public override func(a: int, b: int): boolean {
        if (a + b > 10) {
            return true;
        } else {
            return false;
        }
    }
}

let parentMethodInParentWasCalled: int = 1;
let childMethodInChildWasCalled: int = 2;
let overridedMethodInChildWasCalled: int = 3;
let overridedMethodInParentWasCalled: int = 4;

class Parent {
    methodChecker: int = 0;
    getCheckerValue(): int {
        return this.methodChecker;
    }

    parentMethod(): boolean {
        this.methodChecker = parentMethodInParentWasCalled;
        return true;
    }
    overridedMethod(): boolean {
        this.methodChecker = overridedMethodInParentWasCalled;
        return true;
    }
}
class Child extends Parent {
    childMethod(): boolean {
        this.methodChecker = childMethodInChildWasCalled;
        return true;
    }
    overridedMethod(): boolean {
        this.methodChecker = overridedMethodInChildWasCalled;
        return true;
    }
}