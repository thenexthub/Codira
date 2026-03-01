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

{% for item in std_serialization_json_static_2 %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_serialization_json_lib.j2' %}


{% for class_name in item.data_class_name_list %}
class {{.class_name}} {
    constructor({{.item.data_class_attributes_index[class_name]|join(',')}}) {
{%- for param_tuple in item.data_class_attributes_index[class_name] %}
	this.{{.param_tuple.split(":")|first}} = {{.param_tuple.split(":")|first}}
{%- endfor %}
    }
{%- for param_tuple in item.data_class_attributes_index[class_name] %}
    {{.param_tuple}}
{%- endfor %}
}
{% endfor %}
{% endfor %}
