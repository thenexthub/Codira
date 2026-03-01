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
desc: Inherit multiple methods with override-equivalent signatures, but method in superclass is abstract.
assert: >-
    It is a compile-time error if a class C inherits a default method whose signature is override-equivalent
    with another method inherited by C, unless there exists an abstract method declared in a superclass of C
    and inherited by C that is override-equivalent with the two methods.
    Otherwise, the set of override-equivalent methods consists of at least one abstract method and zero or more
    default methods; then the lass is necessarily an abstract class and is considered to inherit all the methods.
tags: [compile-only]
---*/

abstract class A {
  abstract m(): int;
}

interface I {
  m(): int {
    return 1;
  }
}

class C extends A {
  override m(): int {
    return 5;
  }
}
