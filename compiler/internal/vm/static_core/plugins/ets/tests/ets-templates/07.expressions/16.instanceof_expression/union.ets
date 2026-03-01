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

{% for c in cases %}
/*---
desc: |-
    The result of an instanceof expression is true if the actual type of evaluated expr is a subtype of T.
    Otherwise, the result is false. Generic type in the form of type name can be used as T operand of an
    instanceof expression. In this case, the check is performed against the type name, and type parameters
    are ignored.
---*/

class OtherType1 {}
class OtherType2 {}

{{c.decl}}

function pass_through(value: Any): Any { return value }

let a1: {{c.type}} = {{c.init1}}
let a2: {{c.type}} = {{c.init2}}

function main() {
    arktest.assertTrue(pass_through(a1) instanceof ({{c.effective_type}}),
        "failed 1 at union of type {{c.type}} effective type {{c.effective_type}}")
    arktest.assertTrue(pass_through(a2) instanceof ({{c.effective_type}}),
        "failed 2 at union of type {{c.type}} effective type {{c.effective_type}}")
    arktest.assertFalse(pass_through(a1) instanceof (OtherType1|OtherType2),
        "failed negative 1 at union of type {{c.type}} effective type {{c.effective_type}}")
    arktest.assertFalse(pass_through(a2) instanceof ((OtherType1|OtherType2)),
        "failed negative 2 at union of type {{c.type}} effective type {{c.effective_type}}")
}
{% endfor %}
