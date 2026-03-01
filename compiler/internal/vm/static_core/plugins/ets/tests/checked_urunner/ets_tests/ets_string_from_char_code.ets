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

//! CHECKER       AOT IR Builder, check String creation from a single and numerous char code(s)
//! RUN_PAOC      options: "--compiler-regex='.*from.*CharCode.*' --compiler-inlining=true --compiler-inline-external-methods-aot=true"
//!
//! METHOD        "ets_string_from_char_code.ETSGLOBAL::fromCharCodes"
//! PASS_AFTER    "Peephole"
//! INST          /CallStatic .*std.core.String::fromCharCode/
//! PASS_AFTER    "Inline"
//! INST          /Intrinsic.StdCoreStringFromCharCode/
//! INST_NOT      /Intrinsic.CompilerEtsStringFromCharCodeSingle/
//! INST_NOT      /Intrinsic.CompilerEtsStringFromCharCodeSingleNoCache/
//!
//! METHOD        "ets_string_from_char_code.ETSGLOBAL::fromLargeCharCode"
//! PASS_AFTER    "Peephole"
//! INST_NOT      /Intrinsic.StdCoreStringFromCharCodeSingle/
//! INST          /Intrinsic.CompilerEtsStringFromCharCodeSingle/
//! INST_NOT      /Intrinsic.CompilerEtsStringFromCharCodeSingleNoCache/

//! CHECKER       AOT IR Builder, check String creation from a single and numerous char code(s) with no string caches
//! RUN_PAOC      options: "--compiler-regex='.*from.*CharCode.*' --compiler-inlining=true --compiler-inline-external-methods-aot=true --use-string-caches=false"
//!
//! METHOD        "ets_string_from_char_code.ETSGLOBAL::fromCharCodes"
//! PASS_AFTER    "Peephole"
//! INST          /CallStatic .*std.core.String::fromCharCode/
//! PASS_AFTER    "Inline"
//! INST          /Intrinsic.StdCoreStringFromCharCode/
//! INST_NOT      /Intrinsic.CompilerEtsStringFromCharCodeSingle/
//! INST_NOT      /Intrinsic.CompilerEtsStringFromCharCodeSingleNoCache/
//!
//! METHOD        "ets_string_from_char_code.ETSGLOBAL::fromLargeCharCode"
//! PASS_AFTER    "Peephole"
//! INST_NOT      /Intrinsic.StdCoreStringFromCharCodeSingle/
//! INST          /Intrinsic.CompilerEtsStringFromCharCodeSingleNoCache/

//! CHECKER       JIT IR Builder, check String creation from a single and numerous char code(s)
//! RUN           force_jit: true, options: "--compiler-regex='.*from.*CharCode.*' --compiler-inlining=true", entry: "ets_string_from_char_code.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_from_char_code.ETSGLOBAL::fromCharCodes"
//! PASS_AFTER    "Peephole"
//! INST          /CallStatic .*std.core.String::fromCharCode/
//! PASS_AFTER    "Inline"
//! INST          /Intrinsic.StdCoreStringFromCharCode/
//! INST_NOT      /Intrinsic.CompilerEtsStringFromCharCodeSingle/
//! INST_NOT      /Intrinsic.CompilerEtsStringFromCharCodeSingleNoCache/
//!
//! METHOD        "ets_string_from_char_code.ETSGLOBAL::fromLargeCharCode"
//! PASS_AFTER    "Peephole"
//! INST_NOT      /Intrinsic.StdCoreStringFromCharCodeSingle/
//! INST_NOT      /Intrinsic.CompilerEtsStringFromCharCodeSingleNoCache/
//! INST          /Intrinsic.CompilerEtsStringFromCharCodeSingle/

function fromCharCodes(code1: int, code2: int): string {
    return string.fromCharCode(code1, code2);
}

function fromLargeCharCode(code: int): string {
    return string.fromCharCode(code << 8 | code);
}

function fromOneByteCharCode(code: int): string {
    arktest.assertLT(code, 128);
    return string.fromCharCode(code);
}

function main() {
    arktest.assertEQ(fromCharCodes(0xd842, 0xdfb7), '𠮷');
    arktest.assertEQ(fromLargeCharCode(0x42), '䉂'); // 0x4242
    arktest.assertEQ(fromOneByteCharCode(0x41), 'A');
}
