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

package org.code.codekit.core.util;

public class StringUtils {
    public static String stripPrefix(String mangledName, String prefix) {
        if (mangledName.startsWith(prefix)) {
            return mangledName.substring(prefix.length());
        }
        return mangledName;
    }

    public static String stripSuffix(String mangledName, String suffix) {
        if (mangledName.endsWith(suffix)) {
            return mangledName.substring(0, mangledName.length() - suffix.length());
        }
        return mangledName;
    }

    public static String hexString(long number) {
        return String.format("0x%02x", number);
    }

}
