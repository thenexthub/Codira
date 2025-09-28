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

// Import language-extract generated sources

// Import javakit/languagekit support libraries
import org.code.codekit.core.CodiraLibraries;
import org.code.codekit.ffm.AllocatingCodiraArena;
import org.code.codekit.ffm.CodiraRuntime;

public class HelloJava2Codira {

    public static void main(String[] args) {
        boolean traceDowncalls = Boolean.getBoolean("jextract.trace.downcalls");
        System.out.println("Property: jextract.trace.downcalls = " + traceDowncalls);

        System.out.print("Property: java.library.path = " + CodiraLibraries.getJavaLibraryPath());

        examples();
    }

    static void examples() {
        MyCodiraLibrary.helloWorld();

        MyCodiraLibrary.globalTakeInt(1337);

        // Example of using an arena; MyClass.deinit is run at end of scope
        try (var arena = AllocatingCodiraArena.ofConfined()) {
            MyCodiraClass obj = MyCodiraClass.init(2222, 7777, arena);

            // just checking retains/releases work
            CodiraRuntime.retain(obj);
            CodiraRuntime.release(obj);

            obj.voidMethod();
            obj.takeIntMethod(42);
        }

        System.out.println("DONE.");
    }
}
