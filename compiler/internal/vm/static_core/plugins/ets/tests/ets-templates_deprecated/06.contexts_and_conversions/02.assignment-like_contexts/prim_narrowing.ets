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
desc: |-
  22 specific conversions on primitive types are called the narrowing primitive conversions:
    -	short to byte or char
    -	char to byte or short
    -	int to byte, short, or char
    -	long to byte, short, char, or int
    -	float to byte, short, char, int, or long
    -	double to byte, short, char, int, long, or float
---*/

function main() {
  {%- for t in case['types'] %}
  let src_{{loop.index}}: {{t.stype}} = {{t.src|safe}};
  arktest.assertEQ((src_{{loop.index}}.to{{t.dtype}}()), ({{t.dst|safe}}),  "Assert fails on item {{loop.index}}"  )
  {% endfor %}
}
{% endfor %}
