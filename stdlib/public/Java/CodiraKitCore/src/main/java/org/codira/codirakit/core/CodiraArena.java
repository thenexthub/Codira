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

/**
 * A Codira arena manages Codira allocated memory for classes, structs, enums etc.
 * When an arena is closed, it will destroy all managed language objects in a way appropriate to their type.
 *
 * <p> A confined arena has an associated owner thread that confines some operations to
 * associated owner thread such as {@link ClosableCodiraArena#close()}.
 */
public interface CodiraArena  {
    /**
     * Register a Codira object.
     * Its memory should be considered managed by this arena, and be destroyed when the arena is closed.
     */
    void register(CodiraInstance instance);
}

/**
 * Represents a list of resources that need a cleanup, e.g. allocated classes/structs.
 */
interface CodiraResourceList {
    void runCleanup();
}
