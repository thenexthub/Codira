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
  A continue statement stops execution of the current loop iteration and passes control
  to the next iteration with proper check of the loop exit condition depending on the loop kind.
---*/

function main(): void {
    let sum: int, y: int
    while (y++ < 100) {
        let x = 0
        while (x++ < 100) {
            sum += x
        }
        if (y % 2 == 0) {
            continue; // the outer loop only
        }
    }
    arktest.assertEQ(sum, 505000)
}
