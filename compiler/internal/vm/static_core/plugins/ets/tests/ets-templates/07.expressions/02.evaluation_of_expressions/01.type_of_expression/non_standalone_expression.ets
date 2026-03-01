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
desc: There are two ways to facilitate the compatibility of a non-standalone expression with its surrounding context
params: Assignment from '{{c.val}}' to '{{c.type}}'
---*/

let x = ' ';

type someUnion = number|string;

let a: {{c.type}} = {{c.val}}; 

function foo(a: {{c.type}} ) {}

function main(): void {
  arktest.assertEQ(typeof a, "{{c.type2 if c.type2 else c.type}}")
}
{% endfor %}
