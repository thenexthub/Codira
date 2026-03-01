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

/*---
desc: Create 2D Array with dimension assignable to int type
---*/

enum ImpIntEnum { V1 = 1, V2 = 30 }
enum ExpIntEnum: int { V1 = 1, V2 = 30 }
enum ExpByteEnum: byte { V1 = 1, V2 = 30 }
enum ExpShortEnum: short { V1 = 1, V2 = 30 }

{%- for c in cases %}
{%- set c_id = loop.index %}

function runTest1_{{c_id}}(): boolean {
  let size: {{c.check_type}} = {{c.set_size_value}}
  let a: number[][] = new number[size][1]
  arktest.assertEQ(a.length, 30)
  arktest.assertTrue(a[0][0] instanceof number)
  for (let i = 0; i < 30; i++) {
    arktest.assertEQ(a[i][0], 0.0)
  }
  return true
}

function runTest2_{{c_id}}(): boolean {
  let size: {{c.check_type}} = {{c.set_size_value}}
  let a: number[][] = new number[1][size]
  arktest.assertEQ(a[0].length, 30)
  arktest.assertTrue(a[0][0] instanceof number)
  for (let i = 0; i < 30; i++) {
    arktest.assertEQ(a[0][i], 0.0)
  }
  return true
}

{%- endfor %}

function main(): void {
{%- for c in cases %}
{%- set c_id = loop.index %}
  arktest.assertTrue(runTest1_{{c_id}}())
  arktest.assertTrue(runTest2_{{c_id}}())
{%- endfor %}
}
