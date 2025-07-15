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

import com.example.swift.MySwiftClass;
import com.example.swift.MySwiftStruct;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.DisabledIf;
import org.swift.swiftkit.core.util.PlatformUtils;
import org.swift.swiftkit.ffm.AllocatingSwiftArena;
import org.swift.swiftkit.ffm.SwiftRuntime;

import static org.junit.jupiter.api.Assertions.*;
import static org.swift.swiftkit.ffm.SwiftRuntime.retainCount;

public class SwiftArenaTest {

    static boolean isAmd64() {
        return PlatformUtils.isAmd64();
    }

    // FIXME: The destroy witness table call hangs on x86_64 platforms during the destroy witness table call
    //        See: https://github.com/swiftlang/swift-java/issues/97
    @Test
    @DisabledIf("isAmd64")
    public void arena_releaseClassOnClose_class_ok() {
        try (var arena = AllocatingSwiftArena.ofConfined()) {
            var obj = MySwiftClass.init(1, 2, arena);

            SwiftRuntime.retain(obj);
            assertEquals(2, retainCount(obj));

            SwiftRuntime.release(obj);
            assertEquals(1, retainCount(obj));
        }
    }

    // FIXME: The destroy witness table call hangs on x86_64 platforms during the destroy witness table call
    //        See: https://github.com/swiftlang/swift-java/issues/97
    @Test
    public void arena_markAsDestroyed_preventUseAfterFree_class() {
        MySwiftClass unsafelyEscapedOutsideArenaScope = null;

        try (var arena = AllocatingSwiftArena.ofConfined()) {
            var obj = MySwiftClass.init(1, 2, arena);
            unsafelyEscapedOutsideArenaScope = obj;
        }

        try {
            unsafelyEscapedOutsideArenaScope.echoIntMethod(1);
            fail("Expected exception to be thrown! Object was supposed to be dead.");
        } catch (IllegalStateException ex) {
            return;
        }
    }

    // FIXME: The destroy witness table call hangs on x86_64 platforms during the destroy witness table call
    //        See: https://github.com/swiftlang/swift-java/issues/97
    @Test
    public void arena_markAsDestroyed_preventUseAfterFree_struct() {
        MySwiftStruct unsafelyEscapedOutsideArenaScope = null;

        try (var arena = AllocatingSwiftArena.ofConfined()) {
            var s = MySwiftStruct.init(1, 2, arena);
            unsafelyEscapedOutsideArenaScope = s;
        }

        try {
            unsafelyEscapedOutsideArenaScope.echoIntMethod(1);
            fail("Expected exception to be thrown! Object was supposed to be dead.");
        } catch (IllegalStateException ex) {
            return;
        }
    }

    @Test
    public void arena_initializeWithCopy_struct() {

    }
}
