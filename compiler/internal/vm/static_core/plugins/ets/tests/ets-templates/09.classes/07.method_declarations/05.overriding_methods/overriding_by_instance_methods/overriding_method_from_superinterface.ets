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
desc: Overriding method of superinterface.
assert: >-
    An instance method mC declared in or inherited by class C, overrides from C another method mI declared in
    interface I, iff all of the following are true. I is a superinterface of C. mI is not static.
    C does not inherit mI. The signature of mC is a subsignature (link to signatures) of the signature of mI. mI is public.
---*/

interface I {
  m(): int;
}

class C implements I {
  override m(): int {
    return 5;
  }
}

function main(): void {
  let instance: C = new C();

  arktest.assertTrue( instance.m() == 5 );
}
