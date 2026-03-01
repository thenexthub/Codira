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

{% for mod in modifiers %}

/*---
desc: Hiding method with different access method.
assert: >-
    The access modifier of an overriding or hiding method must provide at least as much access as the overridden
    or hidden method, as follows.  If the overridden or hidden method is public,
    then the overriding or hiding method must be public; otherwise, a compile-time error occurs.
    If the overridden or hidden method is protected, then the overriding or hiding method must be protected or public;
    otherwise, a compile-time error occurs. If the overridden or hidden method has package access,
    then the overriding or hiding method must be internal; otherwise, a compile-time error occurs.
params: {{mod.m1}} and {{mod.m2}}
tags: {{mod.tags}}
---*/

class A {
  {{mod.m1}} m(): int {
    return 1;
  }
}

final class C extends A {
  {{mod.m2}} m(): int {
    return 5;
  }
  public test(): int {
    return this.m()
  }
}

function main(): void {
  let c = new C()
  arktest.assertEQ(c.test(), 5)
}

{% endfor %}
