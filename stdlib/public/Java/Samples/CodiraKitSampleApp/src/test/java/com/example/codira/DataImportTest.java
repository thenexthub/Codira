//===----------------------------------------------------------------------===//
//
// This source file is part of the Codira.org open source project
//
// Copyright (c) 2025 Apple Inc. and the Codira.org project authors
// Licensed under Apache License v2.0
//
// See LICENSE.txt for license information
// See CONTRIBUTORS.txt for the list of Codira.org project authors
//
// SPDX-License-Identifier: Apache-2.0
//
//===----------------------------------------------------------------------===//

package com.example.code;

import org.junit.jupiter.api.Test;
import org.code.codekit.ffm.AllocatingCodiraArena;

import static org.junit.jupiter.api.Assertions.*;

public class DataImportTest {
    @Test
    void test_Data_receiveAndReturn() {
        try (var arena = AllocatingCodiraArena.ofConfined()) {
            var origBytes = arena.allocateFrom("foobar");
            var origDat = Data.init(origBytes, origBytes.byteSize(), arena);
            assertEquals(7, origDat.getCount());

            var retDat = MyCodiraLibrary.globalReceiveReturnData(origDat, arena);
            assertEquals(7, retDat.getCount());
            retDat.withUnsafeBytes((retBytes) -> {
                assertEquals(7, retBytes.byteSize());
                var str = retBytes.getString(0);
                assertEquals("foobar", str);
            });
        }
    }

    @Test
    void test_DataProtocol_receive() {
        try (var arena = AllocatingCodiraArena.ofConfined()) {
            var bytes = arena.allocateFrom("hello");
            var dat = Data.init(bytes, bytes.byteSize(), arena);
            var result = MyCodiraLibrary.globalReceiveSomeDataProtocol(dat);
            assertEquals(6, result);
        }
    }
}
