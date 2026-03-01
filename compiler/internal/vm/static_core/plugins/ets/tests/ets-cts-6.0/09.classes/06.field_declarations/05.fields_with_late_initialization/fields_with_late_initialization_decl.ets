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

{% for decl in c.decl_variants %}

{% if 'check_variants' in c %}
  {% set check_variants=c.check_variants %}
{% else %}
  {% set check_variants=[{}] %}
{% endif %}

{% for cv in check_variants %}

{% set logic_data  = "  " ~ (scl|indent(width=2)) %}

{% set ns = namespace() %}

{% set ns.logic_data = logic_data %}
{% for k, v in cv.items() %}
  {% set ns.logic_data = ns.logic_data|replace("[" ~ k ~ "]", v) %}
{% endfor %}
{% set logic_data=ns.logic_data %}

{% set ns.decl = decl %}
{% for k, v in cv.items() %}
  {% set ns.decl = ns.decl|replace("[" ~ k ~ "]", v) %}
{% endfor %}
{% set decl=ns.decl %}

/*---
desc: {{c.desc}}
---*/

{{decl}}

function main(): void {
{{logic_data}}
}

{% endfor %}
{% endfor %}
{% endfor %}
{% endfor %}
