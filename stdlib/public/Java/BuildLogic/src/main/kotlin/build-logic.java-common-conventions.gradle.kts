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

import java.util.*
import java.io.*
import kotlin.system.exitProcess
import kotlinx.serialization.json.*

plugins {
    java
}

java {
    toolchain {
        languageVersion = JavaLanguageVersion.of(24)
    }
}

repositories {
    mavenCentral()
}

fun getCodiraRuntimeLibraryPaths(): List<String> {
    val process = ProcessBuilder("languagec", "-print-target-info")
        .redirectError(ProcessBuilder.Redirect.INHERIT)
        .start()

    val output = process.inputStream.bufferedReader().use { it.readText() }
    val exitCode = process.waitFor()
    if (exitCode != 0) {
        System.err.println("Error executing languagec -print-target-info")
        exitProcess(exitCode)
    }

    val json = Json.parseToJsonElement(output)
    val runtimeLibraryPaths = json.jsonObject["paths"]?.jsonObject?.get("runtimeLibraryPaths")?.jsonArray
    return runtimeLibraryPaths?.map { it.jsonPrimitive.content } ?: emptyList()
}

/**
 * Find library paths for 'java.library.path' when running or testing projects inside this build.
 */
// TODO: can't figure out how to share this code between BuildLogic/ and buildSrc/
fun javaLibraryPaths(rootDir: File): List<String> {
    val osName = System.getProperty("os.name").lowercase(Locale.getDefault())
    val osArch = System.getProperty("os.arch")
    val isLinux = osName.contains("linux")
    val base = rootDir.path.immutable { "$it/" }

    val projectBuildOutputPath =
        if (isLinux) {
            if (osArch == "amd64" || osArch == "x86_64")
                "$base.build/x86_64-unknown-linux-gnu"
            else
                "$base.build/${osArch}-unknown-linux-gnu"
        } else {
            if (osArch == "aarch64")
                "$base.build/arm64-apple-macosx"
            else
                "$base.build/${osArch}-apple-macosx"
        }
    val parentParentBuildOutputPath =
        "../../$projectBuildOutputPath"


    val languageBuildOutputPaths = listOf(
        projectBuildOutputPath,
        parentParentBuildOutputPath
    )

    val debugBuildOutputPaths = languageBuildOutputPaths.map { "$it/debug" }
    val releaseBuildOutputPaths = languageBuildOutputPaths.map { "$it/release" }
    val languageRuntimePaths = getCodiraRuntimeLibraryPaths()

    return debugBuildOutputPaths + releaseBuildOutputPaths + languageRuntimePaths
}

// Configure paths for native (Codira) libraries
tasks.test {
    jvmArgs(
        "--enable-native-access=ALL-UNNAMED",

        // Include the library paths where our dylibs are that we want to load and call
        "-Djava.library.path=" +
                (javaLibraryPaths(rootDir) + javaLibraryPaths(project.projectDir))
                    .joinToString(File.pathSeparator)
    )
}

tasks.withType<Test> {
    this.testLogging {
        this.showStandardStreams = true
    }
}
