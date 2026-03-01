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

/*---
desc: If a single argument corresponding to the rest parameter has the spread operator,
      then the expression that follows the operator must refer to an array of a type 
      compatible with the type of the array rest parameter
---*/

{{ c.defs }}

class A {
    field: {{ c.to_type }};
    constructor(...a: {{ c.to_type }}) {
        this.field = a;
    }

    bar(...callee: {{ c.to_type }}): {{ c.to_type }} {
        return callee;
    }
}

function foo(...callee: {{ c.to_type }}): {{ c.to_type }} {
    return callee;
}

function main() {

    let arr: {{ c.to_type }} = {{ c.value }};
    let a: A = new A(...arr);
    let method_result = a.bar(...arr);
    let func_result = foo(...arr);

    {%- for res in c['result'] %}
    arktest.assertEQ(method_result[{{ loop.index0 }}], {{ res }})
    arktest.assertEQ(func_result[{{ loop.index0 }}], {{ res }})
    arktest.assertEQ(a.field[{{ loop.index0 }}], {{ res }})
    {%- endfor %}
}

{% endfor %}
