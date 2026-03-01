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
    let sum: int
    for (let x: int = 0; x < 100; x++) {
        if (x % 2 == 0) {
            continue; // a single loop
        }
        sum += x
    }
    arktest.assertEQ(sum, 2500)
}
