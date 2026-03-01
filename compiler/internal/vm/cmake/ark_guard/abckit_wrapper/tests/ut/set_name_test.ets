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
function foo1() {
}

namespace Ns1 {
    const field1 = "field1";

    function foo2() {
    }

    namespace Ns2 {
        const field2 = "field2";

        function foo3() {
        }
    }

    enum Shape {
        Circle,
    }

    interface Interface3 {

    }
}

@interface Anno1 {}

const m1 = 1;

interface Interface1 {
}

interface Interface2 {
}

interface Interface3 extends Interface1, Interface2 {
}

interface Interface4 {
    field1: int;
}

enum Color {
    RED
}

class C1 {
    public field1 = 1;

    public M1() {
    };

    public static create(): C1 {
        return new C1();
    }
}

class C3 extends C1 implements Interface1, Interface2 {
    public create(): C3 {
        return new C3();
    }
}

abstract class C2 {}

@Anno1()
class C4 {}


class ClassA {
    public fieldA : ClassB;
    public constructor(field: ClassB) {
        this.fieldA = field;
    }

    public methodA(object: ClassB) {}
}

class ClassB {
    public fieldB1 : ClassA;
    public fieldB2 : ClassA;
    public fieldB3 : ClassA;
    public fieldB4 : ClassA | string;
    public constructor(field1: ClassA, field2: ClassA, field3: ClassA, field4: ClassA | string) {
        this.fieldB1 = field1;
        this.fieldB2 = field2;
        this.fieldB3 = field3;
        this.fieldB4 = field4;
    }
    public methodB(object: ClassA) {}
}

class ClassC {
    public methodC(object: ClassA) :ClassB {
        return new ClassB(object, object, object, object);
    }
}

function foo_multiple_types(object: ClassA | ClassB) :ClassA | ClassB {
    if (object instanceof ClassA) {
        return new ClassB(object, object, object, object);
    } else {
        return new ClassA(object);
    }
}

function foo_multiple_class_array(aArray: ClassA[]): ClassB {
    return new ClassB(aArray[0], aArray[1], aArray[2], aArray[3]);
}

function foo4(i: Interface1){}

interface interface_multiple_type {
    field : ClassA | ClassB;

    methodAAA(){
        console.log(this.field);
    }
}

async function foo5(){}