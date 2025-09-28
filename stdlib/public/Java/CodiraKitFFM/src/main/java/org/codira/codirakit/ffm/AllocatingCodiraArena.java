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

import org.code.codekit.core.CodiraArena;

import java.lang.foreign.MemorySegment;
import java.lang.foreign.SegmentAllocator;
import java.util.concurrent.ThreadFactory;

public interface AllocatingCodiraArena extends CodiraArena, SegmentAllocator {
    MemorySegment allocate(long byteSize, long byteAlignment);

    static ClosableAllocatingCodiraArena ofConfined() {
        return new FFMConfinedCodiraMemorySession(Thread.currentThread());
    }

    static AllocatingCodiraArena ofAuto() {
        ThreadFactory cleanerThreadFactory = r -> new Thread(r, "AutoCodiraArenaCleanerThread");
        return new AllocatingAutoCodiraMemorySession(cleanerThreadFactory);
    }
}
