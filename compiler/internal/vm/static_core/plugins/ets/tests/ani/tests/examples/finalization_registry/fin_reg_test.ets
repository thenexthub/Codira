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

final class CallbackHolder {
    private cbPtr: long = 0

    private constructor(cbPtr: long) {
        this.cbPtr = cbPtr
    }

    public native invoke(arg: Object | undefined): void

    public static createCallback(cb: CallbackHolder): () => void {
        return (): void => {
            cb.invoke(undefined)
        }
    }

    public static createCallbackWithArg(cb: CallbackHolder): (arg: Object | undefined) => void {
        return (arg: Object | undefined): void => {
            cb.invoke(arg)
        }
    }

    // Required only for purposes of test, do not use it in production code
    public static doFullGC() {
        GC.waitForFinishGC(GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE));
        // We need to switch from main coroutine to coroutine created by FinalizationRegistry because 
        // if main coro is executed first, the test will return false
        Coroutine.Schedule()
    }
}

class FinalizationMarker {
    private finalized: boolean

    public constructor() {
        this.finalized = false
    }
}
