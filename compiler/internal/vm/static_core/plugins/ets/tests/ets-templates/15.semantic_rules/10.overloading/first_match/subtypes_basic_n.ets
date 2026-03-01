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
desc: First match for "basic types"
tags: [compile-only, negative]
expected_out: |- 
    [subtypes_basic_n_0.ets:26:1] Semantic error ESE0131: Function foo with this assembly signature already declared.
    [subtypes_basic_n_0.ets:30:1] Semantic error ESE0131: Function bar with this assembly signature already declared.
---*/

function foo(p: {{c.type_1}}): void {}
function foo(p: {{c.type_2}}): void {}

// reverse order
function bar(p: {{c.type_2}}): void {}
function bar(p: {{c.type_1}}): void {}

{% endfor %}
