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
desc: Issue 21143. Assertion. Line should be unreachable
tags: []
---*/


let x: int = 0;

function foo(callback: () => void) {
    callback()
    x++
}

class Bar<T> {
    bar(t: T): void {
        if(t == undefined) {
            return;
        }
        const local = t
        foo(() => { local })
    }
}

function main() {
    x = 0;
    const bar = new Bar<String>();
    bar.bar("12")

    arktest.assertEQ(1, x)
}
