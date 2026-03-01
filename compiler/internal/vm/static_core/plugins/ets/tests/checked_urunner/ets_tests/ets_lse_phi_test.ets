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

//! CHECKER       Check LSE with PHI in JIT
//! RUN           force_jit: true, options: "--compiler-regex='.*ETSGLOBAL::main'", entry: "ets_lse_phi_test.ETSGLOBAL::main"
//! METHOD        "ets_lse_phi_test.ETSGLOBAL::main"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /StoreObject .* ets_lse_phi_test.C0.x0/, 2

//! CHECKER       Check LSE with PHI in AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: ""
//! METHOD        "ets_lse_phi_test.ETSGLOBAL::main"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /StoreObject .* ets_lse_phi_test.C0.x0/, 2
//! RUN           entry: "ets_lse_phi_test.ETSGLOBAL::main"


class C0 {
  x0: boolean
}

function main() : void {
  let x1: C0 = {x0: true}
  while(x1.x0) {
    x1.x0 = x1.x0
    x1.x0 = false
  }
}