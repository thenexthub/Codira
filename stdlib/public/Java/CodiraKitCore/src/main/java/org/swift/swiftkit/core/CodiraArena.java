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

package org.swift.swiftkit.core;

/**
 * A Swift arena manages Swift allocated memory for classes, structs, enums etc.
 * When an arena is closed, it will destroy all managed swift objects in a way appropriate to their type.
 *
 * <p> A confined arena has an associated owner thread that confines some operations to
 * associated owner thread such as {@link ClosableSwiftArena#close()}.
 */
public interface SwiftArena  {
    /**
     * Register a Swift object.
     * Its memory should be considered managed by this arena, and be destroyed when the arena is closed.
     */
    void register(SwiftInstance instance);
}

/**
 * Represents a list of resources that need a cleanup, e.g. allocated classes/structs.
 */
interface SwiftResourceList {
    void runCleanup();
}
