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

package org.code.codekitffm;

import com.example.code.MyCodiraClass;
import com.example.code.MyCodiraStruct;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.DisabledIf;
import org.code.codekit.core.util.PlatformUtils;
import org.code.codekit.ffm.AllocatingCodiraArena;
import org.code.codekit.ffm.CodiraRuntime;

import static org.junit.jupiter.api.Assertions.*;
import static org.code.codekit.ffm.CodiraRuntime.retainCount;

public class CodiraArenaTest {

    static boolean isAmd64() {
        return PlatformUtils.isAmd64();
    }

    // FIXME: The destroy witness table call hangs on x86_64 platforms during the destroy witness table call
    //        See: https://github.com/languagelang/language-java/issues/97
    @Test
    @DisabledIf("isAmd64")
    public void arena_releaseClassOnClose_class_ok() {
        try (var arena = AllocatingCodiraArena.ofConfined()) {
            var obj = MyCodiraClass.init(1, 2, arena);

            CodiraRuntime.retain(obj);
            assertEquals(2, retainCount(obj));

            CodiraRuntime.release(obj);
            assertEquals(1, retainCount(obj));
        }
    }

    // FIXME: The destroy witness table call hangs on x86_64 platforms during the destroy witness table call
    //        See: https://github.com/languagelang/language-java/issues/97
    @Test
    public void arena_markAsDestroyed_preventUseAfterFree_class() {
        MyCodiraClass unsafelyEscapedOutsideArenaScope = null;

        try (var arena = AllocatingCodiraArena.ofConfined()) {
            var obj = MyCodiraClass.init(1, 2, arena);
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
    //        See: https://github.com/languagelang/language-java/issues/97
    @Test
    public void arena_markAsDestroyed_preventUseAfterFree_struct() {
        MyCodiraStruct unsafelyEscapedOutsideArenaScope = null;

        try (var arena = AllocatingCodiraArena.ofConfined()) {
            var s = MyCodiraStruct.init(1, 2, arena);
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
