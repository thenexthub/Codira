/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: check remainder of division by zero for char operands
tags: [compile-only, negative]
---*/

function main(): void {
    try {
        const a: char = 1
        const b: char = 0
        const result: char = (a % b) as char
        arktest.assertTrue(false)
    } catch (e: ArithmeticError) {
    }
}
