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

function match(a: bigint, b: bigint, c: bigint): void {
    let result: bigint = a & b
    arktest.assertEQ(result, c)
}

function main() : int {
    match(0n, 1n, 0n)
    match(65535n, 255n, 255n)

    match(12345678n, 12345678n, 12345678n)
    match(12345678n, -12345678n, 2n)
    match(123456789012n, -123456789012n, 4n)

    match(18446744073709552000n, -256n, 18446744073709551872n)
    match(-256n, 18446744073709552000n, 18446744073709551872n)

    match(18446744073709552000n, 18446744073709552000n, 18446744073709552000n)
    match(18446744073709552000n, -18446744073709552000n, 128n)

    match(1844674407370955200018446744073709552000n, 1844674407370955200018446744073709552000n, 1844674407370955200018446744073709552000n)
    match(1844674407370955200018446744073709552000n, -1844674407370955200018446744073709552000n, 128n)

    return 0
}
