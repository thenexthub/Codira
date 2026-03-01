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
    A casting conversion from union converts an expression of union type
    to one of the types of the union, or to a type that is derived from such one type.
params: from {{c.from_type|safe}}
---*/
{{c.decl}}
let src: {{c.from_type}}

function main(): int {
{%- for v in c['types'] %}
    src = {{v.v}}
    let d{{loop.index}} = src as {{v.type}}
    if (d{{loop.index}} != {{v.v}}) {
        return 1
    }
{%- endfor %}
    return 0
}
{%- endfor %}
