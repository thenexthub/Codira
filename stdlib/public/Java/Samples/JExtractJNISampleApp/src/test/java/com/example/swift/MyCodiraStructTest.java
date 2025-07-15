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

package com.example.swift;

import org.junit.jupiter.api.Test;
import org.swift.swiftkit.core.ConfinedSwiftMemorySession;

import static org.junit.jupiter.api.Assertions.*;

public class MySwiftStructTest {
    @Test
    void init() {
        try (var arena = new ConfinedSwiftMemorySession()) {
            MySwiftStruct s = MySwiftStruct.init(1337, 42, arena);
            assertEquals(1337, s.getCapacity());
            assertEquals(42, s.getLen());
        }
    }

    @Test
    void getAndSetLen() {
        try (var arena = new ConfinedSwiftMemorySession()) {
            MySwiftStruct s = MySwiftStruct.init(1337, 42, arena);
            s.setLen(100);
            assertEquals(100, s.getLen());
        }
    }

    @Test
    void increaseCap() {
        try (var arena = new ConfinedSwiftMemorySession()) {
            MySwiftStruct s = MySwiftStruct.init(1337, 42, arena);
            long newCap = s.increaseCap(10);
            assertEquals(1347, newCap);
            assertEquals(1347, s.getCapacity());
        }
    }
}