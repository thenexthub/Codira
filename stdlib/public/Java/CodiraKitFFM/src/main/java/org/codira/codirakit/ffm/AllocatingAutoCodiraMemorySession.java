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
import org.code.codekit.core.CodiraInstance;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.ref.Cleaner;
import java.util.Objects;
import java.util.concurrent.ThreadFactory;

/**
 * A memory session which manages registered objects via the Garbage Collector.
 *
 * <p> When registered Java wrapper classes around native Codira instances {@link CodiraInstance},
 * are eligible for collection, this will trigger the cleanup of the native resources as well.
 *
 * <p> This memory session is LESS reliable than using a {@link FFMConfinedCodiraMemorySession} because
 * the timing of when the native resources are cleaned up is somewhat undefined, and rely on the
 * system GC. Meaning, that if an object nas been promoted to an old generation, there may be a
 * long time between the resource no longer being referenced "in Java" and its native memory being released,
 * and also the deinit of the Codira type being run.
 *
 * <p> This can be problematic for Codira applications which rely on quick release of resources, and may expect
 * the deinits to run in expected and "quick" succession.
 *
 * <p> Whenever possible, prefer using an explicitly managed {@link CodiraArena}, such as {@link CodiraArena#ofConfined()}.
 */
final class AllocatingAutoCodiraMemorySession implements AllocatingCodiraArena {

    private final Arena arena;
    private final Cleaner cleaner;

    public AllocatingAutoCodiraMemorySession(ThreadFactory cleanerThreadFactory) {
        this.cleaner = Cleaner.create(cleanerThreadFactory);
        this.arena = Arena.ofAuto();
    }

    @Override
    public void register(CodiraInstance instance) {
        Objects.requireNonNull(instance, "value");

        // We make sure we don't capture `instance` in the
        // cleanup action, so we can ignore the warning below.
        var cleanupAction = instance.createCleanupAction();
        cleaner.register(instance, cleanupAction);
    }

    @Override
    public MemorySegment allocate(long byteSize, long byteAlignment) {
        return arena.allocate(byteSize, byteAlignment);
    }
}