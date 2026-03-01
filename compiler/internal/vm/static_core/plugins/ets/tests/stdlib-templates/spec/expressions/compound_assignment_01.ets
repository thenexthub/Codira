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

{% for item in compound_assignment_01 %}

/*---
desc: {function: {{.item.type1|lower}}_{{.item.op_name}}_{{.item.type2|lower}}}
tags: []
---*/

type EasingCurve = (value: {{.item.type1}}) => {{.item.type1}}

function foo(count: {{.item.type2}}): EasingCurve {
  return (value: {{.item.type1}}) => {
    value {{.item.operation}}= count
    return value
  }
}

function main(): void {
  let val1: {{.item.type2}} = {{.item.value2}}
  let val2: {{.item.type1}} = {{.item.value1}}
  let curve: EasingCurve = foo(val1)
  let value = curve(val2)
  arktest.assertEQ(value {{.item.casting}}, {{.item.result}}, "value = " + value)
}

{%- endfor %}
