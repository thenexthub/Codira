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
  files: [ext_funcs_3.ets]
---*/

// Source file must be last in the compile list
//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            inputs: "ext_funcs_3.ets", output: "ext_inlining_4_ext_funcs_3.ets.abc"
//! RUN_FRONTEND            options: "--opt-level=2 --ets-strings-concat=false"


import { A } from './ext_funcs_3.ets';
import { risky } from './ext_funcs_3.ets';
import { maybe_throw } from './ext_funcs_3.ets';
import { sum_array } from './ext_funcs_3.ets';

//! CHECKER    Check deep external inlining
//! RUN_PAOC   options: "--panda-files=../ext_inlining_4_ext_funcs_3.ets.abc", entry: "ext_inlining_4.ETSGLOBAL::main0" # CC-OFF(G.FMT.02) special comment for testin
//! EVENT      /Inline,ext_funcs_3.ETSGLOBAL::B,ext_funcs_3.ETSGLOBAL::C,.*,STATIC,SUCCESS/
//! EVENT      /Inline,ext_funcs_3.ETSGLOBAL::A,ext_funcs_3.ETSGLOBAL::B,.*,STATIC,SUCCESS/
//! EVENT      /Inline,ext_inlining_4.ETSGLOBAL::main0,ext_funcs_3.ETSGLOBAL::A,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../ext_inlining_4_ext_funcs_3.ets.abc", entry: "ext_inlining_4.ETSGLOBAL::main0" # CC-OFF(G.FMT.02) special comment for testing
//! METHOD     "ext_inlining_4.ETSGLOBAL::main0"
//! PASS_BEFORE "Inline"
//! INST       /CallStatic.*ext_funcs_3\.ETSGLOBAL::A.*\[file=\d+\]/

function main0(): int { 
    if (A(5) == 6) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER    Check inline external method and inline internal method inside (no exception)
//! RUN_PAOC   options: "--panda-files=../ext_inlining_4_ext_funcs_3.ets.abc", entry: "ext_inlining_4.ETSGLOBAL::main1" # CC-OFF(G.FMT.02) special comment for testin
//! EVENT      /Inline,ext_funcs_3.ETSGLOBAL::risky,ext_funcs_3.ETSGLOBAL::maybe_throw,.*,STATIC,SUCCESS/
//! EVENT      /Inline,ext_inlining_4.ETSGLOBAL::main1,ext_funcs_3.ETSGLOBAL::risky,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../ext_inlining_4_ext_funcs_3.ets.abc", entry: "ext_inlining_4.ETSGLOBAL::main1" # CC-OFF(G.FMT.02) special comment for testing
//! METHOD     "ext_inlining_4.ETSGLOBAL::main1"
//! PASS_BEFORE "Inline"
//! INST       /CallStatic.*ext_funcs_3\.ETSGLOBAL::risky.*\[file=\d+\]/

function main1(): int {
    let a: int[] = [1, 2, 3];
    if (risky(a, 1) == 50) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER    Check inline external method and inline internal method inside (exception)
//! RUN_PAOC   options: "--panda-files=../ext_inlining_4_ext_funcs_3.ets.abc", entry: "ext_inlining_4.ETSGLOBAL::main2" # CC-OFF(G.FMT.02) special comment for testin
//! EVENT      /Inline,ext_funcs_3.ETSGLOBAL::risky,ext_funcs_3.ETSGLOBAL::maybe_throw,.*,STATIC,SUCCESS/
//! EVENT      /Inline,ext_inlining_4.ETSGLOBAL::main2,ext_funcs_3.ETSGLOBAL::risky,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../ext_inlining_4_ext_funcs_3.ets.abc", entry: "ext_inlining_4.ETSGLOBAL::main2" # CC-OFF(G.FMT.02) special comment for testin
//! EVENT      /Exception,ext_funcs_3.ETSGLOBAL::maybe_throw,.*,THROW/
//! EVENT      /Deoptimization,ext_inlining_4.ETSGLOBAL::main2,.*,IFRAME/
//! METHOD     "ext_inlining_4.ETSGLOBAL::main2"
//! PASS_BEFORE "Inline"
//! INST       /CallStatic.*ext_funcs_3\.ETSGLOBAL::risky.*\[file=\d+\]/

function main2(): int {
    let a: int[] = [1, 0, 3];
    try {
        risky(a, 1);
    } catch (e) {
        return 0;
    }
    return 1;
}

//! CHECKER    Check exception inside of external inlined method
//! RUN_PAOC   options: "--panda-files=../ext_inlining_4_ext_funcs_3.ets.abc", entry: "ext_inlining_4.ETSGLOBAL::main3" # CC-OFF(G.FMT.02) special comment for testin
//! EVENT      /Inline,ext_inlining_4.ETSGLOBAL::main3,ext_funcs_3.ETSGLOBAL::maybe_throw,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../ext_inlining_4_ext_funcs_3.ets.abc", entry: "ext_inlining_4.ETSGLOBAL::main3" # CC-OFF(G.FMT.02) special comment for testin
//! EVENT      /Exception,ext_funcs_3.ETSGLOBAL::maybe_throw,.*,THROW/
//! EVENT      /Deoptimization,ext_inlining_4.ETSGLOBAL::main3,.*,IFRAME/
//! METHOD     "ext_inlining_4.ETSGLOBAL::main3"
//! PASS_BEFORE "Inline"
//! INST       /CallStatic.*ext_funcs_3\.ETSGLOBAL::maybe_throw.*\[file=\d+\]/

function main3(): int {
    let total = 0;
    for (let i = 2; i >= 0; i--) {
        try {
            total += maybe_throw(i);
        } catch (e) {
            total += -1;
        }
    }
   if (total == 149) {
        return 0;
    } else {
        return 1;
    }
}

//! CHECKER    Check deoptimization inside of external inlined method
//! RUN_PAOC   options: "--panda-files=../ext_inlining_4_ext_funcs_3.ets.abc", entry: "ext_inlining_4.ETSGLOBAL::main4" # CC-OFF(G.FMT.02) special comment for testin
//! EVENT      /Inline,ext_inlining_4.ETSGLOBAL::main4,ext_funcs_3.ETSGLOBAL::sum_array,.*,STATIC,SUCCESS/
//! RUN        options: "--panda-files=../ext_inlining_4_ext_funcs_3.ets.abc", entry: "ext_inlining_4.ETSGLOBAL::main4" # CC-OFF(G.FMT.02) special comment for testin
//! EVENT      /DeoptimizationReason,ext_funcs_3.ETSGLOBAL::sum_array,BOUNDS_CHECK_WITH_DEOPT/
//! EVENT      /Deoptimization,ext_inlining_4.ETSGLOBAL::main4,.*,IFRAME/
//! METHOD     "ext_inlining_4.ETSGLOBAL::main4"
//! PASS_BEFORE "Inline"
//! INST       /CallStatic.*ext_funcs_3\.ETSGLOBAL::sum_array.*\[file=\d+\]/

function main4(): int {
    let arr: int[] = [1, 2, 3];
    let total = 0;

    try {
        total = sum_array(arr);
    } catch (e) {
        total = -1;
    }

    if (total == -1) {
        return 0;
    } else {
        return 1;
    }
}
