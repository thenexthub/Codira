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

{% for bw in bitwise_complement %}

/*---
desc: Bitwise complement operator
params: {{bw.type}} {{bw.origin}} => {{bw.dest}}
assertion: If the type of the operand is double or float, then it is truncated first to long or int, respectively. 
           If the type of the operand is byte or short, then the operand is widened to int. 
           If the type of the operand is bigint, then no conversion is required.
---*/

function main(): void {
  let a: {{bw.type}} = {{bw.origin}};
  arktest.assertEQ(~a, {{bw.dest}});
  {% if bw.type == 'long' or bw.type == 'double' %}
  arktest.assertTrue(
    ~a instanceof long, 
    "Type of bitwise complement on {{bw.type}} should be long"
  );
  {% endif %}  
  {% if bw.type == 'byte' or bw.type == 'short' or bw.type == 'int' or bw.type == 'float' %}
  arktest.assertTrue(
    ~a instanceof int,
    "Type of bitwise complement on {{bw.type}} should be int"
  );
  {% endif %}  
  {% if bw.type == 'bigint' %}
  arktest.assertTrue(
    ~a instanceof bigint,
    "Type of bitwise complement on bigint should be bigint"
  );
  {% endif %}  
}

{% endfor %}
