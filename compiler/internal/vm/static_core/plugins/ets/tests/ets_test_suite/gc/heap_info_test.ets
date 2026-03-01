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

import { random } from "std/math"

function checkHeapInfo(): void {
    let reserved_size = GC.getReservedHeapSize();
    let free_size = GC.getFreeHeapSize();
    let used_size = GC.getUsedHeapSize();

    arktest.assertLE(free_size + used_size, reserved_size, "Incorrect heap info statistics");
}

function main(): void {
    checkHeapInfo();
    const MAX_ALLOCATION_COUNT = 100;
    const MAX_ALLOC_SIZE = 100;
    for (let i = 0; i < MAX_ALLOCATION_COUNT; ++i) {
        let alloc_size = (random() * MAX_ALLOC_SIZE).toInt();
        new Object[alloc_size];
        checkHeapInfo();
    }
}