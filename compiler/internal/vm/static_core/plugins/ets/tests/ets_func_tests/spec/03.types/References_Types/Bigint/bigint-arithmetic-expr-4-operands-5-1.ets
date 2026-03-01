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
    let x1: bigint = 2n;
    let x2: bigint = 3n;
    let x3: bigint = -2n;
    let x4: bigint = -3n;
    let x5: bigint = 4n;
    let z = x1++ * x2-- / x3++ - x4-- + (-x5);
    console.log("z = " + z);
    console.log("x1 = " + x1);
    console.log("x2 = " + x2);
    console.log("x3 = " + x3);
    console.log("x4 = " + x4);
    if (z == -4n && x1 == 3n && x2 == 2n && x3 == -1n && x4 == -4n) return 0;
    return 1;
}
