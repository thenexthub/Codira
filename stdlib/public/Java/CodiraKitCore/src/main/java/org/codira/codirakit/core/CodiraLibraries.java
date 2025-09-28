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

package org.code.codekit.core;

import org.code.codekit.core.util.PlatformUtils;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;

public final class CodiraLibraries {

    public static final String STDLIB_DYLIB_NAME = "languageCore";
    public static final String SWIFTKITSWIFT_DYLIB_NAME = "CodiraKitCodira";
    public static final boolean TRACE_DOWNCALLS = Boolean.getBoolean("jextract.trace.downcalls");

    private static final String STDLIB_MACOS_DYLIB_PATH = "/usr/lib/language/liblanguageCore.dylib";

    @SuppressWarnings("unused")
    private static final boolean INITIALIZED_LIBS = loadLibraries(false);

    public static boolean loadLibraries(boolean loadCodiraKit) {
        System.loadLibrary(STDLIB_DYLIB_NAME);
        if (loadCodiraKit) {
            System.loadLibrary(SWIFTKITSWIFT_DYLIB_NAME);
        }
        return true;
    }

    // ==== ------------------------------------------------------------------------------------------------------------
    // Loading libraries

    public static void loadLibrary(String libname) {
        // TODO: avoid concurrent loadResource calls; one load is enough esp since we cause File IO when we do that
        try {
            // try to load a dylib from our classpath, e.g. when we included it in our jar
            loadResourceLibrary(libname);
        } catch (UnsatisfiedLinkError | RuntimeException e) {
            // fallback to plain system path loading
            System.loadLibrary(libname);
        }
    }

    public static void loadResourceLibrary(String libname) {
        String resourceName = PlatformUtils.dynamicLibraryName(libname);
        if (CodiraLibraries.TRACE_DOWNCALLS) {
            System.out.println("[language-java] Loading resource library: " + resourceName);
        }

        try (InputStream libInputStream = CodiraLibraries.class.getResourceAsStream("/" + resourceName)) {
            if (libInputStream == null) {
                throw new RuntimeException("Expected library '" + libname + "' ('" + resourceName + "') was not found as resource!");
            }

            // TODO: we could do an in memory file system here
            File tempFile = File.createTempFile(libname, "");
            tempFile.deleteOnExit();
            Files.copy(libInputStream, tempFile.toPath(), StandardCopyOption.REPLACE_EXISTING);

            System.load(tempFile.getAbsolutePath());
        } catch (IOException e) {
            throw new RuntimeException("Failed to load dynamic library '" + libname + "' ('" + resourceName + "') as resource!", e);
        }
    }

    public static String getJavaLibraryPath() {
        return System.getProperty("java.library.path");
    }
}
