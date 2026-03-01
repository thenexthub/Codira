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
desc: A value of the null type (the null reference is the only such value) may be assigned to any reference type.
---*/

type Intfoo = (i: int) => int;

class A {
  private val: int;

  public constructor(
    {%- for t in c['types'] %}
    p{{loop.index}}: {{t|safe}}{{ ", " if not loop.last }}
    {%- endfor %}
  ) {
    if (
      {%- for t in c['types'] %}
      p{{loop.index}} == null {{ ("&&" if not loop.last) | safe}}
      {%- endfor %}
    ) {
      this.val = 42;
    }
  }

  public foo(
    {%- for t in c['types'] %}
    p{{loop.index}}: {{t|safe}}{{ ", " if not loop.last }}
    {%- endfor %}
  ): void {
    {%- for t in c['types'] %}
    arktest.assertEQ(p{{loop.index}}, null)
    {%- endfor %}
    
  }
}

function main(): void {
  let c: A = new A({% for t in c['types'] %}null{{ ", " if not loop.last }}{% endfor %});
  c.foo({% for t in c['types'] %}null{{ ", " if not loop.last }}{% endfor %});
}
{% endfor %}
