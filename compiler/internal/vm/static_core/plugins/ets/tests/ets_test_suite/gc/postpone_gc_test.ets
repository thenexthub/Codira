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

function check_errors(): void {
    let error_catched: boolean = false;
    try {
        GC.postponeGCEnd();
    } catch(error) {
        if (error instanceof IllegalStateError) {
            error_catched = true;
        }
    }
    arktest.assertTrue(error_catched, "Didn't catch the error");
    error_catched = false;
    try {
        GC.postponeGCStart();
        GC.postponeGCStart();
    } catch(error) {
        if (error instanceof IllegalStateError) {
            error_catched = true;
        }
    }
    arktest.assertTrue(error_catched, "Didn't catch the error");
    GC.postponeGCEnd();
    error_catched = false;
    try {
        GC.postponeGCStart();
        let gc_id = GC.startGC(GC.Cause.THRESHOLD);
        GC.waitForFinishGC(gc_id);
    } catch(error) {
        if (error instanceof IllegalStateError) {
            error_catched = true;
        } else {
            arktest.assertTrue(false, "Unexpected error during GC");
        }
    }
    arktest.assertTrue(error_catched, "Didn't catch the error");
    GC.postponeGCEnd();
}


function main(): void {
    check_errors();
    let elements_count: int = 10000;
    let obj_holder: Object[] = new Object[elements_count];
    let obj_addr: long[] = new long[elements_count];

    GC.postponeGCStart();
    // Run GC for preventing a race between setting postpone GC start
    // and a collection in the GC thread
    try {
        let full_gc_id = GC.startGC(GC.Cause.FULL);
        GC.waitForFinishGC(full_gc_id);
    } catch (e) {
        arktest.assertTrue(false, "Unexpected error during GC");
    }
    // Allocate elements
    for (let i: int = 0; i < elements_count; i++) {
        let new_object: Object = new Object();
        obj_holder[i] = new_object;
        obj_addr[i] = GC.getObjectAddress(new_object)
    }
    try {
        let young_gc_id = GC.startGC(GC.Cause.YOUNG);
        GC.waitForFinishGC(young_gc_id);
    } catch (e) {
        arktest.assertTrue(false, "Unexpected error during GC");
    }
    // Check that we didn't move any object (i.e. didn't waste any time for that)
    for (let i: int = 0; i < elements_count; i++) {
        arktest.assertEQ(obj_addr[i], GC.getObjectAddress(obj_holder[i]), "The object has been moved");
    }
    // Allocate new object in young space
    let young_object: Object = new Object();
    let young_object_addr: long = GC.getObjectAddress(young_object);
    try {
        arktest.assertEQ(GC.ObjectSpaceType.YOUNG, GC.getObjectSpaceType(young_object), "The object is not in young");
        GC.postponeGCEnd();
        // Check that the object will be moved after executing postpone end function.
        let gc_id = GC.startGC(GC.Cause.YOUNG);
        GC.waitForFinishGC(gc_id);
        arktest.assertNE(young_object_addr, GC.getObjectAddress(young_object), "The object hasn't been moved");
    } catch (e) {
        arktest.assertTrue(false, "Unexpected error during GC");
    }
}
