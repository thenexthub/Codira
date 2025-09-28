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
import com.example.code.MyCodiraLibrary;
import org.openjdk.jmh.annotations.*;

import com.example.code.MyCodiraClass;

import java.util.concurrent.TimeUnit;

@BenchmarkMode(Mode.AverageTime)
@Warmup(iterations = 5, time = 200, timeUnit = TimeUnit.MILLISECONDS)
@Measurement(iterations = 10, time = 500, timeUnit = TimeUnit.MILLISECONDS)
@OutputTimeUnit(TimeUnit.NANOSECONDS)
@Fork(value = 3, jvmArgsAppend = { "--enable-native-access=ALL-UNNAMED" })
public class JavaToCodiraBenchmark {

    @State(Scope.Benchmark)
    public static class BenchmarkState {
        ClosableAllocatingCodiraArena arena;
        MyCodiraClass obj;

        @Setup(Level.Trial)
        public void beforeAll() {
            arena = AllocatingCodiraArena.ofConfined();
            obj = MyCodiraClass.init(1, 2, arena);
        }

        @TearDown(Level.Trial)
        public void afterAll() {
            arena.close();
        }
    }

    @Benchmark
    public long jextract_getInt_ffm(BenchmarkState state) {
        return MyCodiraLibrary.globalMakeInt();
    }

    @Benchmark
    public long getInt_global_jni(BenchmarkState state) {
        return HelloJava2Codira.jniGetInt();
    }

    @Benchmark
    public long getInt_member_ffi(BenchmarkState state) {
        return state.obj.makeIntMethod();
    }

}
