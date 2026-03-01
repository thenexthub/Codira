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
      If superinterfaces have more then one default implementations (see Default Interface Method Declarations)
      for some method m, then
      - The class that implements these interfaces has method that overrides m (see Override-Compatible Signatures)
      Otherwise, a compile-time error occurs.
tags: [compile-only, negative]
---*/

class C1 {}
class C2 extends C1 {}

interface I1 {
  foo(i: C1) {}
}
interface I2 {
  foo(i: C2) {}
}

class C implements I1, I2 {
  foo(i: C2) {}
}
