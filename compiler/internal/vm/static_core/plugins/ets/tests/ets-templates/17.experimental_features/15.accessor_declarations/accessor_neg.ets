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
    Accessor is either a top-level declaration or a declaration inside a namespace that
    can be used to replace a variable and provide additional control in an operation
    of getting or setting a variable value. An accessor can be either a getter or a setter.
tags: [compile-only, negative]
expected_out: "{{c.expected_out}}"
---*/

{{c.decl}}

function main() {
    {{c.use}}
}

{%- endfor %}
