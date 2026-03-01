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
name: 05.generics/genericTypeArgumentWithDefaultParameterValue_06
tags: []
---*/

class A<T1=Int, T2=string> {
    data1: T1
    data2: T2
    constructor(p1 : T1, p2: T2 ) {
        this.data1 = p1;
        this.data2 = p2;
    }
}

function main(): void {
    let a = new A(1, "world");
    let a1 = new A<Int, string>(1, "world")
    arktest.assertTrue( a.data1 instanceof Int)
    arktest.assertTrue( a.data2 instanceof string)
    arktest.assertTrue( a1.data1 instanceof Int)
    arktest.assertTrue( a1.data2 instanceof string)
}
