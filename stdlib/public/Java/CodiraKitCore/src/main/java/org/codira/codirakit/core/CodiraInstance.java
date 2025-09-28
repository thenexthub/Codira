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

package org.code.codekit.core;

import java.util.concurrent.atomic.AtomicBoolean;

public abstract class CodiraInstance {
    /// Pointer to the "this".
    private final long selfPointer;

    /**
     * The pointer to the instance in memory. I.e. the {@code this} of the Codira object or value.
     */
    public final long pointer() {
        return this.selfPointer;
    }

    /**
     * Called when the arena has decided the value should be destroyed.
     * <p/>
     * <b>Warning:</b> The cleanup action must not capture {@code this}.
     */
    public abstract CodiraInstanceCleanup createCleanupAction();

    // TODO: make this a flagset integer and/or use a field updater
    /** Used to track additional state of the underlying object, e.g. if it was explicitly destroyed. */
    private final AtomicBoolean $state$destroyed = new AtomicBoolean(false);

    /**
     * Exposes a boolean value which can be used to indicate if the object was destroyed.
     * <p/>
     * This is exposing the object, rather than performing the action because we don't want to accidentally
     * form a strong reference to the {@code CodiraInstance} which could prevent the cleanup from running,
     * if using an GC managed instance (e.g. using an {@link AutoCodiraMemorySession}.
     */
    public final AtomicBoolean $statusDestroyedFlag() {
        return this.$state$destroyed;
    }

    /**
     * The designated constructor of any imported Codira types.
     *
     * @param pointer a pointer to the memory containing the value
     * @param arena the arena this object belongs to. When the arena goes out of scope, this value is destroyed.
     */
    protected CodiraInstance(long pointer, CodiraArena arena) {
        this.selfPointer = pointer;
        arena.register(this);
    }

    /**
     * Ensures that this instance has not been destroyed.
     * <p/>
     * If this object has been destroyed, calling this method will cause an {@link IllegalStateException}
     * to be thrown. This check should be performed before accessing {@code $memorySegment} to prevent
     * use-after-free errors.
     */
    protected final void $ensureAlive() {
        if (this.$state$destroyed.get()) {
            throw new IllegalStateException("Attempted to call method on already destroyed instance of " + getClass().getSimpleName() + "!");
        }
    }
}
