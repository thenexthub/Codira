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

import org.swift.swiftkit.core.ConfinedSwiftMemorySession;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;

final class FFMConfinedSwiftMemorySession extends ConfinedSwiftMemorySession implements AllocatingSwiftArena, ClosableAllocatingSwiftArena {
    final Arena arena;

    public FFMConfinedSwiftMemorySession(Thread owner) {
        super(owner);
        this.arena = Arena.ofConfined();
    }

    @Override
    public void close() {
        super.close();
        this.arena.close();
    }

    @Override
    public MemorySegment allocate(long byteSize, long byteAlignment) {
        return arena.allocate(byteSize, byteAlignment);
    }
}
