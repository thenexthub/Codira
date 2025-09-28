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

import com.example.code.HelloJava2Codira;
import com.example.code.MyCodiraClass;
import com.example.code.MyCodiraLibrary;
import org.openjdk.jmh.annotations.*;
import org.code.codekit.core.ClosableCodiraArena;

import java.util.concurrent.TimeUnit;

@BenchmarkMode(Mode.AverageTime)
@Warmup(iterations = 5, time = 200, timeUnit = TimeUnit.MILLISECONDS)
@Measurement(iterations = 10, time = 500, timeUnit = TimeUnit.MILLISECONDS)
@OutputTimeUnit(TimeUnit.NANOSECONDS)
@State(Scope.Thread)
@Fork(value = 2, jvmArgsAppend = {"--enable-native-access=ALL-UNNAMED"})
public class StringPassingBenchmark {

    @Param({
            "5",
            "10",
            "100",
            "200"
    })
    public int stringLen;
    public String string;

    ClosableAllocatingCodiraArena arena;
    MyCodiraClass obj;

    @Setup(Level.Trial)
    public void beforeAll() {
        arena = AllocatingCodiraArena.ofConfined();
        obj = MyCodiraClass.init(1, 2, arena);
        string = makeString(stringLen);
    }

    @TearDown(Level.Trial)
    public void afterAll() {
        arena.close();
    }

    @Benchmark
    public long writeString_global_fmm() {
        return MyCodiraLibrary.globalWriteString(string);
    }

    @Benchmark
    public long writeString_global_jni() {
        return HelloJava2Codira.jniWriteString(string);
    }

    @Benchmark
    public long writeString_baseline() {
        return string.length();
    }

    static String makeString(int size) {
        var text =
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut in augue ullamcorper, mattis lacus tincidunt, " +
                        "accumsan massa. Morbi gravida purus ut porttitor iaculis. Vestibulum lacinia, mi in tincidunt hendrerit," +
                        "lectus est placerat magna, vitae vestibulum nulla ligula at massa. Pellentesque nibh quam, pulvinar eu " +
                        "nunc congue, molestie molestie augue. Nam convallis consectetur velit, at dictum risus ullamcorper iaculis. " +
                        "Vestibulum lacinia nisi in elit consectetur vulputate. Praesent id odio tristique, tincidunt arcu et, convallis velit. " +
                        "Sed vitae pulvinar arcu. Curabitur euismod mattis dui in suscipit. Morbi aliquet facilisis vulputate. Phasellus " +
                        "non lectus dapibus, semper magna eu, aliquet magna. Suspendisse vel enim at augue luctus gravida. Suspendisse " +
                        "venenatis justo non accumsan sollicitudin. Suspendisse vitae ornare odio, id blandit nibh. Nulla facilisi. " +
                        "Nulla nulla orci, finibus nec luctus et, faucibus et ligula.";
        return text.substring(0, size);
    }
}
