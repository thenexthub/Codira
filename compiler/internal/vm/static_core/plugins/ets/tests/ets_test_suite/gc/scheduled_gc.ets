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

function simpleTest(): void {
    GC.scheduleGcAfterNthAlloc(2, GC.Cause.YOUNG);
    new Object();
    arktest.assertFalse(GC.isScheduledGCTriggered(), "GC was scheduled before 2nd allocation, current is 1st");
    new Object();
    arktest.assertTrue(GC.isScheduledGCTriggered(), "2 allocation already done, but GC has not been trigger");
}

function incorrectCounterTest(): void {
    let detected = false;
    try {
        GC.scheduleGcAfterNthAlloc(-1, GC.Cause.YOUNG);
    } catch (e) {
        if (e instanceof IllegalArgumentError) {
            detected = true;
        }
    }
    arktest.assertTrue(detected, "scheduleGcAfterNthAlloc must throw IllegalArgumentError for negative counter");
}

function main(): int {
    try {
        simpleTest();
    }
    catch (e) {
        arktest.assertTrue(false, "Simple test must not throw exceptions");
    }
    incorrectCounterTest();
    return 0;
}
