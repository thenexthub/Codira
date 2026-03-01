/*
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
*/

/*---
desc: 3.7.1 Integer Types and Operations. BigInt operations
tags: []
---*/

function main() : int {
    let x: bigint = 5n;
    let y: bigint = 3n;
    let z = x % y;
    console.println("z = " + z);
    if (z == 2n) return 0;
    return 1;
}
