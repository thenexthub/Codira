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
desc: Invocation contexts allow a widening primitive conversion.
params: Widening primitive conversion to '{{c.ctype}}'
---*/

class A {
  public fld1: {{c.ctype}};
  public fld2: {{c.ctype}};

  public constructor(p: {{c.ctype}}) {
    this.fld1 = p;
  }

  public foo(p: {{c.ctype}}): void {
    this.fld2 = p;
  }
}

function main(): void {
  {% for t in c['values'] %}
  let v{{loop.index}}: {{t.type}} = {{t.val|safe}};
  let obj{{loop.index}}: A = new A({% if t.obj_param_trans is defined %}{{t.obj_param_trans|safe}}(v{{loop.index}}){% else %}v{{loop.index}}{% endif %});
  obj{{loop.index}}.foo({% if t.obj_param_trans is defined %}{{t.obj_param_trans|safe}}(v{{loop.index}}){% else %}v{{loop.index}}{% endif %});
  arktest.assertEQ(obj{{loop.index}}.fld1, {{t.val2|safe}})
  arktest.assertEQ(obj{{loop.index}}.fld2, {{t.val2|safe}})
  {% endfor %}
}
{% endfor %}
