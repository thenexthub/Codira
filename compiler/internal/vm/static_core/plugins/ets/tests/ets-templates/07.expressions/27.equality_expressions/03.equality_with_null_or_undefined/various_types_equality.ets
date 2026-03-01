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
desc: >-
    Any entity can be compared to null by using the operators == and !=. This comparison can return true only for
    the entities of nullable types if they actually have the null value during the program execution. It must be known
    during the compilation if other kinds of tests return false.
    Similarly, a comparison to undefined produces false unless the variable being compared is of type undefined,
    or of a union type that has undefined as one of its types.
---*/

{{c.decl}}

function main(): void {
  arktest.assertFalse(v == undefined)
  arktest.assertFalse(v === undefined)
  arktest.assertFalse(v == null)
  arktest.assertFalse(v === null)
  arktest.assertTrue(v != undefined)
  arktest.assertTrue(v !== undefined)
  arktest.assertTrue(v != null)
  arktest.assertTrue(v !== null)
  arktest.assertFalse(undefined == v)
  arktest.assertFalse(undefined === v)
  arktest.assertFalse(null == v)
  arktest.assertFalse(null === v)
  arktest.assertTrue(undefined != v)
  arktest.assertTrue(undefined !== v)
  arktest.assertTrue(null != v)
  arktest.assertTrue(null !== v)
}
{% endfor %}
