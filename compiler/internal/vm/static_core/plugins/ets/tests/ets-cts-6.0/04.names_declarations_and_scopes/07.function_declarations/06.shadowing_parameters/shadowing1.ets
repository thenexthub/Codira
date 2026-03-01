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
    If the name of the parameter is identical to the name of the top-level variable
    accessible within the body of a function or a method with that a parameter,
    then within the body of such a function or a method the name of that parameter
    shadows the name of the top-level variable.
---*/

let q: int = 1;

function bar(q: int): int { // q shadows top-level q
    return q + q;
}

function main() {
    q = 1;
    arktest.assertEQ(bar(10), 20);
    arktest.assertEQ(q, 1);
}
