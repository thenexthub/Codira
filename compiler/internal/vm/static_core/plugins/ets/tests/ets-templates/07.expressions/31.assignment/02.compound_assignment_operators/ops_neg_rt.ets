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

{% for c in cases %}
/*---
desc: >-
  A compound assignment expression of the form E1 op= E2 is equivalent to E1 = ((E1) op (E2)) as T,
  where T is the type of E1, except that E1 is evaluated only once.
params: {{c.type}}, "{{c.op|safe}}="
---*/

function main(): void {
	let v:{{c.type}} = {{c.v}}
	let q:{{c.type}} = {{c.q}}
	arktest.assertEQ((v {{c.op}} q).toString(), (v {{c.op}}= q).toString())
}
{% endfor %}
