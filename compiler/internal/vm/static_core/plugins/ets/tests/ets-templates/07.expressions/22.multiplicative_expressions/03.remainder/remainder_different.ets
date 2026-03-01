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
desc: Remainder of numbers
params: {{c.x}} % {{c.y}} == {{c.result}}
---*/

function main(): void {
  let x: {{c.typex}} = {{c.x}}
  let y: {{c.typey}} = {{c.y}}
  {% if c.typex == 'bigint' %}
  arktest.assertEQ(x % BigInt(y), {{c.result}})
  arktest.assertEQ(BigInt(y) % x, {{c.result}})
  {% elif c.typey =='bigint' %}
  arktest.assertEQ(BigInt(x) % y, {{c.result}})
  arktest.assertEQ(y % BigInt(x), {{c.result}})
  {% else %}
  arktest.assertEQ(x % y, {{c.result}})
  arktest.assertEQ(y % x, {{c.result}})
  {% endif %}
}
{% endfor %}
