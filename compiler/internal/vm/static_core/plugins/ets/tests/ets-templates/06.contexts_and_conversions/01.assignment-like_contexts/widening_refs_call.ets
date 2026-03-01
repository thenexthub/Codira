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
desc: Invocation contexts allow a widening reference conversion.
params: Widening reference conversion from '{{c.fromtype}}' to '{{c.totype}}'
---*/

interface I {}
interface J extends I {}
class A implements I {}
class B extends A implements J {}
class C extends B {}

class Clazz {
  public fld1: {{c.totype}};
  public fld2: {{c.totype}}|null = null;

  public constructor(p: {{c.totype}}) {
    this.fld1 = p;
  }

  public foo(p: {{c.totype}}): void {
    this.fld2 = p;
  }
}

function main(): void {
  let v: {{c.fromtype}} = {{c.fromval | safe}};
  let obj: Clazz = new Clazz(v);
  obj.foo(v);
  arktest.assertEQ(obj.fld1, v) 
  arktest.assertEQ(obj.fld2, v)
}
{% endfor %}
