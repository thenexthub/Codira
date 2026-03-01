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
desc: class method Rest parameters with array type.
---*/

class RestClass {
    static sums(...numbers: number[]): number {
        let res = 0;
        for (let num of numbers) {
            res += num;
        }
        return res;
    }

}

function main() {
    let emp: number[] = []
    let tmp: number[] = [1, 2, 3];
    let arr: Array<number> = Array.create<number>(1, 1.0)

    arktest.assertEQ(RestClass.sums(), 0);
    arktest.assertEQ(RestClass.sums(...emp), 0);

    arktest.assertEQ(RestClass.sums(1, 2, 3), 6);
    arktest.assertEQ(RestClass.sums(...tmp), 6);
    arktest.assertEQ(RestClass.sums(...arr), 1);

}
