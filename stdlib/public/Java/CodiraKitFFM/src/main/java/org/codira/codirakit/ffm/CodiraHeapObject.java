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


import java.lang.foreign.MemorySegment;

/**
 * Represents a wrapper around a Codira heap object, e.g. a {@code class} or an {@code actor}.
 */
public interface CodiraHeapObject {
    MemorySegment $memorySegment();

    /**
     * Pointer to the instance.
     */
    default MemorySegment $instance() {
        return this.$memorySegment().get(CodiraValueLayout.SWIFT_POINTER, 0);
    }
}
