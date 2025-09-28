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

import com.example.code.MyCodiraLibrary;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;

import java.util.Arrays;
import java.util.concurrent.CountDownLatch;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

public class MyCodiraLibraryTest {
    @Test
    void call_helloWorld() {
        MyCodiraLibrary.helloWorld();
    }

    @Test
    void call_globalTakeInt() {
        MyCodiraLibrary.globalTakeInt(12);
    }

    @Test
    void call_globalMakeInt() {
        long i = MyCodiraLibrary.globalMakeInt();
        assertEquals(42, i);
    }

    @Test
    void call_globalTakeIntInt() {
        MyCodiraLibrary.globalTakeIntInt(1337, 42);
    }

    @Test
    void call_writeString_jextract() {
        var string = "Hello Codira!";
        long reply = MyCodiraLibrary.globalWriteString(string);

        assertEquals(string.length(), reply);
    }

    @Test
    void globalVariable() {
        assertEquals(0, MyCodiraLibrary.getGlobalVariable());
        MyCodiraLibrary.setGlobalVariable(100);
        assertEquals(100, MyCodiraLibrary.getGlobalVariable());
    }
}