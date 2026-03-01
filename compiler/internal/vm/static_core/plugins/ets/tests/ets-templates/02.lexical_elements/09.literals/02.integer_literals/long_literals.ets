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
desc: Correct long literals with bases 16, 10, 8 and 2
---*/

{% for c in cases %}
let v{{loop.index}} = {{c['lit']}}
let b{{loop.index}} = new Long({{c['lit']}})
{% endfor %}

function main(): void {
{% for c in cases %}
    arktest.assertEQ(v{{loop.index}}, {{c['val']}})
    arktest.assertEQ(b{{loop.index}}.toLong(), {{c['val']}})
    arktest.assertEQ(b{{loop.index}}.longValue(), {{c['val']}})
{% endfor %}
}
