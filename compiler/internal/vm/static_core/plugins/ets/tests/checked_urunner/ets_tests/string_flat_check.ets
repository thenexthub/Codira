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


//! CHECKER      Check StringFlatCheck is inserted and SaveState was not broken
//! RUN          force_jit: true, options: "--compiler-regex=string_flat_check.* --use-all-strings=true", entry: "string_flat_check.ETSGLOBAL::saveStateNotBroken"
//! METHOD       "string_flat_check.ETSGLOBAL::saveStateNotBroken"
//! PASS_BEFORE  "StringFlatCheck"
//! INST_NOT     "StringFlatCheck"
//! PASS_AFTER   "StringFlatCheck"
//! INST         "StringFlatCheck"
//! INST_NEXT    "Intrinsic.StdCoreStringRepeat"
function saveStateNotBroken(): void {
    let thrown = false
    let i = 1
    let s = '' + i  // avoid method call for LoadString result
    try {
        s.repeat(-1)
    } catch (e) {
        thrown = true
    }

    arktest.assertTrue(thrown)
}

//! CHECKER      Check StringFlatCheck OOM does not break interpreter
//! RUN          force_jit: true, options: "--compiler-regex=string_flat_check.* --use-all-strings=true", entry: "string_flat_check.ETSGLOBAL::testOutOfMemory"
//! METHOD       "string_flat_check.ETSGLOBAL::compare"
//! PASS_AFTER   "StringFlatCheck"
//! INST         "StringFlatCheck"
function testOutOfMemory(): void {
    let s1 = ''
    let s2 = ''
    for (let i = 0; i < 2000; i++) {
        s1 += i
        s2 += i
    }
    let buf = new Array<Any>()
    try {
        while (true) {
            buf.push(new Array<byte>(1000))
        }
    } catch (e) {
    }

    // memory is exhausted

    let thrown = false
    try {
        compare(s1, s2)
    } catch (e) {
        thrown = true
    }

    // allow GC to free memory
    for (let i = 0; i < buf.length; i++) {
      buf[i] = null
    }

    arktest.assertTrue(thrown)
    arktest.assertEQ(0, compare(s1, s2))
}

function compare(s1: string, s2: string): int {
    // require StringFlatCheck
    return s1.compareTo(s2)
}

//! CHECKER      Check StringFlatCheck is not inserted if tree string is disabled
//! RUN          force_jit: true, options: "--compiler-regex=string_flat_check.* --use-all-strings=false", entry: "string_flat_check.ETSGLOBAL::stringFlatCheckDisabled"
//! METHOD       "string_flat_check.ETSGLOBAL::stringFlatCheckDisabled"
//! PASS_AFTER   "Codegen"
//! INST_NOT     "StringFlatCheck"
//! INST         "Intrinsic.StdCoreStringRepeat"
function stringFlatCheckDisabled(): void {
    let i = 1
    let s = '' + i  // avoid method call for LoadString result
    let r = s.repeat(2)
    arktest.assertEQ(2, r.length)
}

//! CHECKER      Check StringFlatCheck is not inserted for instruction which returns flat string
//! RUN          force_jit: true, options: "--compiler-regex=string_flat_check.* --use-all-strings=true", entry: "string_flat_check.ETSGLOBAL::stringFlatCheckForLoadString"
//! METHOD       "string_flat_check.ETSGLOBAL::stringFlatCheckForLoadString"
//! PASS_AFTER   "Codegen"
//! INST_NOT     "StringFlatCheck"
//! INST         "Intrinsic.StdCoreStringRepeat"
function stringFlatCheckForLoadString(): void {
    let r = "a".repeat(2)
    arktest.assertEQ(2, r.length)
}

//! CHECKER      Check StringFlatCheck does not break OSR
//! SKIP_IF      @architecture != "arm64"
//! RUN          options: "--compiler-regex=string_flat_check.* --use-all-strings=true --no-async-jit --compiler-hotness-threshold=1 --compiler-enable-osr=true --compiler-ignore-failures=true", entry: "string_flat_check.ETSGLOBAL::main"
//! METHOD       "string_flat_check.ETSGLOBAL::testOsr_OSR"
//! PASS_AFTER   "StringFlatCheck"
//! INST_COUNT   "StringFlatCheck", 4
//! EVENT        /Compilation,string_flat_check.ETSGLOBAL::testOsr,0.*,COMPILED/
function testOsr(s1: string, s2: string): int {
    let s0 = s1 + s2 // should create TreeString
    let s = s1
    // StringFlatCheck can be potentially inserted here but
    // it breaks execution in OSR since this part of code
    // is executed by interpreter
    let r = s.compareTo(s0)
    for (let i = 0; i < 1000; ++i) {
        // OSR entry here
        s = s + s1
        r += s.compareTo(s0)
    }
    return r
}

function main(): void {
    let r = testOsr("1234567", "1234567")  // long enough strings to create tree string
    arktest.assertTrue(r > 0)
}
