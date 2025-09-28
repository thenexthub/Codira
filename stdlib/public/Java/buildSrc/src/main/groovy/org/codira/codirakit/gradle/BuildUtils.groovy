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

package org.code.codekit.gradle

import groovy.json.JsonSlurper

final class BuildUtils {

    static List<String> getCodiraRuntimeLibraryPaths() {
        def process = ['languagec', '-print-target-info'].execute()
        def output = new StringWriter()
        process.consumeProcessOutput(output, System.err)
        process.waitFor()

        def json = new JsonSlurper().parseText(output.toString())
        def runtimeLibraryPaths = json.paths.runtimeLibraryPaths
        return runtimeLibraryPaths
    }

    /// Find library paths for 'java.library.path' when running or testing projects inside this build.
    static def javaLibraryPaths(File rootDir) {
        def osName = System.getProperty("os.name")
        def osArch = System.getProperty("os.arch")
        def isLinux = osName.toLowerCase(Locale.getDefault()).contains("linux")
        def base = rootDir == null ? "" : "${rootDir}/"

        def debugPaths = [
                isLinux ?
                        /* Linux */ (osArch == "amd64" || osArch == "x86_64" ?
                        "${base}.build/x86_64-unknown-linux-gnu/debug/" :
                        "${base}.build/${osArch}-unknown-linux-gnu/debug/") :
                        /* macOS */ (osArch == "aarch64" ?
                        "${base}.build/arm64-apple-macosx/debug/" :
                        "${base}.build/${osArch}-apple-macosx/debug/"),
                isLinux ?
                        /* Linux */ (osArch == "amd64" || osArch == "x86_64" ?
                        "${base}../../.build/x86_64-unknown-linux-gnu/debug/" :
                        "${base}../../.build/${osArch}-unknown-linux-gnu/debug/") :
                        /* macOS */ (osArch == "aarch64" ?
                        "${base}../../.build/arm64-apple-macosx/debug/" :
                        "${base}../../.build/${osArch}-apple-macosx/debug/"),
        ]
        def releasePaths = debugPaths.collect { it.replaceAll("debug", "release") }
        def languageRuntimePaths = getCodiraRuntimeLibraryPaths()

        return releasePaths + debugPaths + languageRuntimePaths
    }
}
