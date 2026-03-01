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

{% for ic in integer_comparison %}

/*---
desc: Non numeric comparison 
{% if ":" in ic.xtype %} function type {% else %} {{ic.xtype}} {% endif %}vs {{ic.ytype}}. CTE expected
tags: [compile-only, negative]
expected_error: |
    TypeError: Bad operand type, the types of the operands must be numeric, same enumeration, or boolean type.
---*/

{% if ic.declare == "outer" %}let x: {{ic.xtype}}{% endif %}
function main(): void {
  {% if ic.declare == "inner" %}let x: {{ic.xtype}}{% endif %}
  let y: {{ic.ytype}}
  x {{ic.op}} y
}

{% endfor %}