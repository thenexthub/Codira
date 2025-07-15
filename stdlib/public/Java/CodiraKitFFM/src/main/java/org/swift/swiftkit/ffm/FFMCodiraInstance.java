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

import org.swift.swiftkit.core.SwiftInstance;
import org.swift.swiftkit.core.SwiftInstanceCleanup;

import java.lang.foreign.MemorySegment;

public abstract class FFMSwiftInstance extends SwiftInstance {
    private final MemorySegment memorySegment;

    /**
     * The pointer to the instance in memory. I.e. the {@code self} of the Swift object or value.
     */
    public final MemorySegment $memorySegment() {
        return this.memorySegment;
    }

    /**
     * The Swift type metadata of this type.
     */
    public abstract SwiftAnyType $swiftType();

    /**
     * The designated constructor of any imported Swift types.
     *
     * @param segment the memory segment.
     * @param arena the arena this object belongs to. When the arena goes out of scope, this value is destroyed.
     */
    protected FFMSwiftInstance(MemorySegment segment, AllocatingSwiftArena arena) {
        super(segment.address(), arena);
        this.memorySegment = segment;
    }

    @Override
    public SwiftInstanceCleanup createCleanupAction() {
        var statusDestroyedFlag = $statusDestroyedFlag();
        Runnable markAsDestroyed = () -> statusDestroyedFlag.set(true);

        return new FFMSwiftInstanceCleanup(
                $memorySegment(),
                $swiftType(),
                markAsDestroyed
        );
    }


    /**
     * Returns `true` if this swift instance is a reference type, i.e. a `class` or (`distributed`) `actor`.
     *
     * @return `true` if this instance is a reference type, `false` otherwise.
     */
    public boolean isReferenceType() {
        return this instanceof SwiftHeapObject;
    }
}
