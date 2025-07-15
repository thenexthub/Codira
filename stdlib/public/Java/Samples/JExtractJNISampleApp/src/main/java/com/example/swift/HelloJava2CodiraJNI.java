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

// Import swift-extract generated sources

// Import javakit/swiftkit support libraries

import org.swift.swiftkit.core.SwiftLibraries;
import org.swift.swiftkit.core.ConfinedSwiftMemorySession;

public class HelloJava2SwiftJNI {

    public static void main(String[] args) {
        System.out.print("Property: java.library.path = " + SwiftLibraries.getJavaLibraryPath());

        examples();
    }

    static void examples() {
        MySwiftLibrary.helloWorld();

        MySwiftLibrary.globalTakeInt(1337);
        MySwiftLibrary.globalTakeIntInt(1337, 42);

        long cnt = MySwiftLibrary.globalWriteString("String from Java");

        long i = MySwiftLibrary.globalMakeInt();

        MySwiftClass.method();

        try (var arena = new ConfinedSwiftMemorySession()) {
            MySwiftClass myClass = MySwiftClass.init(10, 5, arena);
            MySwiftClass myClass2 = MySwiftClass.init(arena);

            System.out.println("myClass.isWarm: " + myClass.isWarm());

            try {
                myClass.throwingFunction();
            } catch (Exception e) {
                System.out.println("Caught exception: " + e.getMessage());
            }

            MySwiftStruct myStruct = MySwiftStruct.init(12, 34, arena);
            System.out.println("myStruct.cap: " + myStruct.getCapacity());
            System.out.println("myStruct.len: " + myStruct.getLen());
            myStruct.increaseCap(10);
            System.out.println("myStruct.cap after increase: " + myStruct.getCapacity());
        }

        System.out.println("DONE.");
    }
}
