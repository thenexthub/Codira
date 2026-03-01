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
desc: >-
  A continue statement with label identifier transfers control out of the enclosing loop statement
  which has the same label identifier. Such statement must be in the body of the surrounding
  function or method otherwise compile-time error is raised by the compiler.
tags: [compile-only, negative]
---*/

function main(): void {
    let sum: int, y: int
    loop2: while (y++ < 100) {
        let x = 0
        loop1: while (x++ < 100) {
            if (x % 2 == 0) {
                continue loop2;
            }
            sum += x
        }
        if (y % 2 == 0) {
            continue loop1; // misplaced label
        }
    }
    arktest.assertEQ(sum, 250000)
}
