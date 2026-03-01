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

{% for ni in nan %}

/*---
desc: Exponentiation with NaN
params: {{ni.x}} ** {{ni.y}} == {{ni.result}}
---*/

function main(): void {
  let res1 = {{ni.x}} ** {{ni.y}}
  let res2 = Math.pow({{ni.x}}, {{ni.y}})
{% if ni.y == 0 or ni.x == 1 %}
  arktest.assertTrue(res1 == res2 && res2 == {{ni.result}})
{% elif ni.x == 1 and (ni.y == "Double.POSITIVE_INFINITY" or ni.x == "Double.NEGATIVE_INFINITY") %}
  arktest.assertTrue(res1 == {{ni.result}} && res2 == 1)
{% else %}
  arktest.assertTrue(Double.isNaN(res1) && Double.isNaN(res2))
{% endif %}
}

{% endfor %}
