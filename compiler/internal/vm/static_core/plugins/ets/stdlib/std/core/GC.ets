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

package std.core;

/**
 * @class GC defines methods with GC-specific behavior
 */
export namespace GC {
    /**
     * @see getObjectSpaceType
     */
    export enum ObjectSpaceType {
        OBJECT = 1,
        HUMONGOUS = 2,
        NON_MOVABLE = 3,
        YOUNG = 4,
        TENURED = 5
    }

    /**
     * @see startGC
     * @see scheduleGcAfterNthAlloc
     */
    export enum Cause {
        YOUNG = 0,
        THRESHOLD = 1,
        MIXED = 2,
        FULL = 3
    }

    export const NOT_IN_PLACE_MODE: boolean = false;
    export const IN_PLACE_MODE: boolean = true;


    /**
     * @returns long - free heap space in bytes available for new objects
     */
    export native function getFreeHeapSize(): long;

    /**
     * @returns long - currently used heap space in bytes
     */
    export native function getUsedHeapSize(): long;

    /**
     * @returns long - maximum heap space size in bytes
     */
    export native function getReservedHeapSize(): long;

    /**
     * The function triggers specific GC
     *
     * @param cause cause of requested GC
     * @param callback callback is called in concurrent phase
     * @param inPlaceMode run GC in place mode
     *
     * @throws IllegalArgumentError if current GC does not support requested cause
     *
     * @returns id of started GC.
     * - The id can be passed to waitForFinishGC to ensure the GC is finished.
     * - 0 in case the GC is executed in-place. It means there is no need to wait such GC
     * - -1 in case the task is canceled
     */
    export function startGC(cause: Cause, callback: () => void, inPlaceMode: boolean = GC.NOT_IN_PLACE_MODE): long {
        return startGCImpl(cause, callback as Object, inPlaceMode);
    }

    /**
     * The function triggers specific GC
     *
     * @param cause cause of requested GC
     * @param inPlaceMode run GC in place mode
     *
     * @throws IllegalArgumentError if current GC does not support requested cause
     * @throws UnsupportedOperationError if current GC does not support requested cause
     * @throws IllegalStateError if try to run threshold GC collection not in place after calling postponeGCStart
     *
     * @returns id of started GC.
     * - The id can be passed to waitForFinishGC to ensure the GC is finished.
     * - 0 in case the GC is executed in-place. It means there is no need to wait such GC
     * - -1 in case the task is canceled.
     */
    export function startGC(cause: Cause, inPlaceMode: boolean = GC.NOT_IN_PLACE_MODE): long {
        return startGCImpl(cause, undefined, inPlaceMode);
    }
    
    native function startGCImpl(cause: int, callback: Object | undefined, runInPlace: boolean): long;

    /**
    * The function returns when the specified GC gets finished
    *
    * @param gcId id of the GC which is returned by startGC.
    *                      If gcId is 0 or -1 the function returns immediately.
    */
    export native function waitForFinishGC(gcId: long): void;

    /**
     * Method schedules GC before n-th allocation by setting counter to the specific GC trigger
     * Another call may reset the counter. In this case the last counter will be used to trigger the GC.
     *
     * @param counter number of allocation for trigger GC
     * @param cause cause of requested GC
     *
     * @throws IllegalArgumentError if current GC does not support requested cause
     * @throws UnsupportedOperationError if VM is running with incorrect GC trigger
     */
    native function scheduleGcAfterNthAllocImpl(counter: int, cause: int): void;

    export function scheduleGcAfterNthAlloc(counter: int, cause: Cause) {
        scheduleGcAfterNthAllocImpl(counter, cause);
    }

    /**
     * Checks if GC was triggered
     *
     * @returns true if scheduled GC was triggered for a n-th allocation, false - otherwise
     *
     * @see scheduleGcAfterNthAlloc
     */
    export native function isScheduledGCTriggered(): boolean;

