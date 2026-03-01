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

class PinningTestsuite extends arktest.ArkTestsuite {
    constructor() {
        super("PinObject");
    }

    override before(): void {
        try {
            GC.waitForFinishGC(GC.startGC(GC.Cause.FULL)); // Clear heap
        } catch(e) {
            arktest.assertTrue(!"Unexpected error during FULL GC");
        }
    }
}

function main(): int {
    let pinObjTestsuite = new PinningTestsuite();
    pinObjTestsuite.addTest("SimpleTest", (): void =>
    {
        let obj: Object = new Object();
        GC.pinObject(obj);
        arktest.assertEQ(GC.getObjectSpaceType(obj), GC.ObjectSpaceType.YOUNG,
                 "The object before GC must be placed in young space");
        let addr_before_gc: long = GC.getObjectAddress(obj);
        // Run GC
        let gc_id = GC.startGC(GC.Cause.MIXED);
        GC.waitForFinishGC(gc_id);
        let addr_after_gc: long = GC.getObjectAddress(obj);
        arktest.assertEQ(GC.getObjectSpaceType(obj), GC.ObjectSpaceType.TENURED,
                 "The pinned object after GC must be placed in tenured space");
        GC.unpinObject(obj);

        arktest.assertEQ(addr_before_gc, addr_after_gc, "Pinned object must has a fixed address");
    });
    pinObjTestsuite.addTest("AllocatePinnedArrayTest", (): void =>
    {
        let obj = GC.allocatePinnedIntArray(10);
        arktest.assertEQ(GC.getObjectSpaceType(obj), GC.ObjectSpaceType.TENURED, "The pinned array after GC must be placed in tenured space");
        let addr_before_gc: long = GC.getObjectAddress(obj);
        // Run GC
        let gc_id = GC.startGC(GC.Cause.MIXED);
        GC.waitForFinishGC(gc_id);
        let addr_after_gc: long = GC.getObjectAddress(obj);
        GC.unpinObject(obj);
        arktest.assertEQ(addr_before_gc, addr_after_gc, "Pinned array must has a fixed address");
    });
    pinObjTestsuite.addTest("AllocateNegativeSizePinnedArrayTest", (): void =>
    {
        let expectedError = new NegativeArraySizeError("The value must be non negative");
        arktest.expectError((): void => { GC.allocatePinnedBooleanArray(-1) }, expectedError);
        arktest.expectError((): void => { GC.allocatePinnedByteArray(-1) }, expectedError);
        arktest.expectError((): void => { GC.allocatePinnedCharArray(-1) }, expectedError);
        arktest.expectError((): void => { GC.allocatePinnedShortArray(-1) }, expectedError);
        arktest.expectError((): void => { GC.allocatePinnedIntArray(-1) }, expectedError);
        arktest.expectError((): void => { GC.allocatePinnedLongArray(-1) }, expectedError);
        arktest.expectError((): void => { GC.allocatePinnedFloatArray(-1) }, expectedError);
        arktest.expectError((): void => { GC.allocatePinnedDoubleArray(-1) }, expectedError);
    });
    return pinObjTestsuite.run();
}
