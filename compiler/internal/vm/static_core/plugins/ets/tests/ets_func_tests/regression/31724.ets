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
desc: Issue 31724 Es2panda crash, abstract class method without body missed if there's console log
tags: [compile-only, negative]
expected_out: ["Semantic error ESE0017: Only abstract or native methods can't have body."]
---*/

abstract class A{
    private f(): int {return 3}
    calculation(): int
}

class B extends A {
    private f(): int {return 5}
    calculation() {return this.f}
}

let x: Array<A> = [new B(), new B()]
for(let s: A of x) {
    console.log(s.calculation())
}