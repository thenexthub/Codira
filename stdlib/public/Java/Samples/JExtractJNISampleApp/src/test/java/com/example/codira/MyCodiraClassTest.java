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

package com.example.code;

import org.junit.jupiter.api.Test;
import org.code.codekit.core.ConfinedCodiraMemorySession;

import static org.junit.jupiter.api.Assertions.*;

public class MyCodiraClassTest {
    @Test
    void init_noParameters() {
        try (var arena = new ConfinedCodiraMemorySession()) {
            MyCodiraClass c = MyCodiraClass.init(arena);
            assertNotNull(c);
        }
    }

    @Test
    void init_withParameters() {
        try (var arena = new ConfinedCodiraMemorySession()) {
            MyCodiraClass c = MyCodiraClass.init(1337, 42, arena);
            assertNotNull(c);
        }
    }

    @Test
    void sum() {
        try (var arena = new ConfinedCodiraMemorySession()) {
            MyCodiraClass c = MyCodiraClass.init(20, 10, arena);
            assertEquals(30, c.sum());
        }
    }

    @Test
    void xMultiplied() {
        try (var arena = new ConfinedCodiraMemorySession()) {
            MyCodiraClass c = MyCodiraClass.init(20, 10, arena);
            assertEquals(200, c.xMultiplied(10));
        }
    }

    @Test
    void throwingFunction() {
        try (var arena = new ConfinedCodiraMemorySession()) {
            MyCodiraClass c = MyCodiraClass.init(20, 10, arena);
            Exception exception = assertThrows(Exception.class, () -> c.throwingFunction());

            assertEquals("languageError", exception.getMessage());
        }
    }

    @Test
    void constant() {
        try (var arena = new ConfinedCodiraMemorySession()) {
            MyCodiraClass c = MyCodiraClass.init(20, 10, arena);
            assertEquals(100, c.getConstant());
        }
    }

    @Test
    void mutable() {
        try (var arena = new ConfinedCodiraMemorySession()) {
            MyCodiraClass c = MyCodiraClass.init(20, 10, arena);
            assertEquals(0, c.getMutable());
            c.setMutable(42);
            assertEquals(42, c.getMutable());
        }
    }

    @Test
    void product() {
        try (var arena = new ConfinedCodiraMemorySession()) {
            MyCodiraClass c = MyCodiraClass.init(20, 10, arena);
            assertEquals(200, c.getProduct());
        }
    }

    @Test
    void throwingVariable() {
        try (var arena = new ConfinedCodiraMemorySession()) {
            MyCodiraClass c = MyCodiraClass.init(20, 10, arena);

            Exception exception = assertThrows(Exception.class, () -> c.getThrowingVariable());

            assertEquals("languageError", exception.getMessage());
        }
    }

    @Test
    void mutableDividedByTwo() {
        try (var arena = new ConfinedCodiraMemorySession()) {
            MyCodiraClass c = MyCodiraClass.init(20, 10, arena);
            assertEquals(0, c.getMutableDividedByTwo());
            c.setMutable(20);
            assertEquals(10, c.getMutableDividedByTwo());
            c.setMutableDividedByTwo(5);
            assertEquals(10, c.getMutable());
        }
    }

    @Test
    void isWarm() {
        try (var arena = new ConfinedCodiraMemorySession()) {
            MyCodiraClass c = MyCodiraClass.init(20, 10, arena);
            assertFalse(c.isWarm());
        }
    }
}