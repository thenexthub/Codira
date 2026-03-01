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
desc: 03.08.04 never Type
name: 03.types/08.reference_types/04.never_type/useWithLambdaAndUnion
tags: []
---*/

const run: (value: int) => int | never = (value: int): int | never => {
    switch (value) {
        case 1:
            return 1;
        case 0:
            return 0;
        default:
            throw new Error("error");
    }
}

function main(): void {
    let ret = run(0)
    arktest.assertEQ(ret, 0)
}
