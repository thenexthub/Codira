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

{% for scl in c.sub_cases_logic %}

{% set scl = scl|replace("[common_logic]", c.common_logic) %}

{% if 'decl_variants' in c %}
  {% set decl_variants=c.decl_variants %}
{% else %}
  {% set decl_variants=[""] %}
{% endif %}

{% for decl in decl_variants %}

/*---
desc: {{c.desc}}
---*/

{%- set ns = namespace() %}

{%- for cv in global_tests_data[c.check_variants] %}
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
{%- if ns.decl != "" %}
  {{ns.decl|indent(width=2)}}
{%- endif %}

  export function runTest(): boolean {
    {{ns.logic_data|indent(width=4)}}
    return true
  }
}

{%- endfor %}

{%- if ns.first_decl != "" %}
{{ns.first_decl}}
{%- endif %}

function main(): void {
{%- for cv in global_tests_data[c.check_variants] %}
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
