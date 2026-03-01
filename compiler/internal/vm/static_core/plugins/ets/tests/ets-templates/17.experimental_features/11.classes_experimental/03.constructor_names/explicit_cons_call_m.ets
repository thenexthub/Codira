/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
    There are two kinds of explicit constructor calls:
    - Superclass constructor calls (used to call a constructor from the direct superclass) that begin with the keyword super.
    - Other constructor calls that begin with the keyword this (used to call another same-class constructor).
---*/

{{c.decl}}

function main() {
    {{(c.use or '')|indent}}
}
{%- endfor %}
