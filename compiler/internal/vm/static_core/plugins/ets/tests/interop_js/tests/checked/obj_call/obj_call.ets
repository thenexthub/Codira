/**
 * Copyright (c) 2019-2025 Huawei Device Co., Ltd.
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

import { getRetObj }
    from "checked/obj_call/obj_call.js"

class EtsClass {
    intValue: int;
    constructor(value: int) {
        this.intValue = value;
    }
}
class $jscall {
    static Init = JSRuntime.__initJSCallClass();

    native static getRetObj(top: JSValue, qname: JSValue, obj:EtsClass): EtsClass;
};


//! CHECKER      test
//! RUN          force_jit: true, entry: "test", options: "--compiler-regex=ETSGLOBAL::test"

function test() : int {
    let obj: EtsClass = new EtsClass(0);
    obj = $jscall.getRetObj(getRetObj, JSRuntime.getUndefined(), obj);
    if (obj.intValue != 5) {
       return 1;
    }
    return 0;
}
