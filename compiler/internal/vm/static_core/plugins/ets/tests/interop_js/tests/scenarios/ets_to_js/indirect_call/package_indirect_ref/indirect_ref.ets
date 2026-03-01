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

package indirect_call_test_type_ref

function GCODESRuntimeCleanup(): void {
    try {
        // trigger FinalizationRegistry cleanup
        let full_gc_id = GC.startGC(GC.Cause.FULL);
        GC.waitForFinishGC(full_gc_id);
    } catch (e) {
        arktest.assertTrue(false, "Unexpected error during GC");
    }
}

function function_type_ref_array(arg: FixedArray<number>): FixedArray<number> {
    return [...arg];
}

function function_type_ref_tuple(arg: [number, string]): [number, string] {
    return [arg[0], arg[1]];
}

function function_type_ref_map(arg: Map<string, number>): Map<string, number> {
    const KEY = 'key2';
    const VALUE: number = 2;

    let result = new Map<string, number>(arg);
    result.set(KEY, VALUE);

    return result;
  }
