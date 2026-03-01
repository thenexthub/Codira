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
desc: Test {{c.realType}} value '{{c.value}}' assignment (abnormal) for union type ({{c.type}}) as function parameter.
tags: [compile-only, negative]
expected_out: {{c.stdout}}
---*/

{{c.class}}

{{c.enumerate}}

function processValue(value: {{c.type}}): string {
  return `You provided a number: ${value}`;
}

function main(): void {
  const testValue {{c.typeAnno}} = {{c.value}};
  const result = processValue(testValue);
}
{%- endfor %}
