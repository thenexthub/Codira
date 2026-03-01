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
desc: If annotation declaration defines only one field, then it can be used with
      a short notation to specify just one expression instead of an object literal
tags: [compile-only]
---*/

{{ case.decl }}

@Retention("SOURCE")
@interface SomeAnnotation {
   field1: {{ case.type }} = {{ case.value }}
}

@SomeAnnotation({{ case.value }})
const c: string = "some_string"

@SomeAnnotation({{ case.value }})
let l: number = 1

type SomeType<T> = FixedArray<T>

@SomeAnnotation({{ case.value }})
const t: SomeType<number> = [100, 200]

let x = @SomeAnnotation({{ case.value }}) () => { return 1 }

@SomeAnnotation({{ case.value }})
function foo(@SomeAnnotation a: int, @SomeAnnotation b?: string) {
    return a;
}

@SomeAnnotation({{ case.value }})
class MyClass {

    @SomeAnnotation({{ case.value }})
    field: int = 1

    @SomeAnnotation({{ case.value }})
    bar(@SomeAnnotation({{ case.value }}) a: int, @SomeAnnotation b?: string) {
        return a
    }
}

@SomeAnnotation({{ case.value }})
interface MyInterface {}

{% endfor %}
