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
    if the expression 'e' is not a nullable type, then the operator '!' has no effect.
    If the result of the evaluation of e is not equal to null or undefined, then the
    result of e! is the outcome of the evaluation of e. If the result of the evaluation
    of e is equal to null or undefined, then NullPointerError is thrown.
tags: [compile-only, negative]
---*/
{{c.decl}}

function main() {
    {{c.use|indent}}
}
{%- endfor %}
