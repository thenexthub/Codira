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
    A function with receiver declaration is a top-level declaration (see Top-Level Declarations) that looks almost the same
    as Function Declarations, except that the first parameter is mandatory, and the keyword this is used as its name
tags: [compile-only, negative]
expected_out: "Semantic error ESE0087: Property 'b' does not exist on type 'A'"
---*/

{{c.decl}}

function main() {
    {{c.use|indent}}
}

{%- endfor %}
