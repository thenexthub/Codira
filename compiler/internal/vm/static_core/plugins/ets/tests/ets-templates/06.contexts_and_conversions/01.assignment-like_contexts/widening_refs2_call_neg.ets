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

{% for case in cases %}
/*---
desc: Invocation contexts allow the use a widening reference conversion.
tags: [compile-only, negative]
params: {{case.val | safe}} to {{case.type}}
---*/

class B <T extends {{case.type}}> {
  {%- for v in case['ptype'] %}
  fld1_{{v}}: {{v}};
  fld2_{{v}}: {{v}};
  {%- endfor %}

  constructor({% for v in case['ptype'] %}p{{loop.index}}: T{{ ", " if not loop.last }}{% endfor %}) {
    {%- for v in case['ptype'] %}
    this.fld1_{{v}} = p{{loop.index}}
    {%- endfor %}
  }

  public foo({% for v in case['ptype'] %}p{{loop.index}}: T{{ ", " if not loop.last }}{% endfor %}): void {
    {%- for v in case['ptype'] %}
    this.fld2_{{v}} = p{{loop.index}}
    {%- endfor %}
    {%- for v in case['ptype'] %}
    arktest.assertEQ(this.fld1_{{v}}, this.fld2_{{v}})
    {%- endfor %}
  }
}

function main(): void {
  let b: B<{{case.type}}> = new B <{{case.type}}> ({% for v in case['ptype'] %}{{case.val | safe}}{{ ", " if not loop.last }}{% endfor %});
  b.foo({% for v in case['ptype'] %}{{case.val | safe}}{{ ", " if not loop.last }}{% endfor %});
}
{% endfor %}
