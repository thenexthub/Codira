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
desc: A compound assignment expression of the form E1 op= E2 is equivalent to E1 =
  ((E1) op (E2)) as T, where T is the type of E1, except that E1 is evaluated only
  once.
name: ops_boxed_25
params: Char, "+="
---*/

function main(): int {
    let v:Char = c'\uFFFF'
    let q:Char = c'\uFFFF'
    if ((v + q).toChar() != (v += q)) {
        return 1
    }
    return 0
}
