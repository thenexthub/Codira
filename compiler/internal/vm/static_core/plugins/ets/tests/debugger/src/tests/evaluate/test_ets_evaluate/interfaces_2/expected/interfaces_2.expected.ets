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

package evalPatchFunction;

import { E } from "../interfaces_2.base"

class evalPatchFunction {
    private static evalPatchFunction() {
        let e = new E();
        arktest.assertEQ(e.a_foo(42) ,42);
        arktest.assertEQ(e.b_foo(12345.54321), 12345.54321);
        arktest.assertEQ(e.c_foo(c'a'), c'a');
        arktest.assertEQ(e.d_foo(123.321 as float), 123.321 as float);
    }
}
