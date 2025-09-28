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

package org.code.codekitffm;

import com.example.code.MyCodiraStruct;
import org.junit.jupiter.api.Test;
import org.code.codekit.ffm.AllocatingCodiraArena;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class MyCodiraStructTest {

    @Test
    void create_struct() {
        try (var arena = AllocatingCodiraArena.ofConfined()) {
            long cap = 12;
            long len = 34;
            var struct = MyCodiraStruct.init(cap, len, arena);

            assertEquals(cap, struct.getCapacity());
            assertEquals(len, struct.getLength());
        }
    }
}
