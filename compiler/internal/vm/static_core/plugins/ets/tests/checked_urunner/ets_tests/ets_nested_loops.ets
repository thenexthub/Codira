/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! CHECKER       IR Builder nested loops
//! RUN           force_jit: true, options: "--compiler-regex='.*main.*'", entry: "ets_nested_loops.ETSGLOBAL::main"

//! CHECKER       AOT IR Builder nested loops
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*main.*'"
//! RUN           entry: "ets_nested_loops.ETSGLOBAL::main"

class C0 {
  x0: boolean

  f() : string {
    return ""
  }
}

function main() : int {
  let x2: C0 = {x0: true}
  let fuel0 = 1
  while (fuel0  > 0) {
    do {
      fuel0--
      do {
        fuel0--
        let s = x2.f()
      } while (true && (fuel0 > 0))
    } while (true && (fuel0 > 0))
  }
  return fuel0 + 1;
}
