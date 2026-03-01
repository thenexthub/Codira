/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

//! CHECKER         Test cross-file inlining with LoadString/typeof optimization (ESValue.isUndefined)
//! SKIP_IF         @architecture == "arm32"
//! RUN             force_jit: true, options: "--compiler-regex=.*checkESValueUndefined.*",
//!                 entry: "cross_file_inline_typeof.ETSGLOBAL::main"
//! METHOD          "cross_file_inline_typeof.ETSGLOBAL::checkESValueUndefined"
//! PASS_AFTER      "Inline"
//! INST_COUNT      "Intrinsic.StdCoreStringEquals", 1
//! INST_COUNT      "Intrinsic.CompilerEtsTypeof", 1
//! PASS_AFTER      "Peepholes"
//! INST_COUNT      "Intrinsic.CompilerEtsTypeof", 0

function checkESValueUndefined(): boolean {
    // ESValue.isUndefined() implementation in stdlib:
    // return this.typeOf() == "undefined";
    //
    // This perfectly matches the crash scenario:
    // 1. checkESValueUndefined inlines ESValue.isUndefined (from etsstdlib.abc)
    // 2. ESValue.isUndefined inlines ESValue.typeOf
    // 3. Resulting IR has: typeof ev == "undefined"
    //
    // The "undefined" literal ID belongs to etsstdlib.abc.
    // If Peephole optimizer uses the current method's (checkESValueUndefined) context
    // to resolve this string ID, it will crash.
    
    let v = ESValue.Undefined;
    return v.isUndefined();
}

function main(): int {
    if (!checkESValueUndefined()) {
        return 1;
    }
    return 0;
}
