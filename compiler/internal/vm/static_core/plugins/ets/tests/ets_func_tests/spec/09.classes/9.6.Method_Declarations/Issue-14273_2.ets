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
desc: Issue 14273 test
name: Issue-14273_2.ets
tags: []

---*/

class Base {
    public field: number = 20;
}

class Extender extends Base {
    public static field: number = 10;
}


function main(): void {
    let a = new Extender();

    arktest.assertEQ( (a as Base).field, 20)  // Access to static members via instance is prohibited (see 9.5.1 and 7.10.3)
    arktest.assertEQ( Extender.field, 10)

}
