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

import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;
import org.code.codekit.core.CodiraLibraries;
import org.code.codekit.ffm.AllocatingCodiraArena;

import java.io.File;
import java.util.stream.Stream;

import static org.junit.jupiter.api.Assertions.*;

public class MyCodiraClassTest {

    void checkPaths(Throwable throwable) {
        var paths = CodiraLibraries.getJavaLibraryPath().split(":");
        for (var path : paths) {
            Stream.of(new File(path).listFiles())
                    .filter(file -> !file.isDirectory())
                    .forEach((file) -> {
                        System.out.println("  - " + file.getPath());
                    });
        }

        throw new RuntimeException(throwable);
    }

    @Test
    void test_MyCodiraClass_voidMethod() {
        try(var arena = AllocatingCodiraArena.ofConfined()) {
            MyCodiraClass o = MyCodiraClass.init(12, 42, arena);
            o.voidMethod();
        } catch (Throwable throwable) {
            checkPaths(throwable);
        }
    }

    @Test
    void test_MyCodiraClass_makeIntMethod() {
        try(var arena = AllocatingCodiraArena.ofConfined()) {
            MyCodiraClass o = MyCodiraClass.init(12, 42, arena);
            var got = o.makeIntMethod();
            assertEquals(12, got);
        }
    }

    @Test
    @Disabled // TODO: Need var mangled names in interfaces
    void test_MyCodiraClass_property_len() {
        try(var arena = AllocatingCodiraArena.ofConfined()) {
            MyCodiraClass o = MyCodiraClass.init(12, 42, arena);
            var got = o.getLen();
            assertEquals(12, got);
        }
    }

}
