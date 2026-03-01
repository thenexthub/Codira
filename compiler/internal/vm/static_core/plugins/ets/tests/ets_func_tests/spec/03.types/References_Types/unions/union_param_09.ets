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
name: 03.types/08.reference_types/03.08.11.union_types/union_param_06
tags: []
---*/

type Predicate = () => boolean
type Margin = Predicate | string

function callString(s: string): int {
    return s instanceof string ? 0 : -1
}

function callLambda(p: Predicate): int {
    return p() ? 0 : -2
}

function callMargin(a: Margin): int {
    if (a instanceof string){
        return callString(a as string)
    } else  {
        return callLambda(a as Predicate)
    }
}

function main(): int {
    let result: int = callMargin("hello")
    if (result != 0) { return result }

    let a: Margin = "hello"
    result = callMargin(a)
    if (result != 0) { return result }

    let p: Predicate = (): boolean => true
    result = callMargin(p)
    if (result != 0) { return result }

    let b: Margin = p
    result = callMargin(a)

    return result
}
