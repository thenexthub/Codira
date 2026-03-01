/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Predefined type declaration.
---*/

function main() {
    let b1: (string | null)[]
    b1 = ["aa", null]
    arktest.assertEQ(b1[0], "aa")
    arktest.assertEQ(b1[1], null)

    let b2: string | null[]
    b2 = [null, null]
    arktest.assertEQ(b2[0], null)
    arktest.assertEQ(b2[1], null)

    let c: () => string | null
    c = (): string | null => { return null }
    arktest.assertEQ(c(), null)

    let d: (() => string) | null
    d = null
    d = (): string => { return "hi" }
    arktest.assertEQ(d(), "hi")
}
