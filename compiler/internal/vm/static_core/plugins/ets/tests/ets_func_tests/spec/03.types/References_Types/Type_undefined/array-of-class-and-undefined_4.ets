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
desc: 3.8.9 The only value of type undefined is represented by the keyword undefined
name: array-of-class-and-undefined_2.ets
tags:

---*/

class Test {
    n : number | undefined = 1;
}

function check(b : boolean): int {
    return b ? 0 : 1;
}

function callMe(a : Array<Test | undefined>): int {
    let result: int = 0;
    result += check(a?.length !== undefined);
    result += check(a.length  === 2);
    result += check(a!.length >   0);
    result += check(a!.length === 2);
    result += check(a?.length === 2);

    result += check(a[0]!.n === 42);

    result += check(a[0]!.n! >    0);
    result += check(a[0]!.n! === 42);
    return result;
}

function main(): int {
    let result: int = 0;
    result += callMe([{n: 42}, undefined]);
    result += callMe([{n: 42} as Test, undefined]);
    return result;
}
