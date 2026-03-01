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
desc: >-
    03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_param_11_02
tags: []
---*/

class A {}
class B extends A {}

type Margin = B | string

function callB(b: B, text: string) {
    arktest.assertTrue( b instanceof B, text)
}

function callA(a: Margin, name?: string) {
    if (a instanceof B){
        callB(a as B, name ?? "")
    }
}

function main(): void {
    callA(new B())
    let a: Margin = new B()
    callA(a, "b is not instance of B")
}
