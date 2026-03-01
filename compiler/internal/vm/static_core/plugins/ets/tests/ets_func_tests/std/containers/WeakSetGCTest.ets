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

import { launch } from 'std/concurrency'

class Key {
    public readonly id: int

    constructor(id: int) {
        this.id = id
    }
}

const NULL_ID = 0
let reclaimedElemId = NULL_ID

const elementsRegistry = new FinalizationRegistry<Int>((elemId: Int) => {
    reclaimedElemId = elemId
})

function isStwGcType(): boolean {
    try {
        // using the fact that startGC(GC.Cause.YOUNG) request throws when GC type is STW
        GC.startGC(GC.Cause.YOUNG)
    } catch (gcEx) {
        return true
    }

    return false
}

const STW_GC: boolean = isStwGcType()

function createTestWeakSet(elemId: int): WeakSet<Key> {
    const set = new WeakSet<Key>()

    if (elemId != NULL_ID) {
        let elem: Key | null = new Key(elemId)
        set.add(elem)

        elementsRegistry.register(elem, elemId)
        elem = null
    }

    return set
}

function weakSetElementGarbageCollection(): void {
    if (STW_GC) {
        // non-production scenario
        return
    }

    const ELEM_ID = 4
    reclaimedElemId = NULL_ID

    // using coroutine to avoid WeakSet creation function inlining
    const weakSetCreator = launch<WeakSet<Key>, (elemId: int) => WeakSet<Key>>(createTestWeakSet, ELEM_ID)
    const weakSet = weakSetCreator.Await()

    while (reclaimedElemId != ELEM_ID) {
        try {
            const gcId = GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE)
            // We need to switch from main coroutine to coroutine created by FinalizationRegistry because
            // if main coro is executed first, the test will return false
            Coroutine.Schedule()
            if (gcId > 0) {
                GC.waitForFinishGC(gcId)
            }
        } catch (gcEx) {
            arktest.assertTrue(false)
        }
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite('WeakSet GC tests')
    suite.addTest('WeakSet element garbage collection', weakSetElementGarbageCollection)

    return suite.run()
}
