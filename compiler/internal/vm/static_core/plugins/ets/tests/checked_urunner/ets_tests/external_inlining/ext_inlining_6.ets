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
  files: [ext_funcs_4.ets, ext_funcs_4_1.ets, ext_funcs_5.ets]
---*/

// Source file must be last in the compile list
//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            inputs: "ext_funcs_4.ets", output: "ext_inlining_6_ext_funcs_4.ets.abc"
//! RUN_FRONTEND            inputs: "ext_funcs_4_1.ets", output: "ext_inlining_6_ext_funcs_4_1.ets.abc"
//! RUN_FRONTEND            inputs: "ext_funcs_5.ets", output: "ext_inlining_6_ext_funcs_5.ets.abc"
//! RUN_FRONTEND            options: "--opt-level=2 --ets-strings-concat=false"


import { top } from './ext_funcs_5.ets';
import { top2 } from './ext_funcs_5.ets';

//! CHECKER    Check external inlining with 3 files
//! RUN_PAOC   options: "--panda-files=../ext_inlining_6_ext_funcs_5.ets.abc:../ext_inlining_6_ext_funcs_4.ets.abc:../ext_inlining_6_ext_funcs_4_1.ets.abc", entry: "ext_inlining_6.ETSGLOBAL::main1" # CC-OFF(G.FMT.02) special comment for testin
//! EVENT      /Inline,ext_funcs_5.ETSGLOBAL::top,ext_funcs_4_1.ETSGLOBAL::compute,.*,STATIC,SUCCESS/
//! EVENT      /Inline,ext_inlining_6.ETSGLOBAL::main1,ext_funcs_5.ETSGLOBAL::top,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../ext_inlining_6_ext_funcs_5.ets.abc:../ext_inlining_6_ext_funcs_4.ets.abc:../ext_inlining_6_ext_funcs_4_1.ets.abc", entry: "ext_inlining_6.ETSGLOBAL::main1" # CC-OFF(G.FMT.02) special comment for testing
//! METHOD     "ext_inlining_6.ETSGLOBAL::main1"
//! PASS_BEFORE "Inline"
//! INST       /CallStatic.*ext_funcs_5\.ETSGLOBAL::top.*\[file=\d+\]/

function main1(): int {
    let total = top(3); // inlining: main → top → compute
    if (total == 8) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER    Check external inlining with 3 files 2
//! RUN_PAOC   options: "--panda-files=../ext_inlining_6_ext_funcs_5.ets.abc:../ext_inlining_6_ext_funcs_4.ets.abc:../ext_inlining_6_ext_funcs_4_1.ets.abc", entry: "ext_inlining_6.ETSGLOBAL::main2" # CC-OFF(G.FMT.02) special comment for testin
//! EVENT      /Inline,ext_funcs_5.ETSGLOBAL::top2,ext_funcs_4_1.ETSGLOBAL::compute2,.*,STATIC,SUCCESS/
//! EVENT      /Inline,ext_inlining_6.ETSGLOBAL::main2,ext_funcs_5.ETSGLOBAL::top2,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../ext_inlining_6_ext_funcs_5.ets.abc:../ext_inlining_6_ext_funcs_4.ets.abc:../ext_inlining_6_ext_funcs_4_1.ets.abc", entry: "ext_inlining_6.ETSGLOBAL::main2" # CC-OFF(G.FMT.02) special comment for testing
//! METHOD     "ext_inlining_6.ETSGLOBAL::main2"
//! PASS_BEFORE "Inline"
//! INST       /CallStatic.*ext_funcs_5\.ETSGLOBAL::top2.*\[file=\d+\]/

function main2(): int {
    let total = top2(3); // Expect: 0²+1 + 1²+1 + 2²+1 = 1 + 2 + 5 = 8
    if (total == 8) {
        return 0;
    } else {
        return 1;
    }
}
