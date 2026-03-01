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

/*---
desc: Ambient declaration does not introduce a new annotation but provides type information
      to use an annotation that must be defined elsewhere
---*/

enum MyEnum { EnumField1, EnumField2 }

{% for case in cases %}

export @interface SomeAnnotation{{ loop.index }} {
   field1: {{ case.type1 }} = {{ case.value1 }}
   field2: {{ case.type2 }} = {{ case.value2 }}
}

@SomeAnnotation{{ loop.index }}
class SomeClass{{ loop.index }} {}

{% endfor %}

function main(): void {
    {%- for case in cases %}
    let sc{{ loop.index }}: SomeClass{{ loop.index }} = new SomeClass{{ loop.index }}
    {%- endfor %}
}
