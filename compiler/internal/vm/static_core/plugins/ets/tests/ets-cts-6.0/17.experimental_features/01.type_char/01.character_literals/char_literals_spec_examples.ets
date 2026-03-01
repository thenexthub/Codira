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
desc: Char literals. Spec examples of char literals.
---*/

function main() {
    let c1 = c'a'
    let c2 = c'\n'
    let c3 = c'\x7F'
    let c4 = c'\u0000'
    let c5 = c'\uffff'

    let c1b = new Char(c1)
    let c2b = new Char(c2)
    let c3b = new Char(c3)
    let c4b = new Char(c4)
    let c5b = new Char(c5)

    arktest.assertEQ(c1b, c1)
    arktest.assertEQ(c2b, c2)
    arktest.assertEQ(c3b, c3)
    arktest.assertEQ(c4b, c4)
    arktest.assertEQ(c5b, c5)
}
