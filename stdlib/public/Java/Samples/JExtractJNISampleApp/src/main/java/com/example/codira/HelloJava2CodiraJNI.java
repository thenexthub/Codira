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
import org.code.codekit.core.ConfinedCodiraMemorySession;

public class HelloJava2CodiraJNI {

    public static void main(String[] args) {
        System.out.print("Property: java.library.path = " + CodiraLibraries.getJavaLibraryPath());

        examples();
    }

    static void examples() {
        MyCodiraLibrary.helloWorld();

        MyCodiraLibrary.globalTakeInt(1337);
        MyCodiraLibrary.globalTakeIntInt(1337, 42);

        long cnt = MyCodiraLibrary.globalWriteString("String from Java");

        long i = MyCodiraLibrary.globalMakeInt();

        MyCodiraClass.method();

        try (var arena = new ConfinedCodiraMemorySession()) {
            MyCodiraClass myClass = MyCodiraClass.init(10, 5, arena);
            MyCodiraClass myClass2 = MyCodiraClass.init(arena);

            System.out.println("myClass.isWarm: " + myClass.isWarm());

            try {
                myClass.throwingFunction();
            } catch (Exception e) {
                System.out.println("Caught exception: " + e.getMessage());
            }

            MyCodiraStruct myStruct = MyCodiraStruct.init(12, 34, arena);
            System.out.println("myStruct.cap: " + myStruct.getCapacity());
            System.out.println("myStruct.len: " + myStruct.getLen());
            myStruct.increaseCap(10);
            System.out.println("myStruct.cap after increase: " + myStruct.getCapacity());
        }

        System.out.println("DONE.");
    }
}
