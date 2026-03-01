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

{% for item in std_core_array_fill %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{% for item in std_core_array_fill_2 %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{% for item in std_core_array_filter %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{% for item in std_core_array_filter_2 %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{% for item in std_core_array_foreach %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{% for item in std_core_array_foreach_2 %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_includes %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{% for item in std_core_array_indexOf %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{% for item in std_core_array_indexOf_2 %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{% for item in std_core_array_lastIndexOf %}
/*---
desc:  {function: std_core_array_lastIndexOf_{{.item.method_array_param_type}}_{{.item.method_array_item_param_type}}_int}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_map %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_map_2 %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_sort %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{% for item in std_core_array_sort_2 %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{% for item in std_core_array_copyOf %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_copyOf_2 %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_copyOf_3 %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_concat %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_reverse %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_join %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_copyTo %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_copyTo_negative %}
/*---
desc:  {function: {{.item.method_name}}_negative_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}



{%- for item in std_core_array_lowerBoundSearch %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_lowerBoundSearch_2 %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_upperBoundSearch %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_upperBoundSearch_2 %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib.j2' with context %}
{%- endfor %}
