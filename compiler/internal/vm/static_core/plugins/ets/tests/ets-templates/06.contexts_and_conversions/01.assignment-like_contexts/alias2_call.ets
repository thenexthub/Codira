/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

{% for c in cases %}
/*---
desc: Invocation contexts followed by a widening primitive conversion.
params: from '{{c.ref_type}}' to '{{c.type}}'
---*/

class A {
  {%- for v in c['values'] %}
  public fld_{{loop.index}}: {{c.type}};
  {%- endfor %}

  public constructor({% for v in c['values'] %}p{{loop.index}}: {{c.type}}{{ ", " if not loop.last }}{% endfor %}) {
    {%- for v in c['values'] %}
    this.fld_{{loop.index}} = p{{loop.index}};
    {%- endfor %}
  }

  public foo({% for v in c['values'] %}p{{loop.index}}: {{c.type}}{{ ", " if not loop.last }}{% endfor %}): void {
    {%- for v in c['values'] %}
    arktest.assertEQ(this.fld_{{loop.index}}, p{{loop.index}})
    {%- endfor %}
  }
}

function main(): void {
  {%- for v in c['values'] %}
  let v{{loop.index}}: {{c.ref_type}} = {{v|safe}};
  {%- endfor %}

  let c: A = new A({% for v in c['values'] %}v{{loop.index}}{{ ", " if not loop.last }}{% endfor %});
  c.foo({% for v in c['values'] %}v{{loop.index}}{{ ", " if not loop.last }}{% endfor %});
}
{% endfor %}
