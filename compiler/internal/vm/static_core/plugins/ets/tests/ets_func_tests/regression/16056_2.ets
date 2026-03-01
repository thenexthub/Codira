/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */
/*---
desc: Issue 16056. Lambda capture cast to float segfault
tags: []
---*/

export type KInt = number

function foo(): KInt {
    let res: KInt = 2;
    return res;
}

function foo2(n: KInt): void {
}



function main(): void {
    let x = foo();
    let array = new Uint8Array(x);
    let result = 1;

    let exec: () => void = (): void => {
        foo2(x)
        result = 0;
    }

    exec();
    arktest.assertEQ(result, 0);
}
