/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

//! CHECKER      Check string interop JIT (slow)
//! RUN          force_jit: true, entry: "testString", options: "--compiler-regex=strings.ETSGLOBAL::testString --compiler-enable-fast-interop=false"
//! METHOD       "testString"
//! PASS_AFTER   "IrBuilder"
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueString"
//! INST_NEXT    "Intrinsic.StdCoreStringEquals"

//! CHECKER      Check string interop JIT (fast)
//! RUN          force_jit: true, entry: "testString", options: "--compiler-regex=strings.ETSGLOBAL::testString"
//! METHOD       "testString"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT    "Intrinsic.CompilerConvertJSValueToLocal"
//! INST_NEXT    "Intrinsic.CompilerResolveQualifiedJSCall"
//! INST_NOT     /LoadImmediate.*string/
//! INST_NEXT    "Intrinsic.CompilerLoadResolvedJSFunction"
//! INST_NEXT    "Intrinsic.CompilerJSCallCheck"
//! INST_NEXT    "Intrinsic.CompilerConvertRefTypeToLocal"
//! INST_NEXT    "Intrinsic.CompilerJSCallFunction"
//! INST_NEXT    "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_NEXT    "Intrinsic.CompilerDestroyLocalScope"
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueString"
//! INST_NEXT    "Intrinsic.StdCoreStringEquals"

//! CHECKER      Check string interop AOT (slow)
//! RUN_AOT      options: "--compiler-regex=strings.ETSGLOBAL::testString --compiler-enable-fast-interop=false"
//! METHOD       "testString"
//! PASS_AFTER   "IrBuilder"
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueString"
//! INST_NEXT    "Intrinsic.StdCoreStringEquals"
//! RUN          entry: "testString"
//! EVENT        /AotManager,.*,ADDED/

//! CHECKER      Check string interop AOT (fast)
//! RUN_PAOC     options: "--compiler-regex=strings.ETSGLOBAL::testString"
//! METHOD       "testString"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT    "Intrinsic.CompilerConvertJSValueToLocal"
//! INST_NEXT    "Intrinsic.CompilerResolveQualifiedJSCall"
//! INST_NOT     /LoadImmediate.*PandaFileOffset/
//! INST_NEXT    "Intrinsic.CompilerLoadResolvedJSFunction"
//! INST_NEXT    "Intrinsic.CompilerJSCallCheck"
//! INST_NEXT    "Intrinsic.CompilerConvertRefTypeToLocal"
//! INST_NEXT    "Intrinsic.CompilerJSCallFunction"
//! INST_NEXT    "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_NEXT    "Intrinsic.CompilerDestroyLocalScope"
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueString"
//! INST_NEXT    "Intrinsic.StdCoreStringEquals"
//! RUN          entry: "testString"
//! EVENT        /AotManager,.*,ADDED/

import { addString } from "checked/example/strings.js"

function testString(): int {
    let res = addString("abc");
    if (JSRuntime.getValueString(res) != "abc abc") {
        return 1;
    }
    return 0;
}
