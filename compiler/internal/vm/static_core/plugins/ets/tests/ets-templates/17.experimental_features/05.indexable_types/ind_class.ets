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
    If a class or an interface declares one or two functions with names
    $_get and $_set, and signatures (index: Type1): Type2 and
    (index: Type1, value: Type2) respectively, then an indexing expression
    can be applied to variables of such types
tags: [{{c.tags}}]
---*/
{{c.decl}}

abstract class Base<T, U> {
    ind: T
    val: U
    abstract $_get(i: T): U|undefined;
    $_set(i: T, v: U) {
        this.ind = i
        this.val = v
    }
}
class A<T, U> extends Base<T, U> {
    $_get(i: T): U|undefined {
        return i == this.ind ? this.val : undefined
    }
}

function main() {

    let a: A<{{c.ind_type}}, {{c.val_type}}> = new A<{{c.ind_type}}, {{c.val_type}}>
    {% for t in c['values'] %}
	arktest.assertEQ(a[{{t.ind}}], undefined)
	a[{{t.ind}}] = {{t.val}}
	arktest.assertEQ(a[{{t.ind}}], {{t.val}})
    {% endfor %}
    
}
{% endfor %}
