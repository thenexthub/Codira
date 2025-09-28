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

import java.util.concurrent.TimeUnit;

import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.BenchmarkMode;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.Mode;
import org.openjdk.jmh.annotations.OutputTimeUnit;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.infra.Blackhole;

import com.example.code.MyCodiraClass;

@SuppressWarnings("unused")
public class JavaToCodiraBenchmark {

    @State(Scope.Benchmark)
    public static class BenchmarkState {
        ClosableCodiraArena arena;
        MyCodiraClass obj;

        @Setup(Level.Trial)
        public void beforeALl() {
            System.loadLibrary("languageCore");
            System.loadLibrary("ExampleCodiraLibrary");

            // Tune down debug statements so they don't fill up stdout
            System.setProperty("jextract.trace.downcalls", "false");

            arena = CodiraArena.ofConfined();
            obj = MyCodiraClass.init(1, 2, arena);
        }

        @TearDown(Level.Trial)
        public void afterAll() {
            arena.close();
        }
    }

    @Benchmark @BenchmarkMode(Mode.AverageTime) @OutputTimeUnit(TimeUnit.NANOSECONDS)
    public void simpleCodiraApiCall(BenchmarkState state, Blackhole blackhole) {
        blackhole.consume(state.obj.makeRandomIntMethod());
    }
}
