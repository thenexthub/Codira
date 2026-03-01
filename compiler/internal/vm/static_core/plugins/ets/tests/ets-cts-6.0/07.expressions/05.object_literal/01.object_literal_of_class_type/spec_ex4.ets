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
    If some class fields have default values (see Default Values for Types)
    or explicit initializers (see Variable and Constant Declarations),
    then such fields can be skipped in the object literal.
---*/

class Friend {
    name: string = ""
    private nick: string = ""
    age: number
    sex?: "male"|"female"
}

function main(): void {
    // name, nick, age, and sex have either default value or explicit initializer
    let f: Friend = {}
    arktest.assertTrue(f instanceof Friend)
    arktest.assertEQ(f.name, "")
    arktest.assertEQ(f.age, 0)
    arktest.assertEQ(f.sex, undefined)
}
