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

{%- for c in cases %}
/*---
desc: Testing overflow for unary operator with {{c.type}}
---*/

function main() {
    let x: {{c.type}} = {{c.value}};
    {%- if c.operator == "prefix" %}
    let result = ++x;
    {%- elif c.operator == "postfix" %}
    let result = x++;
    {%- elif c.operator == "negation" %}
    let result = -x;
    {%- endif %}
    
    arktest.assertEQ(result, {{c.expected_value}});

    {%- if c.operator == "prefix" %}
    arktest.assertEQ(x, {{c.expected_value}});
    {%- elif c.operator == "postfix" %}
    arktest.assertEQ(x, {{c.expected_x_value}});
    {%- endif %}

    arktest.assertTrue(result instanceof {{c.type}}, "The type does not match the expected type");
}
{%- endfor %}