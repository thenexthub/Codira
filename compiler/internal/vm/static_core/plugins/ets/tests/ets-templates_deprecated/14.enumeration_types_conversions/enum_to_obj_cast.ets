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
desc: Every enum type is compatible with type Object
---*/

enum {{ c.enum_name }} { Red = {{ c.value1 }}, Green = {{ c.value2 }} }

let c1: {{ c.enum_name }} = {{ c.enum_name }}.Red
let c2: {{ c.enum_name }} = {{ c.enum_name }}.Green

let o1: Object = c1 // Autoboxing of enum type to its reference version
let o2: Object = c2 

// Such reference version type has no name, but can be detected by instanceof
if (o1 instanceof {{ c.enum_name }} && o2 instanceof {{ c.enum_name }}) {
    c1 = o1 as {{ c.enum_name }} // And cast back by cast operator
    c2 = o2 as {{ c.enum_name }}
}

assertEQ(c1.valueOf(), {{ c.value1 }})
assertEQ(c2.valueOf(), {{ c.value2 }})

{% endfor %}