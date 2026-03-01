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

{% for case in cases %}
/*---
desc: >-
    A value x of primitive type T is first converted to a reference value as if by
    giving it as an argument to an appropriate class instance creation expression.
---*/

function main() {
  {%- for t in case['types'] %}
  let src_{{loop.index}}: {{t.type}} = {{t.val|safe}}{{t.type2}};
  let dst_{{loop.index}}: string = "" + src_{{loop.index}};
  let exp_{{loop.index}}: string = (new {{t.type|capitalize}}(src_{{loop.index}})).toString();
  arktest.assertEQ( dst_{{loop.index}}, exp_{{loop.index}} )
  {% endfor %}
}
{% endfor %}
