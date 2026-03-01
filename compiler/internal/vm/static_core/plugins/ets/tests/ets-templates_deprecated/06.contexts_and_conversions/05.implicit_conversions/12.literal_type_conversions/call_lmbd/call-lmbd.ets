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
desc: Call context, lambda, literal type to type of literal conversion, implicit
params: to {{c.to_type}}
tags: [{{c.tags}}]
---*/

let d: {{c.to_type}};

{%- for v in c['values'] %}
let s{{loop.index}}: {{v.type}} = {{v.expr}};
{%- endfor %}

function main() {
{%- for v in c['values'] %}
    ((p: {{c.to_type}}) => { d = p })(s{{loop.index}});
    arktest.assertEQ(d, {{v.val or v.expr}});
{%- endfor %}
}
{%- endfor %}
