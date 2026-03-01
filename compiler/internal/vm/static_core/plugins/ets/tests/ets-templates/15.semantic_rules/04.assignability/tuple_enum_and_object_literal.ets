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
desc: Type T1 is compatible with type T2 if there is an implicit conversion (see Implicit Conversions)
      that allows converting type T1 to type T2.
tags: []
---*/

 class C {
     p1: string = "";
     p2: int;
 }
 function foo(i: C) { return i.p2; }

function main() {
     let c: C = { p1: "Xzc", p2: 2 };
     arktest.assertEQ(foo(c), 2);
}


