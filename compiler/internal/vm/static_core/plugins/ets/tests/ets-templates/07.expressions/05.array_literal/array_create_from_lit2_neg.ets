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

{% for c in cases %}

{% if 'sub_cases_logic' in c %}
  {% set sub_cases_logic=c.sub_cases_logic %}
{% else %}
  {% set sub_cases_logic=[""] %}
{% endif %}

{% for scl in sub_cases_logic %}

/*---
desc: {{c.desc}}
tags: [compile-only, negative]
---*/

{{c.decl}}

function main(): void {
{% if 'logic' in c %}  {{c.logic|indent(width=2)}}
{% endif -%}
{% if scl != "" %}  {{scl|indent(width=2)}}
{% endif -%}
}

{% endfor %}
{% endfor %}
