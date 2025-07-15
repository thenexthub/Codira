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

package org.swift.swiftkit.ffm;

import org.swift.swiftkit.core.SwiftInstanceCleanup;

import java.lang.foreign.MemorySegment;

public class FFMSwiftInstanceCleanup implements SwiftInstanceCleanup {
    private final MemorySegment selfPointer;
    private final SwiftAnyType selfType;
    private final Runnable markAsDestroyed;

    public FFMSwiftInstanceCleanup(MemorySegment selfPointer, SwiftAnyType selfType, Runnable markAsDestroyed) {
        this.selfPointer = selfPointer;
        this.selfType = selfType;
        this.markAsDestroyed = markAsDestroyed;
    }

    @Override
    public void run() {
        markAsDestroyed.run();

        // Allow null pointers just for AutoArena tests.
        if (selfType != null && selfPointer != null) {
            System.out.println("[debug] Destroy swift value [" + selfType.getSwiftName() + "]: " + selfPointer);
            SwiftValueWitnessTable.destroy(selfType, selfPointer);
        }
    }
}
