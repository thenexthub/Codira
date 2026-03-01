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

import { BaseClass } from "./class_demo1.ets";

abstract class AbstractClass {
    field: number = 123;
    final methodFinal() {
        return 'abstract method0';
    }

    abstract methodAbs(): string;

    public methodSync() {
        console.log('abstract method2');
    }

    public native methodNative(): void;
}

interface Interface1 {
    field_inf: string;
    print();
}

interface Interface2 {
    getDescription(): string;
}

interface Interface3 extends Interface1 {
    field_inf2: number;
}

class ClassA extends AbstractClass implements Interface1 {
    public static field0: int = 0;
    public field1: int = 1;
    protected field2: int = 2;
    private field3: int = 3;
    internal field4: int = 4;
    field_inf: string;

    constructor() {
        super();
        this.field_inf = 'ClsA';
    }

    public static method0() {
        return `A ${ClassA.field0++}`;
    }

    public method1() {
        return 'A ' + this.methodFinal();
    }

    protected method2() {
        this.methodSync();
        return 'A method2';
    }

    private method3() {
        return 'A method3';
    }

    internal method4() {
        return 4;
    }

    methodAbs() {
        return ClassA.method0() + this.method1() + this.method2() + this.method3() + this.method4() +
            (this.field + this.field1 + this.field3 + this.field4);
    }

    print() {
        console.log(this.methodAbs());
    }
}

class ClassB extends ClassA implements Interface2 {
    public method1() {
        return 'B ' + super.method1();
    }

    getDescription(): string {
        return "ClsB " + this.methodAbs() + this.method2();
    }
}

class ClassC extends ClassB implements Interface3 {
    field_inf2: number = 123;

    protected method2() {
        return 'C';
    }

    getDescription(): string {
        return "ClsC " + this.methodAbs() + this.field_inf2;
    }
}
final class FinalClass{
    final methodFinal() {
        let clsA = new ClassA();
        clsA.print();
        let clsB = new ClassB();
        clsB.print();
        console.log(clsB.getDescription());
        let clsC = new ClassC();
        clsC.print();
        console.log(clsC.getDescription());
    }
}

function main() {
    let clsFinal = new FinalClass();
    clsFinal.methodFinal();
}

class ChildClass extends BaseClass {
    public field2 = 1;

    update(x: int) {
        this.field1 = x;
    }
}
