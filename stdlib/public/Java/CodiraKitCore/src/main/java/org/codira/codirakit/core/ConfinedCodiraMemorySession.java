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

import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

public class ConfinedCodiraMemorySession implements ClosableCodiraArena {

    final static int CLOSED = 0;
    final static int ACTIVE = 1;

    final Thread owner;
    final AtomicInteger state;

    final ConfinedResourceList resources;

    public ConfinedCodiraMemorySession() {
        this(Thread.currentThread());
    }

    public ConfinedCodiraMemorySession(Thread owner) {
        this.owner = owner;
        this.state = new AtomicInteger(ACTIVE);
        this.resources = new ConfinedResourceList();
    }

    public void checkValid() throws RuntimeException {
        if (this.owner != null && this.owner != Thread.currentThread()) {
            throw new WrongThreadException(String.format("ConfinedCodira arena is confined to %s but was closed from %s!", this.owner, Thread.currentThread()));
        } else if (this.state.get() < ACTIVE) {
            throw new RuntimeException("CodiraArena is already closed!");
        }
    }

    @Override
    public void close() {
        checkValid();

        // Cleanup all resources
        if (this.state.compareAndSet(ACTIVE, CLOSED)) {
            this.resources.runCleanup();
        } // else, was already closed; do nothing
    }

    @Override
    public void register(CodiraInstance instance) {
        checkValid();

        CodiraInstanceCleanup cleanup = instance.createCleanupAction();
        this.resources.add(cleanup);
    }

    static final class ConfinedResourceList implements CodiraResourceList {
        // TODO: Could use intrusive linked list to avoid one indirection here
        final List<CodiraInstanceCleanup> resourceCleanups = new LinkedList<>();

        void add(CodiraInstanceCleanup cleanup) {
            resourceCleanups.add(cleanup);
        }

        @Override
        public void runCleanup() {
            for (CodiraInstanceCleanup cleanup : resourceCleanups) {
                cleanup.run();
            }
            resourceCleanups.clear();
        }
    }
}
