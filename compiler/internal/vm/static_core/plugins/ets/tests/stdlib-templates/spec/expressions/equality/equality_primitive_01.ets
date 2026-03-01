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

{% for item in equality_primitive_01 %}

/*---
desc: {function: {{.item.type1|lower}}_equal_{{.item.type2|lower}}}
tags: []
---*/

function main(): void {
  let left: {{.item.type1}} = ({{.item.value1}}).to{%if item.type1 == 'number'%}Double{%else%}{{.item.type1}}{%endif%}()
  let right: {{.item.type2}} = ({{.item.value2}}).to{%if item.type2 == 'number'%}Double{%else%}{{.item.type2}}{%endif%}()
  let result1: boolean = left == right
  let result2: boolean = left === right
  arktest.assertEQ(result1, result2)
  arktest.assertEQ(result1, {{.item.result}})
}

{%- endfor %}
