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
desc: Issue 14597 array type cannot resolve from literal
---*/

interface MyInterface {}
class MyClass {}

function instantiateEmptyArray(): {{c.type}}[] {
  return [] as {{c.type}}[]
}

function instantiateArrayWithValues(): {{c.type}}[] {
  return [{{c.vals|join(', ')}}] as {{c.type}}[]
}

function main(): void {
  let a: {{c.type}}[] = instantiateEmptyArray()
  arktest.assertEQ(a.length, 0)

  let b: {{c.type}}[] = instantiateArrayWithValues()
  arktest.assertEQ(b.length, {{c.vals|length}})
  arktest.assertTrue(b[0] instanceof {{c.type}})
  {% if 'no_check_vals' not in c -%}
  {% for v in c.vals -%}
  arktest.assertEQ(b[{{loop.index0}}], {{v}})
  {% endfor %}
  {%- endif %}
}

{% endfor %}
