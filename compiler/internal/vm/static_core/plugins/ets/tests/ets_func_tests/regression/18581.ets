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
desc: Issue 18581. Short notion of void-lambdas cannot be compiled
tags: []
---*/

let cc = 0
class InnerOwner {
    outer(callee: () => void) {
        callee()
    }

    inner(val: boolean) {
        cc++
    }

    static staticInner(val: boolean) {
        cc++
    }
}

function outer(callee: () => void) {
    callee()
}


function main() {
    cc = 0
    let io = new InnerOwner
    io.outer(() => io.inner(true))

    outer(() => io.inner(true))


    let callee: () => void = () => io.inner(true)

    io.outer(callee)
    outer(callee)

    io.outer(() => InnerOwner.staticInner(true))
    outer(() => InnerOwner.staticInner(true))

    arktest.assertEQ(6, cc)

}
