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
desc: |-
    The type of a nullish-coalescing expression is a normalized union type (see Union Types) formed from the following:
    • Non-nullish variant of the type of the left-hand-side expression; and
    • Type of the right-hand-side expression.
---*/

type int_nullish = int | null | undefined

{{c.decl}}

function setValueOfType(x: {{c.type}}): {{c.type}} {
    return x
}

function main(): void {
  let lhs: {{c.type}} = setValueOfType({{c.value}});
  let rhs: string = "aa";

  let x = lhs{{c.access}} ?? rhs;
  // check that x is exactly of expected type, without extra unions
  let y: {{c.expected_type}} = x
}

{%- endfor %}
