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

import java.util.Optional;
import java.util.OptionalLong;
import java.util.OptionalInt;
import java.util.OptionalDouble;

@ThreadSafe
public class ThreadSafeHelperClass {
    public ThreadSafeHelperClass() { }

    public Optional<String> text = Optional.of("");

    public final OptionalDouble val = OptionalDouble.of(2);

    public String getValue(Optional<String> name) {
        return name.orElse("");
    }

    public Optional<String> getText() {
        return text;
    }

    public OptionalLong from(OptionalInt value) {
        return OptionalLong.of(value.getAsInt());
    }
}
