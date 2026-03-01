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

/*---
desc: >-
    The type of the right-hand-side expression is known from the expression itself,
    then this type becomes the type of the variable as in the example below
---*/

{%- for c in cases %}

let va{{loop.index}} = {{c}};
const ca{{loop.index}} = va{{loop.index}};

const cb{{loop.index}} = {{c}};
let vb{{loop.index}} = cb{{loop.index}};

class C{{loop.index}} {
    fa = ca{{loop.index}};
    fb = vb{{loop.index}};
}
{%- endfor %}

function main() {
{%- for c in cases %}
    new C{{loop.index}}().fa = va{{loop.index}}
    new C{{loop.index}}().fb = cb{{loop.index}}
{% endfor %}
}
