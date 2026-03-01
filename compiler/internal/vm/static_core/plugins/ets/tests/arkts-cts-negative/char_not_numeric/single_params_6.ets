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
desc: Type of any argument (except arguments of a rest parameter) must be compatible
  with the type of the corresponding parameter for any function, method, or constructor
  call
name: single_params_6
---*/

class A {
    field: char;
    constructor(a: char) {
        this.field = a;
    }

    bar(callee: char): char {
        return callee;
    }
}

function foo(callee: char): char {
    return callee;
}

function main() {
    let a1: A = new A(0b0 as byte);
    arktest.assertEQ(a1.field, c'\u0000')
    arktest.assertEQ(a1.bar(0b0 as byte), c'\u0000')
    arktest.assertEQ(foo(0b0 as byte), c'\u0000')
    let a2: A = new A(Byte.MIN_VALUE);
    arktest.assertEQ(a2.field, c'\uFF80')
    arktest.assertEQ(a2.bar(Byte.MIN_VALUE), c'\uFF80')
    arktest.assertEQ(foo(Byte.MIN_VALUE), c'\uFF80')
    let a3: A = new A(Byte.MAX_VALUE);
    arktest.assertEQ(a3.field, c'\u007F')
    arktest.assertEQ(a3.bar(Byte.MAX_VALUE), c'\u007F')
    arktest.assertEQ(foo(Byte.MAX_VALUE), c'\u007F')
}
