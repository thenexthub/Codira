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
    03.08.11 Union types - literal types
name: 03.types/08.reference_types/03.08.11.union_types/union_literal_02
tags: []
---*/

type T = "hello" | "goodbye"
function say(greet: T, name: string): string {
    return greet + " " + name
}

function tToString01(greet: T): string {
    // implicit cast
    return greet
}

function tToString02(greet: T): string {
    // explicit cast
    return greet as string
}

function tToString03(greet: T): string {
    let name: string = greet
    return name + "+"
}

function main(): void {
    let t1: T = "hello"
    let t2: T = "goodbye"
    t1 = t2
    arktest.assertEQ( t1, "goodbye")
    arktest.assertEQ( say(t1, "blue sky"), "goodbye blue sky")
    arktest.assertEQ( tToString01(t1), "goodbye")
    arktest.assertEQ( tToString02(t1), "goodbye")
    arktest.assertEQ( tToString03(t1), "goodbye+")
}
