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

import org.code.codekit.core.CodiraInstanceCleanup;

import java.lang.foreign.MemorySegment;

public class FFMCodiraInstanceCleanup implements CodiraInstanceCleanup {
    private final MemorySegment selfPointer;
    private final CodiraAnyType selfType;
    private final Runnable markAsDestroyed;

    public FFMCodiraInstanceCleanup(MemorySegment selfPointer, CodiraAnyType selfType, Runnable markAsDestroyed) {
        this.selfPointer = selfPointer;
        this.selfType = selfType;
        this.markAsDestroyed = markAsDestroyed;
    }

    @Override
    public void run() {
        markAsDestroyed.run();

        // Allow null pointers just for AutoArena tests.
        if (selfType != null && selfPointer != null) {
            System.out.println("[debug] Destroy language value [" + selfType.getCodiraName() + "]: " + selfPointer);
            CodiraValueWitnessTable.destroy(selfType, selfPointer);
        }
    }
}
