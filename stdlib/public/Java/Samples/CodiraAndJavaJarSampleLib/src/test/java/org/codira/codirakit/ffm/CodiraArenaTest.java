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

package org.code.codekit.ffm;

import com.example.code.MyCodiraClass;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.DisabledIf;
import org.code.codekit.core.util.PlatformUtils;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.code.codekit.ffm.CodiraRuntime.*;

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

            retain(obj);
            assertEquals(2, retainCount(obj));

            release(obj);
            assertEquals(1, retainCount(obj));
        }

        // TODO: should we zero out the $memorySegment perhaps?
    }
}
