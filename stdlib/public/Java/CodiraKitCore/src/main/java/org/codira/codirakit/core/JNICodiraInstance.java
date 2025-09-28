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

public abstract class JNICodiraInstance extends CodiraInstance {
    /**
     * The designated constructor of any imported Codira types.
     *
     * @param pointer a pointer to the memory containing the value
     * @param arena   the arena this object belongs to. When the arena goes out of scope, this value is destroyed.
     */
    protected JNICodiraInstance(long pointer, CodiraArena arena) {
        super(pointer, arena);
    }

    /**
     * Creates a function that will be called when the value should be destroyed.
     * This will be code-generated to call a native method to do deinitialization and deallocation.
     * <p>
     * The reason for this "indirection" is that we cannot have static methods on abstract classes,
     * and we can't define the destroy method as a member method, because we assume that the wrapper
     * has been released, when we destroy.
     * <p>
     * <b>Warning:</b> The function must not capture {@code this}.
     *
     * @return a function that is called when the value should be destroyed.
     */
    protected abstract Runnable $createDestroyFunction();

    @Override
    public CodiraInstanceCleanup createCleanupAction() {
        final AtomicBoolean statusDestroyedFlag = $statusDestroyedFlag();
        Runnable markAsDestroyed = new Runnable() {
            @Override
            public void run() {
                statusDestroyedFlag.set(true);
            }
        };

        return new JNICodiraInstanceCleanup(this.$createDestroyFunction(), markAsDestroyed);
    }
}
