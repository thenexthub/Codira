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

        long cnt = MyCodiraLibrary.globalWriteString("String from Java");

        CodiraRuntime.trace("count = " + cnt);

        MyCodiraLibrary.globalCallMeRunnable(() -> {
            CodiraRuntime.trace("running runnable");
        });

        CodiraRuntime.trace("getGlobalBuffer().byteSize()=" + MyCodiraLibrary.getGlobalBuffer().byteSize());

        MyCodiraLibrary.withBuffer((buf) -> {
            CodiraRuntime.trace("withBuffer{$0.byteSize()}=" + buf.byteSize());
        });
        // Example of using an arena; MyClass.deinit is run at end of scope
        try (var arena = AllocatingCodiraArena.ofConfined()) {
            MyCodiraClass obj = MyCodiraClass.init(2222, 7777, arena);

            // just checking retains/releases work
            CodiraRuntime.trace("retainCount = " + CodiraRuntime.retainCount(obj));
            CodiraRuntime.retain(obj);
            CodiraRuntime.trace("retainCount = " + CodiraRuntime.retainCount(obj));
            CodiraRuntime.release(obj);
            CodiraRuntime.trace("retainCount = " + CodiraRuntime.retainCount(obj));

            obj.setCounter(12);
            CodiraRuntime.trace("obj.counter = " + obj.getCounter());

            obj.voidMethod();
            obj.takeIntMethod(42);

            MyCodiraClass otherObj = MyCodiraClass.factory(12, 42, arena);
            otherObj.voidMethod();

            MyCodiraStruct languageValue = MyCodiraStruct.init(2222, 1111, arena);
            CodiraRuntime.trace("languageValue.capacity = " + languageValue.getCapacity());
            languageValue.withCapLen((cap, len) -> {
                CodiraRuntime.trace("withCapLenCallback: cap=" + cap + ", len=" + len);
            });
        }

        // Example of using 'Data'.
        try (var arena = AllocatingCodiraArena.ofConfined()) {
            var origBytes = arena.allocateFrom("foobar");
            var origDat = Data.init(origBytes, origBytes.byteSize(), arena);
            CodiraRuntime.trace("origDat.count = " + origDat.getCount());
            
            var retDat = MyCodiraLibrary.globalReceiveReturnData(origDat, arena);
            retDat.withUnsafeBytes((retBytes) -> {
                var str = retBytes.getString(0);
                CodiraRuntime.trace("retStr=" + str);
            });
        }

        try (var arena = AllocatingCodiraArena.ofConfined()) {
            var bytes = arena.allocateFrom("hello");
            var dat = Data.init(bytes, bytes.byteSize(), arena);
            MyCodiraLibrary.globalReceiveSomeDataProtocol(dat);
        }


        System.out.println("DONE.");
    }

    public static native long jniWriteString(String str);

    public static native long jniGetInt();

}
