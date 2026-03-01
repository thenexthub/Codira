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
desc: Composite context, interface field, constant narrowing integer conversion, implicit
params: from {{c.from_type}} to {{c.to_type}}
---*/
{%- for v in c['values'] %}
const c{{loop.index}}: {{c.from_type}} = {{v.expr|safe}}
{%- endfor %}

interface J { fld: {{c.to_type}} }

function main() {
{%- for v in c['values'] %}
    let d{{loop.index}}: J = { fld: ({{v.expr|safe}}).to{{c.to_type}}() }  // expression
    arktest.assertEQ(d{{loop.index}}.fld, {{v.val}})

    d{{loop.index}} = { fld: c{{loop.index}}.to{{c.to_type}}() }  // constant
    arktest.assertEQ(d{{loop.index}}.fld, {{v.val}})
{%- endfor %}
}
{%- endfor %}
