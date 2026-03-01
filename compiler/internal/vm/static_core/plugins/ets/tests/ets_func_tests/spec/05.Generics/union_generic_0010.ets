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

/*---
desc: 05 Generics
tags: []
---*/

class A1<T> {
    private attr: String;
    private attr2: T;

    constructor(p_attr: String, p_attr2: T ) {
        this.attr = p_attr;
        this.attr2 = p_attr2;
    }

    public foo() : String {
        return (this.attr + this.attr2);
    }

}

class A2 {
    private attr: String;
    private attr2: number;

    constructor(p_attr : String, p_attr2: number) {
        this.attr = p_attr;
        this.attr2 = p_attr2;
    }

    public bar() : String {
        return (this.attr + this.attr2)
    }
}

type A12 = A1<Number> | A2;

function main(): int {
    let var1 : A12;
    let var2 : A12;

    var1 = new A1<Number>("Hello", 496)
    var2 = new A2("Bye", 28);

    arktest.assertEQ( var1.foo(), "Hello496")
    arktest.assertEQ( var2.bar(), "Bye28")
    return 0;
}
