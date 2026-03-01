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
desc: Composite context, class field, widening reference conversion, implicit
params: from {{c.from_type}}
tags: [{{c.tags}}]
---*/
{{c.decl}}

{%- for t in c['values'] %}
class A{{loop.index}} { fld: {{t.to_type}} {%- if t.init_value is defined %} = {{t.init_value}}{%- endif %} }
let s{{loop.index}}: {{c.from_type}} = {{t.expr|safe}};
{%- endfor %}

function main(): int {
{%- for t in c['values'] %}
    let d{{loop.index}}: A{{loop.index}} = { fld: s{{loop.index}} }
    if (d{{loop.index}}.fld !== s{{loop.index}}) {
        return 1
    }
{%- endfor %}
    return 0
}
{%- endfor %}
