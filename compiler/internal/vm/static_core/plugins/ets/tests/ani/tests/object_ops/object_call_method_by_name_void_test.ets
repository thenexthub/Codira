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

	public voidMethod(value1: long, value2: long): void
    {
        this.value = value1 + value2;
    }

    public getValue(): long
    {
        return this.value;
    }

	private value: long;
}

interface ji {
    jf(value: int): void;
}

class B implements ji {
    constructor(data: int) {
        this.data = data;
    }

    data: int;

    public voidMethod(value1: int, value2: int): void {
        this.intValue = value1 + value2;
    }

    public voidMethod(value: char): void {
        this.charValue = value;
    }

    protected protectedMethod(value: int): void {
        this.intValue = value;
    }

    protected protectedMethod1(value1: int, value2: int): int {
        return value1 + value2 as int;
    }

    private privateMethod(value: double): void {
        this.doubleValue = value;
    }

    public nestedMethod(value: int): void {
        let lambdaMethod: (p1: int, p2: int) => int =
            (param1: int, param2: int): int => {
                return param1 + param2 as int;
        };
        this.intValue = lambdaMethod(this.data, value);
    }

    public recursiveMethod(value: int): void {
        if (value <= 0) {
            return ;
        }
        this.intValue += value;
        this.recursiveMethod(value - 1);
    }

    calculateSum(value1: int, value2: char, value3: double): void {
        this.intValue = value1;
        this.charValue = value2;
        this.doubleValue = value3;
    }

    public jf(value: int): void {
        this.intValue = value;
    }


    public getIntValue(): int
    {
        return this.intValue;
    }

    public getDoubleValue(): double
    {
        return this.doubleValue;
    }

    public getCharValue(): char
    {
        return this.charValue;
    }

	private intValue: int = 0;
	private doubleValue: double = 0.0;
	private charValue: char;
}

final class C extends B {
    constructor(data1: int) {
        super(data1);
        this.data1 = data1;
    }

    data1: int;

    public voidSunMethod(value1: int, value2: int): void {
        this.intValue = value1 + value2;
    }

    public override voidMethod(value1: int, value2: int): void {
        this.intValue = this.data1 + value1 + value2;
    }

    public callProtected(value1: int, value2: int): void {
        this.intValue = this.protectedMethod1(value1, value2);
    }

    public jf(value: int): void {
        this.intValue = this.data1 + value;
    }

    public getIntValue(): int
    {
        return this.intValue;
    }

	private intValue: int;
}

class CheckWrongSignature {
    public method(a: String): void {
    }
}
