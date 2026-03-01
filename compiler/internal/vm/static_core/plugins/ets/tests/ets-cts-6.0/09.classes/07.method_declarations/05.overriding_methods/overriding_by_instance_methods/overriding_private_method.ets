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
desc: Overriding method of superclass.
assert: >-
    One of the following is true. mA is public. mA is protected. mA is declared with package access in the
    same package as C, and either C declares mC or mA is a member of the direct superclass of C.
    mA is declared with package access and mC overrides mA from some superclass of C.
tags: [compile-only, negative]
---*/

class A {
  private m(): int {
    return 1;
  }
}

final class C extends A {
  override m(): int {
    return 5;
  }
}
