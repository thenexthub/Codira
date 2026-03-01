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

//! CHECKER       Check loopunroll optimize correctly for shl shr ashr
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex=.*::test_.*", entry: "loop_unroll_shl_shr_ashr.ETSGLOBAL::main"
//! RUN           entry: "loop_unroll_shl_shr_ashr.ETSGLOBAL::main"
//!
//! METHOD       "loop_unroll_shl_shr_ashr.ETSGLOBAL::test_shl_neg"
//! PASS_BEFORE  "LoopUnroll"
//! INST_COUNT   "Shl", 1
//! PASS_AFTER   "LoopUnroll"
//! INST_COUNT   "Shl", 6
//!
//! METHOD       "loop_unroll_shl_shr_ashr.ETSGLOBAL::test_shl_overflow"
//! PASS_BEFORE  "LoopUnroll"
//! INST_COUNT   "Shl", 1
//! PASS_AFTER   "LoopUnroll"
//! INST_COUNT   "Shl", 6
//!
//! METHOD       "loop_unroll_shl_shr_ashr.ETSGLOBAL::test_shl_not_one"
//! PASS_BEFORE  "LoopUnroll"
//! INST_COUNT   "Shl", 1
//! PASS_AFTER   "LoopUnroll"
//! INST_COUNT   "Shl", 6
//!
//! METHOD       "loop_unroll_shl_shr_ashr.ETSGLOBAL::test_shl_not_overflow"
//! PASS_BEFORE  "LoopUnroll"
//! INST_COUNT   "Shl", 1
//! PASS_AFTER   "LoopUnroll"
//! INST_COUNT   "Shl", 8
//!
//! METHOD       "loop_unroll_shl_shr_ashr.ETSGLOBAL::test_shr"
//! PASS_BEFORE  "LoopUnroll"
//! INST_COUNT   "Shr", 1
//! PASS_AFTER   "LoopUnroll"
//! INST_COUNT   "Shr", 8
//!
//! METHOD       "loop_unroll_shl_shr_ashr.ETSGLOBAL::test_ashr"
//! PASS_BEFORE  "LoopUnroll"
//! INST_COUNT   "AShr", 1
//! PASS_AFTER   "LoopUnroll"
//! INST_COUNT   "AShr", 8
//!
//! METHOD       "loop_unroll_shl_shr_ashr.ETSGLOBAL::test_loop_uncounted"
//! PASS_BEFORE  "LoopUnroll"
//! INST_COUNT   "Shl", 1
//! PASS_AFTER   "LoopUnroll"
//! INST_COUNT   "Shl", 6

function test_shl_neg() {
    let m = -1;
    let ret = 0;
    while (m > -258) {
        ret += m;
        m <<= 1;
    }
    return ret;
}

function test_shl_overflow() {
    let m = 0x7FFFFFFFFFFFFFFF;
    let i = 0x100000000000000;
    let ret = 0;
    while (i <= m) {
        ret += i;
        i <<= 1;
    }
    return ret;
}

function test_shl_not_one() {
    let m = 0xFFFFFFFFFF;
    let i = 0x1100;
    let ret = 0;
    while (i <= m) {
        ret += i;
        i <<= 20;
    }
    return ret;
}

function test_shl_not_overflow() {
    let m = 0x100;
    let i = 1;
    let ret = 0;
    while (i < m) {
        ret += i;
        i <<= 1;
    }
    return ret;
}

function test_ashr() {
    let m = 0xff;
    let ret = 0;
    while (m > 0) {
        ret += m;
        m >>= 1;
    }
    return ret;
}

function test_shr() {
    let m = 0xff;
    let ret = 0;
    while (m > 0) {
        ret += m;
        m >>>= 1;
    }
    return ret;
}

function test_loop_uncounted(m : int) {
    let i = 1;
    let ret = 0;
    while (i < m) {
        ret += i;
        i <<= 1;
    }
    return ret;
}

function main()
{
    arktest.assertEQ(test_shl_not_overflow(), 255);
    arktest.assertEQ(test_ashr(),502);
    arktest.assertEQ(test_shr(), 502);
    arktest.assertEQ(test_loop_uncounted(1024), 1023);
    arktest.assertEQ(test_loop_uncounted(0), 0);
    arktest.assertEQ(test_loop_uncounted(-5), 0);
    arktest.assertEQ(test_loop_uncounted(5), 7);
}