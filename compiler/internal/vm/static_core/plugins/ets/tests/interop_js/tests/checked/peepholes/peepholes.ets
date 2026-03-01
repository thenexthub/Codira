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

import { cons, car, cdr, sum, TreeNode, makeDynObject, doNothing, makeSwappable, StaticClass, extractSquaredInt, ObjectWithPrototype, vundefined, dynStorage }
    from "checked/js_call/js_call.js"

//! CHECKER         Fuse getProperty and cast to string in BCO
//! RUN_BCO         method: "peepholes.ETSGLOBAL.jsLoadAndCastStr"
//! METHOD          "jsLoadAndCastStr"
//! PASS_AFTER      "IrBuilder"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 1
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyString"
//! BC_METHOD       "peepholes.ETSGLOBAL.jsLoadAndCastStr"
//! INST_COUNT      /call.* std.interop.js.JSRuntime.getPropertyJSValue:std.interop.js.JSValue;std.core.String;std.interop.js.JSValue;/, 1
//! INST_COUNT      /call.* std.interop.js.JSRuntime.getPropertyString:std.interop.js.JSValue;std.core.String;std.core.String;/, 1
//! RUN             entry: "jsLoadAndCastStr"

function jsLoadAndCastStr(): int {
    let v = dynStorage.str;
    if (v as String != "abcd") {
        return 1;
    }
    return 0;
}

//! CHECKER         Fuse getProperty and cast to double in BCO
//! RUN_BCO         method: "peepholes.ETSGLOBAL.jsLoadAndCastDbl"
//! METHOD          "jsLoadAndCastDbl"
//! PASS_AFTER      "IrBuilder"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 1
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyDouble"
//! BC_METHOD       "peepholes.ETSGLOBAL.jsLoadAndCastDbl"
//! INST_COUNT      /call.*JSRuntime.getPropertyJSValue:std.interop.js.JSValue;std.core.String;std.interop.js.JSValue;/, 1
//! INST_COUNT      /call.*JSRuntime.getPropertyDouble:std.interop.js.JSValue;std.core.String;f64;/, 1
//! RUN             entry: "jsLoadAndCastDbl"
//! RUN             entry: "jsLoadAndCastDbl"

function jsLoadAndCastDbl(): int {
    let v = dynStorage.dbl;
    if (v as double != 1.9) {
        return 1;
    }
    return 0;
}

function jsLoadAndCastIntToDbl(): int {
    let v = dynStorage.integer;
    if (v as double != 6) {
        return 1;
    }
    return 0;
}

//! CHECKER         Fuse getProperty and cast to int in BCO
//! RUN_BCO         method: "peepholes.ETSGLOBAL.jsLoadAndCastInt"
//! METHOD          "jsLoadAndCastInt"
//! PASS_AFTER      "IrBuilder"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! INST            "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT       /i32.*Cast f64/
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 1
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyDouble"
//! BC_METHOD       "peepholes.ETSGLOBAL.jsLoadAndCastInt"
//! INST_COUNT      /call.*JSRuntime.getPropertyJSValue:std.interop.js.JSValue;std.core.String;std.interop.js.JSValue;/, 1
//! INST_COUNT      /call.*JSRuntime.getPropertyDouble:std.interop.js.JSValue;std.core.String;f64;/, 1
//! RUN             entry: "jsLoadAndCastInt"

function jsLoadAndCastInt(): int {
    let v = dynStorage.integer;
    if (v as int != 6) {
        return 1;
    }
    return 0;
}

//! CHECKER         Fuse getProperty and cast to boolean in BCO
//! RUN_BCO         method: "peepholes.ETSGLOBAL.jsLoadAndCastBool"
//! METHOD          "jsLoadAndCastBool"
//! PASS_AFTER      "IrBuilder"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 4
//! INST_COUNT      "Intrinsic.JSRuntimeGetValueBoolean", 2
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyBoolean"
//! BC_METHOD       "peepholes.ETSGLOBAL.jsLoadAndCastBool"
//! INST_COUNT      /call.*JSRuntime.getPropertyJSValue:std.interop.js.JSValue;std.core.String;std.interop.js.JSValue;/, 2
//! INST_COUNT      /call.*JSRuntime.getPropertyBoolean:std.interop.js.JSValue;std.core.String;u1;/, 2
//! RUN             entry: "jsLoadAndCastBool"

function jsLoadAndCastBool(): int {
    let boolFalse = dynStorage.boolFalse as boolean;
    let boolTrue = dynStorage.boolTrue as boolean;
    if (boolFalse || !boolTrue) {
        return 1;
    }
    return 0;
}

//! CHECKER         Fuse cast from string to JSValue and setProperty in BCO
//! RUN_BCO         method: "peepholes.ETSGLOBAL.jsStoreStr"
//! METHOD          "jsStoreStr"
//! PASS_AFTER      "IrBuilder"
//! INST            "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT       "Intrinsic.JSRuntimeNewJSValueString"
//! INST_NEXT       "Intrinsic.JSRuntimeSetPropertyJSValue"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT       "Intrinsic.JSRuntimeSetPropertyString"
//! BC_METHOD       "peepholes.ETSGLOBAL.jsStoreStr"
//! INST_COUNT      /call.*JSRuntime.getPropertyJSValue:std.interop.js.JSValue;std.core.String;std.interop.js.JSValue;/, 1
//! INST_COUNT      /call.*JSRuntime.setPropertyString:std.interop.js.JSValue;std.core.String;std.core.String;/, 1
//! INST_NOT        /JSRuntime.setPropertyJSValue/
//! RUN             entry: "checkJsStore"

function jsStoreStr() {
    dynStorage.str = "dcba";
}

function jsStoreDbl() {
    dynStorage.dbl = 2.4;
}

function jsStoreInt() {
    dynStorage.integer = 31;
}

function jsStoreBool() {
    dynStorage.boolTrue = false;
}

function checkJsStore(): int {
    jsStoreStr();
    jsStoreDbl();
    jsStoreInt();
    jsStoreBool();
    if (dynStorage.verify() as int != 4) {
        return 1;
    }
    return 0;
}
