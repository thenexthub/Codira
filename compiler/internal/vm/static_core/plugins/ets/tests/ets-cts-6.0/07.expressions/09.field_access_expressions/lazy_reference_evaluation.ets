/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: If object reference evaluation completes abruptly, then so does the entire field access expression.
tags: []
---*/

class A {
  b: number[] = [1.2345];
}
type NullableA = A | null

function getClassAOrNull(returnA: Boolean = true): NullableA {
    return returnA ? new A : null
}

function main(): void {

    let a = ((returnA: Boolean = true): NullableA => { return returnA ? new A : null })(false);  // Prevent evaluate value of a statically at compile time...
    let rb = a?.b
    let rb0 = a?.b[0]
    arktest.assertEQ(undefined, rb)
    arktest.assertEQ(undefined, rb0)

    a = ((returnA: Boolean = true): NullableA => { return returnA ? new A : null })();  // Prevent evaluate value of a statically at compile time...
    rb0 = a?.b[0]
    arktest.assertEQ(1.2345, rb0)
}
