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
    If the parameter type is readonly array or tuple type, then no assignment and no function or method call can modify
    elements of this array or tuple. Otherwise, a compile-time error occurs:
---*/

function update(values: readonly Array<int[]>, posX: int, posY: int, input: int) {

    values[posX][posY] = input
    return values[posX][posY]
}

function main() {
    let rw: Array<int[]> = [[1, 2], [3, 4], [5, 6]]
    arktest.assertEQ(12, update(rw, 0, 1, 12));
}
