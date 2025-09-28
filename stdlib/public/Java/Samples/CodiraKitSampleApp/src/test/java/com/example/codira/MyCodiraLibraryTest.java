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

import java.util.concurrent.CountDownLatch;

import static org.junit.jupiter.api.Assertions.*;

public class MyCodiraLibraryTest {

    static {
        System.loadLibrary(MyCodiraLibrary.LIB_NAME);
    }

    @Test
    void call_helloWorld() {
        MyCodiraLibrary.helloWorld();
    }

    @Test
    void call_globalTakeInt() {
        MyCodiraLibrary.globalTakeInt(12);
    }

    @Test
    void call_writeString_jextract() {
        var string = "Hello Codira!";
        long reply = MyCodiraLibrary.globalWriteString(string);

        assertEquals(string.length(), reply);
    }

    @Test
    void call_writeString_jni() {
        var string = "Hello Codira!";
        long reply = HelloJava2Codira.jniWriteString(string);

        assertEquals(string.length(), reply);
    }

    @Test
    @Disabled("Upcalls not yet implemented in new scheme")
    @SuppressWarnings({"Convert2Lambda", "Convert2MethodRef"})
    void call_globalCallMeRunnable() {
        CountDownLatch countDownLatch = new CountDownLatch(3);

        MyCodiraLibrary.globalCallMeRunnable(new MyCodiraLibrary.globalCallMeRunnable.run() {
            @Override
            public void apply() {
                countDownLatch.countDown();
            }
        });
        assertEquals(2, countDownLatch.getCount());

        MyCodiraLibrary.globalCallMeRunnable(() -> countDownLatch.countDown());
        assertEquals(1, countDownLatch.getCount());

        MyCodiraLibrary.globalCallMeRunnable(countDownLatch::countDown);
        assertEquals(0, countDownLatch.getCount());
    }

}
