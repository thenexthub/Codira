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

public class PlatformUtils {
    public static boolean isLinux() {
        return System.getProperty("os.name").toLowerCase().contains("linux");
    }

    public static boolean isMacOS() {
        return System.getProperty("os.name").toLowerCase().contains("mac");
    }

    public static boolean isWindows() {
        return System.getProperty("os.name").toLowerCase().contains("windows");
    }

    public static boolean isAarch64() {
        return System.getProperty("os.arch").equals("aarm64");
    }

    public static boolean isAmd64() {
        String arch = System.getProperty("os.arch");
        return arch.equals("amd64") || arch.equals("x86_64");
    }

    public static String dynamicLibraryName(String base) {
        if (isLinux()) {
            return "lib" + uppercaseFistLetter(base) + ".so";
        } else {
            return "lib" + uppercaseFistLetter(base) + ".dylib";
        }
    }

    static String uppercaseFistLetter(String base) {
        if (base == null || base.isEmpty()) {
            return base;
        }
        return base.substring(0, 1).toUpperCase() + base.substring(1);
    }
}
