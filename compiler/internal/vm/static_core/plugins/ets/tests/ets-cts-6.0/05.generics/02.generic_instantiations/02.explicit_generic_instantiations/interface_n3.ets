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
    An explicit generic instantiation is a language construct, which provides a list of type arguments
    that specify real types or type parameters to substitute corresponding type parameters of a generic.
name: 05.generics/02.generic_instantiations/02.explicit_generic_instantiations/interface_n2
tags:
- compile-only
- negative
---*/

interface A<T>{
    a: T
}

interface B<T, U> extends A<T>{
    b: U
}

function main() {
    let i: I<number, string> = {a: 1, b: "abc"}
    arktest.assertTrue(i.a instanceof number)
    arktest.assertTrue(i.b instanceof string)
}
