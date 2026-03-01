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
desc: >-
    Tuple types conversion is the conversion of one tuple type to another.
    Call context, function, tuple type conversion, implicit
---*/
{{c.decl}}

function zoo(p: [{% for t in c['values'] %}{{t.type}}{% if not loop.last %}, {% endif %}{% endfor %}]): [{% for t in c['values'] %}{{t.type}}{% if not loop.last %}, {% endif %}{% endfor %}] {
    return p
}

function main() {
    let res: [{% for t in c['values'] %}{{t.type}}{% if not loop.last %}, {% endif %}{% endfor %}] = zoo([{% for t in c['values'] %}{{t.expr|safe}}{% if not loop.last %}, {% endif %}{% endfor %}])
    arktest.assertTrue( {% for t in c['values'] %}res[{{loop.index0}}] {{t.eq}} {{t.expr|safe}}{% if not loop.last %} && {% endif %}{% endfor %} )
}
{%- endfor %}
