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

{% for t in cases %}
/*---
desc: Assignment to an array element of reference type.
---*/

interface I {}
interface J {}
interface K {}
class A implements I {}
class B extends A implements J {}

function main(): int {
    let v: Object[] = {{t.init}};
    let w: Object = {{t.expr}};
    try {
      v[0] = w;
      return {{t.fail}};
    } catch (e: ArrayStoreError) {
      return 1 - {{t.fail}};
    } catch (e) {
      return 1;
    }
}
{% endfor %}
