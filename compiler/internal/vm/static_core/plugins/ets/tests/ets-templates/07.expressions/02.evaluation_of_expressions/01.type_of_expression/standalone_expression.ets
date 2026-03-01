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
desc: No target type is expected in the context, then the expression is called a standalone expression
params: Assignment from '{{c.val}}'
---*/

let x = ' ';

let a = {{c.val}}; // no target type is expected

function foo() {
    {{c.val}}; // no target type is expected
}
function main(): void {
  arktest.assertEQ(typeof a, "{{c.type}}")
}

{% endfor %}
