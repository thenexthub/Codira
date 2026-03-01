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
desc: >-
    If type Promise is not a part of a type T declaration, then Awaited<T> leaves T intact.
---*/

async function foo(i: Awaited<Any>, j: Any) {
  arktest.assertEQ(i, j)
  arktest.assertTrue(i instanceof string)
}

async function bar() {
  foo(await Promise.resolve("str"), "str")
}

function main() {
  waitForCompletion(bar)
}
