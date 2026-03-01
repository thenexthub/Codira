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
desc: >-
    Where one operand is of type T1, and the other operand is of type T2,
    while T1, T2, or both are a union type, then a compile-time error occurs
    if T1 and T2 have no overlap
tags: [compile-only, negative]
---*/
{{c.decl}}

function foo(p: {{c.type1}}, q: {{c.type2}}): void {
    p {{c.op}} q
}

function main(): void {
    {{c.code|indent}}
}
{%- endfor %}
