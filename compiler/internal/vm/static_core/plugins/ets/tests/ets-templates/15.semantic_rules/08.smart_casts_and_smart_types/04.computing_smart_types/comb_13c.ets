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
    There could be cases when the type of an entity (mostly local variables)
    is a union type and in the particular context of the program the compiler
    can narrow (smart cast) the static type to a more precise type (smart type)
    and allow operations which are specific to such narrowed type.
---*/

let num: number

function foo(p: number | number[] | undefined): number {
    if (p instanceof Number) {
        num = p * 2 // numeric operation without cast
        return 1
    } else if (p instanceof Array) {
        num = p.length // field access without cast
        return 2
    }
    return 3
}

function main() {
    arktest.assertTrue(foo(42) == 1 && num == 84)
    arktest.assertTrue(foo([1.0, 2.0, 3.0, 4.0, 5.0]) == 2 && num == 5)
    arktest.assertTrue(foo(undefined) == 3)
}
