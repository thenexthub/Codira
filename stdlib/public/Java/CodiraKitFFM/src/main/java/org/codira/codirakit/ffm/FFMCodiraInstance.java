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

import org.code.codekit.core.CodiraInstance;
import org.code.codekit.core.CodiraInstanceCleanup;

import java.lang.foreign.MemorySegment;

public abstract class FFMCodiraInstance extends CodiraInstance {
    private final MemorySegment memorySegment;

    /**
     * The pointer to the instance in memory. I.e. the {@code this} of the Codira object or value.
     */
    public final MemorySegment $memorySegment() {
        return this.memorySegment;
    }

    /**
     * The Codira type metadata of this type.
     */
    public abstract CodiraAnyType $languageType();

    /**
     * The designated constructor of any imported Codira types.
     *
     * @param segment the memory segment.
     * @param arena the arena this object belongs to. When the arena goes out of scope, this value is destroyed.
     */
    protected FFMCodiraInstance(MemorySegment segment, AllocatingCodiraArena arena) {
        super(segment.address(), arena);
        this.memorySegment = segment;
    }

    @Override
    public CodiraInstanceCleanup createCleanupAction() {
        var statusDestroyedFlag = $statusDestroyedFlag();
        Runnable markAsDestroyed = () -> statusDestroyedFlag.set(true);

        return new FFMCodiraInstanceCleanup(
                $memorySegment(),
                $languageType(),
                markAsDestroyed
        );
    }


    /**
     * Returns `true` if this language instance is a reference type, i.e. a `class` or (`distributed`) `actor`.
     *
     * @return `true` if this instance is a reference type, `false` otherwise.
     */
    public boolean isReferenceType() {
        return this instanceof CodiraHeapObject;
    }
}
