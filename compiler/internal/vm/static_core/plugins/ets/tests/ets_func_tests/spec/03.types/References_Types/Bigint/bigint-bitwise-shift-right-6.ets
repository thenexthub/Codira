/*
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
*/

/*---
desc: 3.7.1 Integer Types and Operations. BigInt operations
tags: []
---*/

function main() : int {
    let x1: bigint[] = [0n, -0n, 4n, -4n, 9223372036854775807n, -9223372036854775808n];
    let x2: bigint[] = [0n, -0n, 2n, -2n, 8n, -8n, 50n, -50n];

    let expected: bigint[][] = 
    [
        [0n, 0n, 0n, 0n, 0n, 0n, 0n, 0n],
        [0n, 0n, 0n, 0n, 0n, 0n, 0n, 0n],
        [4n, 4n, 1n, 16n, 0n, 1024n, 0n, 4503599627370496n],
        [-4n, -4n, -1n, -16n, -1n, -1024n, -1n, -4503599627370496n],
        [
        9223372036854775807n,
        9223372036854775807n,
        2305843009213693951n,
        36893488147419103228n,
        36028797018963967n,
        2361183241434822606592n,
        8191n,
        10384593717069655255935092751597568n
        ],
        [
        -9223372036854775808n,
        -9223372036854775808n,
        -2305843009213693952n,
        -36893488147419103232n,
        -36028797018963968n,
        -2361183241434822606848n,
        -8192n,
        -10384593717069655257060992658440192n
        ]
    ]

    for (let n = 0; n < x1.length; n++) {
        for (let n2 = 0; n2 < x2.length; n2++) {
            let result = x1[n] >> x2[n2];
            if (result != expected[n][n2]) {
                return 1
            }
        }
    }
    return 0;
}