/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for c in cases %}

{% for scl in c.sub_cases_logic %}

{% for decl in c.decl_variants %}

/*---
desc: {{c.desc}}
---*/

{%- set ns = namespace() %}

{%- if 'check_variants' in c %}
  {% set check_variants=c.check_variants %}
{%- else %}
  {%- set check_variants=[{}] %}
{%- endif %}
{%- for cv in check_variants %}
{%- set cv_id = loop.index %}

{%- set ns.logic_data = scl %}
{%- for k, v in cv.items() %}
  {%- set ns.logic_data = ns.logic_data|replace("[" ~ k ~ "]", v) %}
{%- endfor %}

{%- set ns.decl = decl %}
{%- for k, v in cv.items() %}
  {%- set ns.decl = ns.decl|replace("[" ~ k ~ "]", v) %}
{%- endfor %}

{%- if cv_id == 1 %}
{%- set ns.first_decl = ns.decl %}
{%- set ns.first_logic_data = ns.logic_data %}
{%- endif %}

namespace testNS{{cv_id}} {
  {{ns.decl|indent(width=2)}}

  export function runTest(): boolean {
    {{ns.logic_data|indent(width=4)}}
    return true
  }
}

{%- endfor %}

{{ns.first_decl}}

function main(): void {
{%- for cv in check_variants %}
{%- set cv_id = loop.index %}
{%- if cv_id == 1 %}
  {{ns.first_logic_data|indent(width=2)}}
{% endif %}
  arktest.assertTrue(testNS{{cv_id}}.runTest())
{%- endfor %}
}

{% endfor %}
{% endfor %}
{% endfor %}
