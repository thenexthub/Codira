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
desc: Single export directive acts as re-export when the declaration referred to by identifier is imported.
files: ['./export_source/re_export_{{c.id}}.ets',
    {% if c.id < 4 %}
        './export_source/export_as_default_{{c.id}}.ets'
    {% else %}
        './export_source/multiple_export.ets'
    {% endif %} ]
---*/

import {{c.name}} from './export_source/re_export_{{c.id}}';

function main() {
    arktest.assertEQ({{c.expression}}, {{c.value}});
}
{% endfor %}
