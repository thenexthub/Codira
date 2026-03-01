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
desc: Bitwise expressions for enumeration with explicit type
---*/
enum TypedEnum: {{ c.enum_values }}

function main(): void {

  // AND
  let res_and_1 = TypedEnum.A & TypedEnum.B
  let res_and_2 = TypedEnum.C & TypedEnum.D
  arktest.assertEQ(res_and_1, 0)
  arktest.assertEQ(res_and_2, 2)

  // exclusive OR
  let res_exc_or_1 = TypedEnum.A ^ TypedEnum.B
  let res_exc_or_2 = TypedEnum.C ^ TypedEnum.D
  arktest.assertEQ(res_exc_or_1, 5)
  arktest.assertEQ(res_exc_or_2, 5)

  // inclusive OR
  let res_inc_or_1 = TypedEnum.A | TypedEnum.B
  let res_inc_or_2 = TypedEnum.C | TypedEnum.D
  arktest.assertEQ(res_inc_or_1, 5)
  arktest.assertEQ(res_inc_or_2, 7)

}

{% endfor %}
