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

{% for c in cases %}

/*---
desc: >-
    The chaining operator is allowed in a method call expression for instance methods only.
    Attempting to use it with a static method is syntactically correct but causes a compile-time error.
tags: [compile-only, negative]
expected_out: {{c.stdout}}
---*/

class A {
    static no_params_no_return() {}
    static no_params_with_return(): int {
        return 1;
    }
    static with_params_with_return(p: int): int {
        return 1;
    }
    static with_params_no_return(p: int) {}
}

{{c.decl}}

function main() {
    {{c.use|indent}}
    arktest.assertTrue(false, "expect CTE")
}
{% endfor %}
