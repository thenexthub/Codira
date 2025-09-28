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

import static org.junit.jupiter.api.Assertions.assertEquals;

import org.junit.jupiter.api.Test;

import com.example.code.MyCodiraClass;
import org.code.codekit.ffm.AllocatingCodiraArena;
import org.code.codekit.ffm.CodiraRuntime;

public class MyCodiraClassTest {

    @Test
    void call_retain_retainCount_release() {
        var arena = AllocatingCodiraArena.ofConfined();
        var obj = MyCodiraClass.init(1, 2, arena);

        assertEquals(1, CodiraRuntime.retainCount(obj));
        // TODO: test directly on CodiraHeapObject inheriting obj

        CodiraRuntime.retain(obj);
        assertEquals(2, CodiraRuntime.retainCount(obj));

        CodiraRuntime.release(obj);
        assertEquals(1, CodiraRuntime.retainCount(obj));
    }
}
