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

{% for type in types %}

/*---
desc: Type T1 is assignable to type T2 if T1 is type never;
tags: []
---*/

function neverFunction(): never {
  throw new Error("This function never returns");
}

try {
    const n: {{type}} = neverFunction(), m = neverFunction();
    let a: {{type}} = neverFunction();
} catch (e) {
    let n: int = 1;
    arktest.assertEQ(n, 1);

}

{% endfor %}
