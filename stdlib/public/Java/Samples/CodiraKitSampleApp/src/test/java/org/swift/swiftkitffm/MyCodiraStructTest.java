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

package org.swift.swiftkitffm;

import com.example.swift.MySwiftStruct;
import org.junit.jupiter.api.Test;
import org.swift.swiftkit.ffm.AllocatingSwiftArena;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class MySwiftStructTest {

    @Test
    void create_struct() {
        try (var arena = AllocatingSwiftArena.ofConfined()) {
            long cap = 12;
            long len = 34;
            var struct = MySwiftStruct.init(cap, len, arena);

            assertEquals(cap, struct.getCapacity());
            assertEquals(len, struct.getLength());
        }
    }
}
