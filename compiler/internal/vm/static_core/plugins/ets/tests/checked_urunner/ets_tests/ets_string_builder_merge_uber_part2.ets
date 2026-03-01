/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

/*---
  files: [ets_string_builder_merge_uber_export.ets]
---*/

// Source file must be last in the compile list
//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false", inputs: "ets_string_builder_merge_uber_export.ets", output: "ets_string_builder_merge_uber_part2_ets_string_builder_merge_uber_export.ets.abc"
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       JIT IR Builder, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--panda-files=../ets_string_builder_merge_uber_part2_ets_string_builder_merge_uber_export.ets.abc --compiler-regex='.*chain_concat[0-9]+.*' --compiler-inlining=false", entry: "ets_string_builder_merge_uber_part2.ETSGLOBAL::main"

//! CHECKER       JIT IR Builder, check String Builders merging (w/o optimizations)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--panda-files=../ets_string_builder_merge_uber_part2_ets_string_builder_merge_uber_export.ets.abc --compiler-regex='.*chain_concat[0-9]+.*' --compiler-inlining=false --compiler-simplify-string-builder=false", entry: "ets_string_builder_merge_uber_part2.ETSGLOBAL::main"

import { uber_chain_concat4 } from "./ets_string_builder_merge_uber_export.ets"

const uber_golden_FFTF = "_aaaa_bababa_aaaa_cbcbcb_bbbbb_aaaa_cbcbcb_dcdcdc_ccccc_dbdb_dcdcdc_aaaa_cbcbcb_dcdcdc_ccccc_acac_";
const uber_golden_FFTT = "_aaaa_bababa_aaaa_cbcbcb_bbbbb_aaaa_cbcbcb_dcdcdc_ccccc_dbdb_dcdcdc_aaaa_cbcbcb_dcdcdc_ccccc_acac_";

function main() {
    arktest.assertEQ(uber_chain_concat4(false, "a", 4, "b", 3, "c", 5, "d", 2, false, false, true, false) , uber_golden_FFTF, "Wrong result at uber_golden_FFTF");
    arktest.assertEQ(uber_chain_concat4(false, "a", 4, "b", 3, "c", 5, "d", 2, false, false, true, true)  , uber_golden_FFTT, "Wrong result at uber_golden_FFTT");
}
