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
desc: Issue 22177 Arrow function type in namespace causes crash
tags: []
---*/

export declare namespace boo {
    export type Callback = (arg: String) => void
    export namespace foo {
        export type OC = boo.Callback
    }
}

function addBoo(callee: boo.Callback) {
    callee("Hello")
}

function addFoo(callee: boo.foo.OC) {
    callee("World")
}

function main() {
    let hello = ""
    let world = ""

    let cboo: boo.Callback = (arg: String) => { hello = arg };
    let cfoo: boo.foo.OC = (arg: String) => { world = arg };
    addBoo(cboo)
    addFoo(cfoo)

    arktest.assertEQ("Hello", hello)
    arktest.assertEQ("World", world)

    cfoo = cboo;
}
