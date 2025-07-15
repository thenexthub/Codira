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

package org.swift.swiftkit.ffm;

import org.swift.swiftkit.ffm.SwiftRuntime;
import org.swift.swiftkit.ffm.SwiftValueLayout;

import java.lang.foreign.GroupLayout;
import java.lang.foreign.MemoryLayout;
import java.lang.foreign.MemorySegment;

public final class SwiftAnyType {

    private static final GroupLayout $LAYOUT = MemoryLayout.structLayout(
            SwiftValueLayout.SWIFT_POINTER
    );

    private final MemorySegment memorySegment;

    public SwiftAnyType(MemorySegment memorySegment) {
//        if (SwiftKit.getSwiftInt(memorySegment, 0) > 0) {
//            throw new IllegalArgumentException("A Swift Any.Type cannot be null!");
//        }

        this.memorySegment = memorySegment.asReadOnly();
    }

    public MemorySegment $memorySegment() {
        return memorySegment;
    }

    public GroupLayout $layout() {
        return $LAYOUT;
    }

    /**
     * Get the human-readable Swift type name of this type.
     */
    public String getSwiftName() {
        return SwiftRuntime.nameOfSwiftType(memorySegment, true);
    }

    @Override
    public String toString() {
        return "AnySwiftType{" +
                "name=" + getSwiftName() +
                ", memorySegment=" + memorySegment +
                '}';
    }

}
