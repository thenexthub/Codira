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

    private count_: int;

    public voidMethod(a0: int, a1: int): void
    {
        this.count_ = a0  + a1;
    }

    public getCount(): int
    {
        return this.count_;
    }

    public voidMethodVoidParam(): void
    {
        this.count_ = 10;
    }

    public voidMethodMultipleParam(a0: int, a1: boolean, a2:float, a3:int, a4:float): void
    {
        if (a1) {
            this.count_ = a0 + a3;
        } else if (a4 > a2) {
            this.count_ = a0 - a3;
        } else {
            this.count_ = a0;
        }
    }

    public nestedMethod(data1: int, data2: int): void
    {
        let method = (value1: int, value2: int): void => {
            this.count_ = value1 + value2;
        }

        return method(data1, data2);
    }

    public recursionMethod(a0: int): void
    {
        if (a0 > 100) {
            this.count_ = a0;
        } else {
            return this.recursionMethod(a0 + 10);
        }
    }
}

final class B extends A {

    private value_: int;
    public subVoidMethodVoidParam(): void
    {
        this.value_ = 20;
    }
}

abstract class C {
    public abstract func(a: int, b: int): void;
}

class D extends C {
    private count_: int;
    public override func(a: int, b: int): void {
        this.count_ = (a + b) as int;
    }

    public getCount(): int
    {
        return this.count_;
    }

}

class E extends D {
    private count_: int;
    public override func(a: int, b: int): void {
        this.count_ = (a - b) as int;
    }

    public getCount(): int
    {
        return this.count_;
    }

}