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

function main(): int {
    let obj = new Object();
    arktest.assertLT(0, GC.getObjectAddress(obj), "getObjectAddress for regular object must return value > 0");
    arktest.assertLT(0, GC.getObjectAddress("non-movable string"), "getObjectAddress for non-movable must return value > 0");
    let h_obj: long[] = new long[1024 * 64];
    arktest.assertLT(0, GC.getObjectAddress(h_obj), "getObjectAddress for humongous must return value > 0");
    return 0;
}
