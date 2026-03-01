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
desc: Ambient declaration does not introduce a new annotation but provides type information
      to use an annotation that must be defined elsewhere
      A runtime error occurs if no annotation corresponds to the ambient annotation used in the program
files: ['./ambient_annotations_n.ets']
---*/

import { SomeAnnotation{{ loop.index }} } from "./ambient_annotations_n.d"
import { MyEnum } from "./ambient_annotations_n"

// Need some situation which causes RTE
@SomeAnnotation{{ loop.index }}({field1: {{ case.value1 }}, field2: {{ case.value2}} })
class SomeClass{{ loop.index }} {}


{% endfor %}
