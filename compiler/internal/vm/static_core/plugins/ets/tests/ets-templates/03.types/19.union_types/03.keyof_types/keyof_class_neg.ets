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
{% for type in types %}
{% for case in cases %}

/*---
desc: The keyword keyof is applied to the class or interface type (see Classes and Interfaces).
tags: [compile-only, negative]
expected_out: {{case.stdout}}
---*/

//extracted negative cases from keyof_class.ets positive cases template

{{case.decl}}

{{type}} A {
    {{case.def|indent}}
}

type t = keyof A

function main() {
    let union: t = {{case.check[0]}}
}

{% endfor %}
{% endfor %}
