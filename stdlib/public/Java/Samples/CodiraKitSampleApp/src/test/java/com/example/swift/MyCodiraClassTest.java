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

import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;
import org.swift.swiftkit.core.SwiftLibraries;
import org.swift.swiftkit.ffm.AllocatingSwiftArena;

import java.io.File;
import java.util.stream.Stream;

import static org.junit.jupiter.api.Assertions.*;

public class MySwiftClassTest {

    void checkPaths(Throwable throwable) {
        var paths = SwiftLibraries.getJavaLibraryPath().split(":");
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
    void test_MySwiftClass_voidMethod() {
        try(var arena = AllocatingSwiftArena.ofConfined()) {
            MySwiftClass o = MySwiftClass.init(12, 42, arena);
            o.voidMethod();
        } catch (Throwable throwable) {
            checkPaths(throwable);
        }
    }

    @Test
    void test_MySwiftClass_makeIntMethod() {
        try(var arena = AllocatingSwiftArena.ofConfined()) {
            MySwiftClass o = MySwiftClass.init(12, 42, arena);
            var got = o.makeIntMethod();
            assertEquals(12, got);
        }
    }

    @Test
    @Disabled // TODO: Need var mangled names in interfaces
    void test_MySwiftClass_property_len() {
        try(var arena = AllocatingSwiftArena.ofConfined()) {
            MySwiftClass o = MySwiftClass.init(12, 42, arena);
            var got = o.getLen();
            assertEquals(12, got);
        }
    }

}
