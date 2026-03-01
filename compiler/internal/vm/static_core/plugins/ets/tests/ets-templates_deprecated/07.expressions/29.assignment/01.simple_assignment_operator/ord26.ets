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
    If the left-hand operand is an array access expression (§7.12), possibly enclosed in one or more pairs of parentheses, then:
    - First, the array reference subexpression of the left-hand operand is evaluated.
    If this evaluation completes abruptly, then the assignment expression completes
    abruptly for the same reason; the index subexpression and the right-hand operand
    are not evaluated and no assignment occurs.
    - Otherwise, the index subexpression of the left-hand operand is evaluated.
    If this evaluation completes abruptly, then the assignment expression completes
    abruptly for the same reason; the right-hand operand is not evaluated and no assignment occurs.
    - Otherwise, the right-hand operand is evaluated. If this evaluation completes abruptly,
    then the assignment expression completes abruptly for the same reason and no assignment occurs.
    - Otherwise, if the value of the index subexpression is less than zero, or greater than or
    equal to the length of the array, then no assignment occurs and an ArrayIndexOutOfBoundsError
    is thrown.
params: ArrayStoreError
---*/

let result: string = ""
let x: Object[] = new Int[3]

function ref(p: string): Object[] {
    result = result + p
    return x;
}

function idx(i: int): int {
    result = result + i
    return i
}

function val(): Object {
    result = result + "V"
    return "ABC"  // get ArrayStoreError
}

function main(): int {
    result = "";  // reset due to jit mode warmup
    
    let s: string = ""
    try {
      s = s + ref("A")[idx(2)] = ref("B")[idx(1)] = ref("C")[idx(0)] = val()
    } catch (e: ArrayStoreError) {
        if (result == "A2B1C0V" && s == "") {
            return 0
        }
    }
    return 1
}
