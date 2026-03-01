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
desc: Issue 17498. Non-void functions should have return statement on all control-flow paths
tags: [compile-only, negative]
---*/

class C<T> {
    public field: T;
}

function foo() {
    let ci = new C<undefined>()
    if(false) return ci.field;

    let flag0: boolean = false
    let flag1: boolean = true
    let flag2: boolean = false
    let flag3: boolean = true
    let flag4: boolean = false
    let flag5: boolean = true

    if(flag0) {
        if(flag1) {
            if(flag2) {
                if(flag4) {
                    if(flag5) return ci.field;
                }
            } else {
            }
        }
    }
}
