//===----------------------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

package org.code.codekit.ffm;

import org.junit.jupiter.api.Test;

import java.lang.foreign.GroupLayout;
import java.lang.foreign.MemorySegment;

public class AutoArenaTest {

    @Test
    @SuppressWarnings("removal") // System.runFinalization() will be removed
    public void cleaner_releases_native_resource() {
        AllocatingCodiraArena arena = AllocatingCodiraArena.ofAuto();

        // This object is registered to the arena.
        var object = new FakeCodiraInstance(arena);
        var statusDestroyedFlag = object.$statusDestroyedFlag();

        // Release the object and hope it gets GC-ed soon

        // noinspection UnusedAssignment
        object = null;

        var i = 1_000;
        while (!statusDestroyedFlag.get()) {
            System.runFinalization();
            System.gc();

            if (i-- < 1) {
                throw new RuntimeException("Reference was not cleaned up! Did Cleaner not pick up the release?");
            }
        }
    }

    private static class FakeCodiraInstance extends FFMCodiraInstance implements CodiraHeapObject {
        public FakeCodiraInstance(AllocatingCodiraArena arena) {
            super(MemorySegment.NULL, arena);
        }

        @Override
        public CodiraAnyType $languageType() {
            return null;
        }
    }
}
