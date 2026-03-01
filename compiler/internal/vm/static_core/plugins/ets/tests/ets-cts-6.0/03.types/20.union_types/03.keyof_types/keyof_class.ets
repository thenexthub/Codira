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

{% for type in types %}
{% for case in cases %}

/*---
desc: >-
    The keyword keyof is applied to the class or interface type (see Classes and Interfaces).
    The resultant new type is a union of names (as string literal types) of all accessible members of the class or interface type.
tags: [{{case.tags}}]
---*/

{{case.decl}}

{{type}} A {
    {{case.def|indent}}
}

type t = keyof A

function main() {
    // It is allowed to assign variable value of union_1 to another variable of union_2 if
    // union_2 contains or equal union_1.
    // Cannot assign value of union_1 to union_2 otherwise.
    // By assigning union1 to union2 and union2 back to union1 we`ll ensure they are same
    // (until another option will be available here)
    {% if "negative" in case.tags %}
      let union: t = {{case.check[0]}}
    {% else %}
      let union1: {% for i in case.check %}{% if i != case.check[0] %}|{% endif %}{{i}}{% endfor %}{% if case.check[0] != "never" %} = {{case.check[0]}}{% endif %}
      let union2: t = union1
      {% if case.check[0] != "never" %}
        {% for i in case.check %}{% if i != case.check[0] %}union2 = {{i}}{% endif %}
        {% endfor %}
        union1 = union2
        arktest.assertEQ(typeof union2, "string")
      {% endif %}
    {% endif %}
}

{% endfor %}
{% endfor %}
