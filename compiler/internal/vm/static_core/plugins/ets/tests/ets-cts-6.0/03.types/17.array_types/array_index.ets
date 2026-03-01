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
desc: The first element of the array is stored at the 0th index, 2nd element is stored on 1st index and so on
---*/

function main(): void {
    let x: int[] = [1, 2, 3];
    arktest.assertTrue(x[0 as byte] == 1);
    arktest.assertTrue(x[1 as short] == 2);
    arktest.assertTrue(x[0 as int] == 1);
    arktest.assertTrue(x[1 as char] == 2);
}
