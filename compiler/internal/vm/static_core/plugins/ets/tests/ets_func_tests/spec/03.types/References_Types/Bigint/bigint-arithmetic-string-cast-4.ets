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
    let x: String = "Hello";
    let x1: String = "World";
    let y: bigint = 123456789123456789123456789n;
    let y1: bigint = 123456789123456789123456789n;
    let z =  y + x + x1+ y1;
    let verdict: int = 0;
    if (z == "123456789123456789123456789HelloWorld123456789123456789123456789") verdict++;
    if (z instanceof String) verdict++;
    if (verdict == 2) return 0;
    return 1;
}
