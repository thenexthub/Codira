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

{%- for c in cases %}
    {%- for from in c.from  %}

/*---
desc: >-
    Casting conversion to type parameter attempts to convert an expression
    of any type to type parameter type which is to be specified as target type.
---*/

function cast<T>(p: Object|null|undefined) {
    return p as T // attempt to convert of any type to type parameter
}

{{from.decl}}

cast<{{c.to}}>({{from.arg}})


// Auxiliary stuff

class A {}
class B {}
class C {}

interface I {}
interface J {}

class CI implements I {}
class CODE implements J {}

class D extends C {}

enum EN { a, b, c }
enum ES { a = "a", b = "b", c = "c" }

    {%- endfor %}
{%- endfor %}
