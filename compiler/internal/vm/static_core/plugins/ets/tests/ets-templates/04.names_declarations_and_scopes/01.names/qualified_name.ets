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

{% for qname in qualified_name %}
/*---
desc: Qualified name
---*/

class Clazz{{loop.index0}} {
    static staticField: int = 1;
    static staticMethod(): int { return 2; }

    field: int = 3;
    method(): int { return 4; }
}

let instance{{loop.index0}}: Clazz{{loop.index0}};
instance{{loop.index0}} = new Clazz{{loop.index0}}();

function main(): void {
    arktest.assertEQ({{qname}})
}
{% endfor %}
