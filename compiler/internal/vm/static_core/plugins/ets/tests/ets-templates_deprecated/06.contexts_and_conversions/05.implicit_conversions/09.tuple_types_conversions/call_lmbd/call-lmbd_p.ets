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
desc: Tuple types conversion is the conversion of one tuple type to another. Call
  context, lambda, tuple type conversion, implicit
name: call-lmbd_n_0
tags:
- compile-only
---*/

let s: [number] = [Double.POSITIVE_INFINITY];

function main() {
    let res1: [Number] =
        ((p: [Number]): [Number] => { return p})([Double.POSITIVE_INFINITY])
    let res2: [Number] =
        ((p: [Number]): [Number] => { return p})(s)
}
