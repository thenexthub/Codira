/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

{%- for c in cases %}
/*---
desc: |-
    Explicit interface method overload allows declaring a name for a set of interface methods.
    An overloaded entity in an explicit overload declaration can be generic.
    If type arguments are provided explicitly in a call of an overloaded entity,
    then only entities with an equal number of type parameters and type arguments
    are considered during Overload Resolution.
    If type arguments are not provided explicitly, then consideration is given to all entities.
    Overloaded default interface methods, numeric parameters.
---*/
{{c.decl}}

interface I {
    {{c.int|indent}}
}

function main() {
    let a: I = {};
    {{c.use|indent}}
}
{%- endfor %}
