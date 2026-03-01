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

//! CHECKER    AOT IR Builder inlining Char.isUpperCase()
//! SKIP_IF    @architecture == "arm32"
//! RUN_AOT    options: "--compiler-regex='.*main.*'"
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callIsUpperCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callIsLowerCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callToUpperCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! INST_NEXT  /Mul/
//! INST_NEXT  /Add/
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callToLowerCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! INST_NEXT  /Mul/
//! INST_NEXT  /Add/

//! CHECKER    Char.isUpperCase() in JIT
//! RUN        force_jit: true, options: "--compiler-regex='.*main.*'", entry: "ets_char_isuppercase.ETSGLOBAL::main"
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callIsUpperCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callIsLowerCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callToUpperCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! INST_NEXT  /Mul/
//! INST_NEXT  /Add/
//! METHOD     "ets_char_isuppercase.ETSGLOBAL::callToLowerCase"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /Sub/
//! INST_NEXT  /Compare BE u16/
//! INST_NEXT  /Mul/
//! INST_NEXT  /Add/

function callIsUpperCase(ch: Char): boolean {
  return Char.isUpperCase(ch);
}

function callToUpperCase(ch: Char): char {
  return Char.toUpperCase(ch);
}

function callIsLowerCase(ch: Char): boolean {
  return Char.isLowerCase(ch);
}

function callToLowerCase(ch: Char): char {
  return Char.toLowerCase(ch);
}

function main(): int {
    arktest.assertTrue(!callIsUpperCase(c'a'));
    arktest.assertTrue(!callIsUpperCase(c'z'));
    arktest.assertTrue(callIsUpperCase(c'A'));
    arktest.assertTrue(callIsUpperCase(c'Z'));
    arktest.assertTrue(!callIsUpperCase(c'+'));

    arktest.assertTrue(callIsLowerCase(c'a'));
    arktest.assertTrue(callIsLowerCase(c'z'));
    arktest.assertTrue(!callIsLowerCase(c'A'));
    arktest.assertTrue(!callIsLowerCase(c'Z'));
    arktest.assertTrue(!callIsLowerCase(c'+'));

    arktest.assertEQ(c'A', callToUpperCase(c'a'));
    arktest.assertEQ(c'Z', callToUpperCase(c'z'));
    arktest.assertEQ(c'A', callToUpperCase(c'A'));
    arktest.assertEQ(c'Z', callToUpperCase(c'Z'));
    arktest.assertEQ(c'+', callToUpperCase(c'+'));

    arktest.assertEQ(c'a', callToLowerCase(c'a'));
    arktest.assertEQ(c'z', callToLowerCase(c'z'));
    arktest.assertEQ(c'a', callToLowerCase(c'A'));
    arktest.assertEQ(c'z', callToLowerCase(c'Z'));
    arktest.assertEQ(c'+', callToLowerCase(c'+'));

    return 0;
}

