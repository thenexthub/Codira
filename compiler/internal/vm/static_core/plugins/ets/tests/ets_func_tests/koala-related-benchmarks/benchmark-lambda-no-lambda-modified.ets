/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

class Context {
  compute(exec: () => void) {
    totalCompute++
    if (true) {
      exec()
    }
  }
  computeNoLambda() {
    totalCompute++
    return true
  }
}
const ctx = new Context()
let totalWorkload = 0
let totalCompute = 0

function benchLambda() {
  ctx.compute(() => {
    totalWorkload++
  })
}
function benchNoLambda() {
  if (ctx.computeNoLambda()) {
    totalWorkload++
  }
}

let start: number = 0
const N = 10000

function Benchmark() {
  console.log("Starting benchmarks, N="+N)
  totalWorkload = 0
  totalCompute = 0
  console.log("Starting lambda benchmark")
  start = Date.now()
  for (let i=0; i<N; i++) {
    benchLambda()
  }
  console.log("lambda time: " + (Date.now() - start))

  totalWorkload = 0
  totalCompute = 0
  console.log("Starting no-lambda benchmark")
  start = Date.now()
  for (let i=0; i<N; i++) {
    benchNoLambda()
  }
  console.log("no lambda time: " + (Date.now() - start))
}

function main() : int {
    Benchmark();
    return 0;
}


