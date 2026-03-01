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
    If one operand of the binary operator ‘+’ is of type string,
    then the string conversion applies to another non-string operand to convert it to string
---*/

function main() {
    let b: byte = 1
    let s: short = 2
    let i: int = 3
    let l: long = 4
    let s1 = "ok"
    let s2 = `lorem
    ipsum`
    arktest.assertTrue((b + s1) instanceof string)
    arktest.assertTrue((s + i + s2) instanceof string)
    arktest.assertTrue((i + "yes" + l) instanceof string)
    arktest.assertTrue((b * i + s1 + l) instanceof string)
}
