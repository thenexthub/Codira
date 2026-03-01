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

{% for case in cases %}

/*---
expected_out: {{case.stdout}}
desc: >-
    The keyword keyof is applied to the class or interface type (see Classes and Interfaces).
    The resultant new type is a union of names (as string literal types) of all accessible members of the class or interface type.
tags: [compile-only,negative]
---*/

{{case.decl|indent}}

function main() {
   {{case.check|indent}}
}

{% endfor %}
