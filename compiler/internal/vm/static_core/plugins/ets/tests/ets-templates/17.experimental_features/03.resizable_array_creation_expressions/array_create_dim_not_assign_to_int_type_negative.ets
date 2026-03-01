/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
    An array creation expression creates an object that is a new array with the elements of
    the type specified by typeReference.
    The type of each dimension expression must be assignable (see Assignability) to an int type.
    Otherwise, a compile-time error occurs.
tags: [compile-only, negative]
expected_out: "{{c.expected_out}}"
---*/

{{c.decl}}

function main() {
    {{c.use|indent}}
}
{%- endfor %}
