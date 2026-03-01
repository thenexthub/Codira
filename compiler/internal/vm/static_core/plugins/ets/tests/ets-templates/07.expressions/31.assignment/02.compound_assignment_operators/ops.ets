/*
 * Copyright (c) 2021 -2026 Huawei Device Co., Ltd.
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

{% for operation in operations %}
/*---
desc: >-
  A compound assignment expression of the form E1 op= E2 is equivalent to E1 = ((E1) op (E2)) as T,
  where T is the type of E1, except that E1 is evaluated only once.
params: "{{operation.op|safe}}="
---*/

function main() {
    {% for c in operation.cases %}
    let v_{{loop.index}}:{{c.type.lower()}} = {{c.v}}
    let q_{{loop.index}}:{{c.type.lower()}} = {{c.q}}
    arktest.assertEQ((v_{{loop.index}} {{operation.op}} q_{{loop.index}}).to{{c.type}}(), v_{{loop.index}} {{operation.op}}= q_{{loop.index}})
    {% endfor %}
}
{% endfor %}