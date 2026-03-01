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
desc: Issue 16167 Enum in conditional expression
tags: [compile-only]
---*/

function getFlag(): boolean {
    let cc = Math.random() > 0.5;
    return cc ? true : true;
}

let flag = getFlag();

enum E1 {
    a = 10, b = 20, c = 30
}

enum E2 {
    d = "10", e = "20", f = "30"
}
function f(): void {
    let e1: E1 = E1.a;
    let e2: E2 = E2.d;

    let res = flag ? e1 : e2;
}
