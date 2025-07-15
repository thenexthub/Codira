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
 * Auto-closable version of {@link SwiftArena}.
 */
public interface ClosableSwiftArena extends SwiftArena, AutoCloseable {

    /**
     * Close the arena and make sure all objects it managed are released.
     * Throws if unable to verify all resources have been release (e.g. over retained Swift classes)
     */
    void close();
}
