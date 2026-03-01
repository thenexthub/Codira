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

{% for member in class_members %}

/*---
desc: Use variables that not declare in an inner class, and don't have readonly modifier.
assert: >-
    Any local variable, formal parameter, or exception and panic parameter used but not declared in an inner class
    must be declared readonly, or a compile-time error occurs where the use is attempted.
tags: [compile-only, negative]
---*/

class A {
  {{member.decl}};

  public class B {
    public foo(): void {
      {{member.call}};
    }

    public foo(y: int): void {
      let b: int = y;
    }
  }
}

{% endfor %}
