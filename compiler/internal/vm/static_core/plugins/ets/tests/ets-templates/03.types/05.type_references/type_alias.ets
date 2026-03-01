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
desc: Generic named type
---*/

type T1 = Object
type T2 = number

function foo(t1: T1, t2: T2) {
    arktest.assertEQ(typeof t1, 'object')
    arktest.assertEQ(typeof t2, 'number')

    t1 = t2
    t2 = t2 + t2

    arktest.assertEQ(t1, 1)    
    arktest.assertEQ(t2, 2)
}

function main() {
    foo(new Object(), 1)
}
