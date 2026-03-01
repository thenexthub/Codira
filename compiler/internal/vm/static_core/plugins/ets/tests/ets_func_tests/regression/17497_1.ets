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
desc: Issue 17497. Implicit lambda parameter discard is forbidden
tags: []
---*/

type TCleanup = () => void

interface StateContext {
    attach(cleanup: TCleanup | undefined): void
}

class StateContextImpl implements StateContext {
    lc: TCleanup = () => {}
    attach(cleanup: TCleanup | undefined): void {
        if(cleanup == undefined) {
            return;
        }
        this.lc = cleanup;
        this.lc()
    }
}

function main() {
    let data = 12;
    const sci = new StateContextImpl

    sci.attach(undefined)
    arktest.assertEQ(12, data)
    let clean: TCleanup | undefined = () => { data = 0 }
    sci.attach(clean)
    arktest.assertEQ(0, data)
}
