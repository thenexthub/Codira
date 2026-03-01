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
desc: Declaration context, static field declaration, widening primitive conversion, implicit
params: from {{c.from_type}} to {{c.to_type}}
---*/
{{c.defs}}
{%- for v in c['values'] %}
class A{{loop.index}} {
    private static s: {{c.from_type}} = {{v.expr|safe}}
    public static d: {{c.to_type}} = A{{loop.index}}.s
}
{%- endfor %}

function main(): void {
{%- for v in c['values'] %}
    arktest.assertEQ(A{{loop.index}}.d, {{v.val}})
{%- endfor %}
}
{%- endfor %}
