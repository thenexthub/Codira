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
import org.swift.swiftkit.ffm.AllocatingSwiftArena;
import org.swift.swiftkit.ffm.SwiftRuntime;

public class HelloJava2Swift {

    public static void main(String[] args) {
        boolean traceDowncalls = Boolean.getBoolean("jextract.trace.downcalls");
        System.out.println("Property: jextract.trace.downcalls = " + traceDowncalls);

        System.out.print("Property: java.library.path = " + SwiftLibraries.getJavaLibraryPath());

        examples();
    }

    static void examples() {
        MySwiftLibrary.helloWorld();

        MySwiftLibrary.globalTakeInt(1337);

        long cnt = MySwiftLibrary.globalWriteString("String from Java");

        SwiftRuntime.trace("count = " + cnt);

        MySwiftLibrary.globalCallMeRunnable(() -> {
            SwiftRuntime.trace("running runnable");
        });

        SwiftRuntime.trace("getGlobalBuffer().byteSize()=" + MySwiftLibrary.getGlobalBuffer().byteSize());

        MySwiftLibrary.withBuffer((buf) -> {
            SwiftRuntime.trace("withBuffer{$0.byteSize()}=" + buf.byteSize());
        });
        // Example of using an arena; MyClass.deinit is run at end of scope
        try (var arena = AllocatingSwiftArena.ofConfined()) {
            MySwiftClass obj = MySwiftClass.init(2222, 7777, arena);

            // just checking retains/releases work
            SwiftRuntime.trace("retainCount = " + SwiftRuntime.retainCount(obj));
            SwiftRuntime.retain(obj);
            SwiftRuntime.trace("retainCount = " + SwiftRuntime.retainCount(obj));
            SwiftRuntime.release(obj);
            SwiftRuntime.trace("retainCount = " + SwiftRuntime.retainCount(obj));

            obj.setCounter(12);
            SwiftRuntime.trace("obj.counter = " + obj.getCounter());

            obj.voidMethod();
            obj.takeIntMethod(42);

            MySwiftClass otherObj = MySwiftClass.factory(12, 42, arena);
            otherObj.voidMethod();

            MySwiftStruct swiftValue = MySwiftStruct.init(2222, 1111, arena);
            SwiftRuntime.trace("swiftValue.capacity = " + swiftValue.getCapacity());
            swiftValue.withCapLen((cap, len) -> {
                SwiftRuntime.trace("withCapLenCallback: cap=" + cap + ", len=" + len);
            });
        }

        // Example of using 'Data'.
        try (var arena = AllocatingSwiftArena.ofConfined()) {
            var origBytes = arena.allocateFrom("foobar");
            var origDat = Data.init(origBytes, origBytes.byteSize(), arena);
            SwiftRuntime.trace("origDat.count = " + origDat.getCount());
            
            var retDat = MySwiftLibrary.globalReceiveReturnData(origDat, arena);
            retDat.withUnsafeBytes((retBytes) -> {
                var str = retBytes.getString(0);
                SwiftRuntime.trace("retStr=" + str);
            });
        }

        try (var arena = AllocatingSwiftArena.ofConfined()) {
            var bytes = arena.allocateFrom("hello");
            var dat = Data.init(bytes, bytes.byteSize(), arena);
            MySwiftLibrary.globalReceiveSomeDataProtocol(dat);
        }


        System.out.println("DONE.");
    }

    public static native long jniWriteString(String str);

    public static native long jniGetInt();

}