    /**
     * Start postponing GC collection - postpone GC for some time.
     * Disable threshold collections, young collections will promote everything.
     * Require to run postponeGCEnd after finishing critical code.
     *
     * @see postponeGCEnd
     *
     * @throws UnsupportedOperationError if GC doesn't support postponing
     * @throws IllegalStateError if it called after calling postponeGCStart
     *         without calling postponeGCEnd
     */
    export native function postponeGCStart(): void;

    /**
     * End postponing GC collection - finish postpone GC collections.
     * Should be called after postponeGCStart
     *
     * @see postponeGCStart
     *
     * @throws UnsupportedOperationError if GC doesn't support postponing
     * @throws IllegalStateError if it called without calling postponeGCStart
     */
    export native function postponeGCEnd(): void;

    /**
     * Pin object - GC will not move pinned object
     *
     * @param obj non-pinned object for pinning
     *
     * @see unpinObject
     *
     * @throws UnsupportedOperationError if used GC does not support pinning
     */
    export native function pinObject(obj: Object): void;

    /**
     * Unpin pinned object - now GC can move such object if it need
     *
     * @note Unpinning of non-pinned object is unspecified behavior
     *
     * @see pinObject
     * @see allocatePinned<PrimitiveType>Array
     *
     * @param obj pinned object for unpinning
     */
    export native function unpinObject(obj: Object): void;

    /**
     * Allocate array of boolean and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationError if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of boolean
     */
    export native function allocatePinnedBooleanArray(length: long): FixedArray<boolean>;

    /**
     * Allocate array of byte and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationError if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of byte
     */
    export native function allocatePinnedByteArray(length: long): FixedArray<byte>;

    /**
     * Allocate array of char and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationError if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of char
     */
    export native function allocatePinnedCharArray(length: long): FixedArray<char>;

    /**
     * Allocate array of short and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationError if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of short
     */
    export native function allocatePinnedShortArray(length: long): FixedArray<short>;

    /**
     * Allocate array of int and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationError if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of int
     */
    export native function allocatePinnedIntArray(length: long): FixedArray<int>;

    /**
     * Allocate array of long and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationError if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of long
     */
    export native function allocatePinnedLongArray(length: long): FixedArray<long>;

    /**
     * Allocate array of float and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationError if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of float
     */
    export native function allocatePinnedFloatArray(length: long): FixedArray<float>;

    /**
     * Allocate array of double and pin (GC will not move pinned array)
     *
     * @param length count of array elements
     *
     * @see unpinObject
     *
     * @throws NegativeArraySizeError if length is negative
     * @throws UnsupportedOperationError if used GC does not support pinning
     * @throws OutOfMemoryError if VM has not enough heap space for array allocation
     *
     * @returns pinned array of double
     */
    export native function allocatePinnedDoubleArray(length: long): FixedArray<double>;

    /**
     * Returns encoded space type
     *
     * @param obj object for which space type is requested
     *
     * @returns type of space which contains object-argument
     */
    native function getObjectSpaceTypeImpl(obj: Object): int;

    export function getObjectSpaceType(obj: Object): ObjectSpaceType {
        let spaceType: int = getObjectSpaceTypeImpl(obj);
        return ObjectSpaceType.fromValue(spaceType);
    }

    /**
     * Returns object address
     *
     * @param obj object for which address is requested
     *
     * @returns address of object
     */
    export native function getObjectAddress(obj: Object): long;

    /**
     * @param obj object for which size is requested
     * @returns size of the object in bytes
     */
    export native function getObjectSize(obj: Object): long;

    /**
     * @throws NegativeArraySizeError if size is negative
     *
     * @param size: long - number of bytes to be registered by GC as native
     */
    export native function registerNativeAllocation(size: long): void;

    /**
     * @throws NegativeArraySizeError if size is negative
     *
     * @param size: long - number of bytes to be freed by GC
     */
    export native function registerNativeFree(size: long): void;
}
