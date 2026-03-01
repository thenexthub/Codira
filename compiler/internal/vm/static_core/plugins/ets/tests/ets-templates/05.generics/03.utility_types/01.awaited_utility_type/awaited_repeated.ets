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

{% for c in cases %}
/*---
desc: >-
    Any occurrence of Awaited type is recursively removed until a generic, a
    function, an array, or a tuple type is detected.
---*/


interface I {
  a: number
  b: string
}

enum ENM {Red = 8, Blue, Green}

type p = Awaited<{{c.type}}>
type o = Awaited<p>

function main() {
  let i: o = {{c.value}}
  arktest.assertEQ({{c.checkVal}})
  arktest.assertTrue({{c.checkType}})
}

{% endfor %}
