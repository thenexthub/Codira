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
desc: |
    Issue 31060. [FE] es2panda crash. Critical issue when accessing extension functions as properties without invocation.
    Gitcode FE issue 8240, simplified code
tags: [compile-only, negative]
expected_out: "Semantic error ESE0087: Property 'foo' does not exist on type 'Int'"
---*/

function foo<T>(this: T): void {}

function main() {
    let x: int = 5
    let a = x.foo
}