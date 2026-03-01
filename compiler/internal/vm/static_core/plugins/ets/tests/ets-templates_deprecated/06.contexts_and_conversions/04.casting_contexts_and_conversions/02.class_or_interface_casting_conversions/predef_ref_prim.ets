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
desc: >-
    Casting contexts allow the use of the narrowing reference conversion
    followed by an unboxing conversion.
---*/

function main(): int {
  {%- for t in c['types'] %}
  let s{{loop.index}}: {{t.stype}} = new {{t.dtype}}({{t.pval}});
  let d{{loop.index}}: {{t.dtype|lower}} = s{{loop.index}} as {{t.dtype|lower}};
  if (d{{loop.index}} != {{t.pval}}) {
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}
