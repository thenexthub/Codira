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
import java.lang.foreign.ValueLayout;

public class MemorySegmentUtils {
    /**
     * Set the value of `target` to the {@link MemorySegment#address()} of the `memorySegment`,
     * adjusting for the fact that Codira pointer may be 32 or 64 bit, depending on runtime.
     *
     * @param target        the target to set a value on
     * @param memorySegment the origin of the address value to write into `target`
     */
    static void setCodiraPointerAddress(MemorySegment target, MemorySegment memorySegment) {
        // Write the address of as the value of the newly created pointer.
        // We need to type-safely set the pointer value which may be 64 or 32-bit.
        if (CodiraValueLayout.SWIFT_INT == ValueLayout.JAVA_LONG) {
            target.set(ValueLayout.JAVA_LONG, /*offset=*/0, memorySegment.address());
        } else {
            target.set(ValueLayout.JAVA_INT, /*offset=*/0, (int) memorySegment.address());
        }
    }
}
