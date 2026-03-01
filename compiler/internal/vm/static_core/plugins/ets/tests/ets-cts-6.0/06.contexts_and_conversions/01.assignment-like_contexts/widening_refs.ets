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
desc: Assignment contexts allow the use a widening reference conversion.
params: Assignment of '{{case.src_type|safe}}' to '{{case.dst_type|safe}}'
---*/
interface I {}
class A implements I {}
class B extends A {}
class C extends B {}

{{case.defs|safe}}

function main(): void {
  let src: {{case.src_type|safe}} = new {{case.src_class|safe}}();
  let dst: {{case.dst_type|safe}} = src;  // this assignment is tested
  arktest.assertEQ(dst, src)
}
{% endfor %}
