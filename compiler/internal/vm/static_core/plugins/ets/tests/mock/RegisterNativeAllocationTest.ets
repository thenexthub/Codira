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

/**
 * Test registerNativeAllocation and registerNativeFree builtins.
 */

class NativeAllocationTest {
    public static allocate_object(): int
    {
        for (let i = 0; i < 100; i++) {
            new ArrayBuffer(1024);
        }
        return 0;
    }

    public static checkRegisterNativeAllocation(): void {
        let total: long = 0;
        let size: long = 16384;
        for (let i = 0; i < 1024; i++) {
            GC.registerNativeAllocation(size);
            total += size;
        }

        while (total > 0) {
            GC.registerNativeFree(size);
            total -= size;
        }
    }

    public static main_method(): int
    {
        for (let i = 0; i < 5; i++) {
            NativeAllocationTest.checkRegisterNativeAllocation();
        }
        return 0;
    }
}
