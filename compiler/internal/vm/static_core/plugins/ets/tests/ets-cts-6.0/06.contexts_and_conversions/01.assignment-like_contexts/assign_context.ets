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
desc: Assignment contexts
params: Conversion from {{c.alias_type}} to {{c.dst_type}}
---*/

function main(): void {
  {% for v in c['values'] %}
  let src_{{loop.index}}: {{c.alias_type}} = {{v|safe}};
  let dst_{{loop.index}}: {{c.dst_type}} = src_{{loop.index}};
  arktest.assertEQ(dst_{{loop.index}}, src_{{loop.index}})
  {% endfor %}
}
{% endfor %}
