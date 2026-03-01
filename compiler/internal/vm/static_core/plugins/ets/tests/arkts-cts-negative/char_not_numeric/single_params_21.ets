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
name: single_params_21
---*/

class A {
    field: long;
    constructor(a: long) {
        this.field = a;
    }

    bar(callee: long): long {
        return callee;
    }
}

function foo(callee: long): long {
    return callee;
}

function main() {
    let a1: A = new A(c'\u0000');
    arktest.assertEQ(a1.field, 0 as long)
    arktest.assertEQ(a1.bar(c'\u0000'), 0 as long)
    arktest.assertEQ(foo(c'\u0000'), 0 as long)
    let a2: A = new A(c'\u0FFF');
    arktest.assertEQ(a2.field, 4095 as long)
    arktest.assertEQ(a2.bar(c'\u0FFF'), 4095 as long)
    arktest.assertEQ(foo(c'\u0FFF'), 4095 as long)
    let a3: A = new A(c'\uFFFF');
    arktest.assertEQ(a3.field, 65535 as long)
    arktest.assertEQ(a3.bar(c'\uFFFF'), 65535 as long)
    arktest.assertEQ(foo(c'\uFFFF'), 65535 as long)
}
