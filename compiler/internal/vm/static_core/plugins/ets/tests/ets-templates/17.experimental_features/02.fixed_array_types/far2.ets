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
    Testing FixedArray<T> with various types of elements, with casting to Object
    and then to a different type. Expecting ClassCastError.
---*/

function toObj(p: Object|null|undefined): Object {
    return p!
}

let x: FixedArray<{{c.type}}> = [{{c.vals}}]

function main() {
    let obj: Object = toObj(x)
{%- for t in types %}
    {% if t != c.type %}
    try {
        let a: FixedArray<{{t}}> = obj as FixedArray<{{t}}>
        arktest.assertTrue(false, 'ClassCastError was expected')
    } catch (e) {
        arktest.assertTrue(e instanceof ClassCastError, 'ClassCastError was expected')
    }
    {%- endif %}
{%- endfor %}
}
{% endfor %}
