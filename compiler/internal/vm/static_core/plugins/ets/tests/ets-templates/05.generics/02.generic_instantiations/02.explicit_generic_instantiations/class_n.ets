/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: >-
    An explicit generic instantiation is a language construct, which provides a list of type arguments
    that specify real types or type parameters to substitute corresponding type parameters of a generic.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0228: Type argument 'Object' should be a subtype of 'A' constraint"
---*/

class A {}
class B extends A {}

class Cls<T extends U, U extends Object> {
    a: T
    b: U
    constructor(a: T, b: U) {
        this.a = a
        this.b = b
    }
}

function main() {
    let cls1: Cls<Object, A> = new Cls<Object, A>(new Object(), new Object())
}
