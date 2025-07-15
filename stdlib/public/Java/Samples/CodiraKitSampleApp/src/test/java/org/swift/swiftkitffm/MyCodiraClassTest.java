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

package org.swift.swiftkitffm;

import static org.junit.jupiter.api.Assertions.assertEquals;

import org.junit.jupiter.api.Test;

import com.example.swift.MySwiftClass;
import org.swift.swiftkit.ffm.AllocatingSwiftArena;
import org.swift.swiftkit.ffm.SwiftRuntime;

public class MySwiftClassTest {

    @Test
    void call_retain_retainCount_release() {
        var arena = AllocatingSwiftArena.ofConfined();
        var obj = MySwiftClass.init(1, 2, arena);

        assertEquals(1, SwiftRuntime.retainCount(obj));
        // TODO: test directly on SwiftHeapObject inheriting obj

        SwiftRuntime.retain(obj);
        assertEquals(2, SwiftRuntime.retainCount(obj));

        SwiftRuntime.release(obj);
        assertEquals(1, SwiftRuntime.retainCount(obj));
    }
}
