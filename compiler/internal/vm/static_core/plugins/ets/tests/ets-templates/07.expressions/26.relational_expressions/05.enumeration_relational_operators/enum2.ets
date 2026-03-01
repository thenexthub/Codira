/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
    If both operands are of the same Enumeration type, then Numerical Relational Operators
    or String Relational Operators are used depending on the kind of enumeration constant value.
    Otherwise, a compile-time error occurs.
---*/

{{c.decl}}

function main() {
    {{c.checks|indent}}
}
{%- endfor %}
