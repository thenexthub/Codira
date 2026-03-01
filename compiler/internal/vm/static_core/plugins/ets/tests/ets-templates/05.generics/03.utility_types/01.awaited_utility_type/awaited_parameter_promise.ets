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
    Type Awaited<T> constructs a type which has no type Promise in it.
---*/

async function foo<T, G>(awaited: Awaited<T>, expected: G) {
    arktest.assertEQ(awaited, expected)
    arktest.assertTrue(awaited instanceof {{c.G}})
}

async function bar() {
    foo<{{c.T}},{{c.G}}>(await Promise.resolve({{c.val}}), {{c.check}})
}

function main() {
  waitForCompletion(bar)
}

{% endfor %}
