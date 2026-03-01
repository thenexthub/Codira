/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

{% for ni in integer_mul %}

/*---
desc: Multiplication of integer numbers
params: {{ni.x}} * {{ni.y}} == {{ni.result}}
---*/

function main(): void {
  let x: {{ni.typex}} = {{ni.x}}
  let y: {{ni.typey}} = {{ni.y}}
  {% if ni.typex == 'bigint' %}
  arktest.assertEQ(x * BigInt(y), {{ni.result}})
  arktest.assertEQ(BigInt(y) * x, {{ni.result}})
  {% elif ni.typey =='bigint' %}
  arktest.assertEQ(BigInt(x) * y, {{ni.result}})
  arktest.assertEQ(y * BigInt(x), {{ni.result}})
  {% else %}
  arktest.assertEQ(x * y, {{ni.result}})
  arktest.assertEQ(y * x, {{ni.result}})
  {% endif %}
}
{% endfor %}
