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

package std.core

/**
 * Returns RuntimeLinker instance used for bootloaded classes.
 */
export function getBootRuntimeLinker(): BootRuntimeLinker {
    return BootRuntimeLinker.instance
}

export native function getNearestNonBootRuntimeLinker(): RuntimeLinker | undefined

export final class BootRuntimeLinker extends RuntimeLinker {
    public static final instance: BootRuntimeLinker = new BootRuntimeLinker()

    private constructor() {
        super()
    }

    /**
     * Find not yet loaded class and load it into boot linker.
     *
     * @param clsName name of the class.
     *
     * @param init indicator whether class must be initialized.
     *
     * @returns Class if it was found, undefined otherwise.
     */
    public native override findAndLoadClass(clsName: string, init: boolean): Class | undefined
}
