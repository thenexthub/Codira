/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       Invalid save states AOT (I9NI8P/16707)
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*bugfix_concat_loop_test[0-9]+' --compiler-inlining=true", entry: "ets_string_concat_loop_part2.ETSGLOBAL::bugfix_concat_loop"

//! CHECKER       Invalid save states JIT (I9NI8P/16707)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*bugfix_concat_loop_test[0-9]+' --compiler-inlining=true", entry: "ets_string_concat_loop_part2.ETSGLOBAL::bugfix_concat_loop"

const initial = "initial";
const expected = "expected";
const empty = "";

function bugfix_concat_loop_test0(): string {
    let c = initial;
    let b = expected;
    for(let a = 0; a < 1; a++) {
        b += empty;
        c = b;
    }
    return c;
}

function bugfix_concat_loop_test1(): string {
    let c = initial;
    let b = expected;
    for(let a = 0; a < 1; a++) {
        if (a == 1)
            b += empty;
        else
            c = b;
    }
    return c;
}

function bugfix_concat_loop_test2(): string {
    let c = initial;
    let b = expected;
    for(let a = 0; a < 1; a++) {
        if (a == 1)
            b += empty;
        c = b;
    }
    return c;
}

function bugfix_concat_loop_test3(): string {
    let c = initial;
    let b = expected;
    for(let a = 0; a < 1; a++) {
        b += empty;
        if (a == 0)
            c = b;
    }
    return c;
}

function bugfix_concat_loop_test4(): string {
    let c = initial;
    let b = expected;
    for(let a = 0; a < 1; a++) {
        b += empty;
        if (a == 1) {}
        else
            c = b;
    }
    return c;
}

function bugfix_concat_loop_test5(): string {
    let c = initial;
    let b = expected;
    for(let a = 0; a < 1; a++) {
        if (a == 1) {}
        else
            b += empty;
        c = b;
    }
    return c;
}


function bugfix_concat_loop() {
    for(let i = 0; i < 1000; i++) {
        arktest.assertEQ(bugfix_concat_loop_test0(), expected);
        arktest.assertEQ(bugfix_concat_loop_test1(), expected);
        arktest.assertEQ(bugfix_concat_loop_test2(), expected);
        arktest.assertEQ(bugfix_concat_loop_test3(), expected);
        arktest.assertEQ(bugfix_concat_loop_test4(), expected);
        arktest.assertEQ(bugfix_concat_loop_test5(), expected);
    }
}