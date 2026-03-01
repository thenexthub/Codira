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

{% for c in cases %}

/*---
desc: Different return type in mA and mC.
assert: >-
    If a method declaration d1 with return type R1 overrides or hides the declaration of another method d2
    with return type R2, then d1 must be return-type-substitutable for d2, or a compile-time error occurs.
tags: []
---*/

class A {
  public m(): B {
    return new B;
  }
}

class B {

}

class D extends B {

}

class C extends A {
  public override m(): D {
    return new D();
  }
}

function getC(): C {
    return new C
}

function getA(): A {
    return new A
}

function main() {
    {{c|indent}}
}

{% endfor %}
