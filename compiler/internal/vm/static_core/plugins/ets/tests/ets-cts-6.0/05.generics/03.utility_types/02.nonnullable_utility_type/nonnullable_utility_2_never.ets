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
desc: Type NonNullable<T> constructs a type by excluding null and undefined types
tags: [compile-only, negative]
---*/

type OnlyNullable = null | undefined
type Cleared = NonNullable<OnlyNullable> // type never is expected

function main() {
    let a1: OnlyNullable = null
    let a2: OnlyNullable = undefined
    let b: Cleared = /* @@ label */{{c.value}};
}

/* @@@ label Error TypeError: Type '{{c.error_type}}' cannot be assigned to type 'never'  */

{% endfor %}
