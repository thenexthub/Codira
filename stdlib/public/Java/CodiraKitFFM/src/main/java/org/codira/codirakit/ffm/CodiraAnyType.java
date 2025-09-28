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

import org.code.codekit.ffm.CodiraRuntime;
import org.code.codekit.ffm.CodiraValueLayout;

import java.lang.foreign.GroupLayout;
import java.lang.foreign.MemoryLayout;
import java.lang.foreign.MemorySegment;

public final class CodiraAnyType {

    private static final GroupLayout $LAYOUT = MemoryLayout.structLayout(
            CodiraValueLayout.SWIFT_POINTER
    );

    private final MemorySegment memorySegment;

    public CodiraAnyType(MemorySegment memorySegment) {
//        if (CodiraKit.getCodiraInt(memorySegment, 0) > 0) {
//            throw new IllegalArgumentException("A Codira Any.Type cannot be null!");
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
     * Get the human-readable Codira type name of this type.
     */
    public String getCodiraName() {
        return CodiraRuntime.nameOfCodiraType(memorySegment, true);
    }

    @Override
    public String toString() {
        return "AnyCodiraType{" +
                "name=" + getCodiraName() +
                ", memorySegment=" + memorySegment +
                '}';
    }

}
