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
desc: A numeric casting conversion occurs if the target type and the expression type are both numeric or char.
params: from NaN to NaN
---*/

function main(): void {
    let s1: float = Float.NaN
    let d1: float = s1
    arktest.assertTrue(Float.isNaN(d1))

    let s2: float = Float.NaN
    let d2: double = s2.toDouble()
    arktest.assertTrue(Double.isNaN(d2))

    let s3: double = Double.NaN
    let d3: double = s3
    arktest.assertTrue(Double.isNaN(d3))

    let s4: double = Double.NaN
    let d4: float = s4.toFloat()
    arktest.assertTrue(Float.isNaN(d4))
}
