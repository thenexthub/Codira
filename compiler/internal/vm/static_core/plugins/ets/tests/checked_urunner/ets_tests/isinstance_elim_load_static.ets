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

//! CHECKER         Check IsInstance elimination for LoadStatic result
//! RUN             force_jit: true, options: "--compiler-regex='.*final_instanceof.*'", entry: "isinstance_elim_load_static.ETSGLOBAL::final_instanceof"
//! METHOD          "isinstance_elim_load_static.ETSGLOBAL::final_instanceof"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "IsInstance"

//! CHECKER         Check IsInstance elimination for LoadStatic result (AOT)
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*final_instanceof.*'"
//! METHOD          "isinstance_elim_load_static.ETSGLOBAL::final_instanceof"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "IsInstance"
//! RUN             options: "", entry: "isinstance_elim_load_static.ETSGLOBAL::final_instanceof"

final class Final {}

class Unrelated {}

const asFinal: Final = new Final()

function final_instanceof(): int {
    if (asFinal instanceof Unrelated) {
        return 1
    }
    return 0
}

//! CHECKER         Check checkcast elimination and complex instanceof non-elimination for LoadStatic result
//! RUN             force_jit: true, options: "--compiler-regex='.*upcast_instanceof.*'", entry: "isinstance_elim_load_static.ETSGLOBAL::upcast_instanceof"
//! METHOD          "isinstance_elim_load_static.ETSGLOBAL::upcast_instanceof"
//! PASS_BEFORE     "ObjectTypeCheckElimination"
//! INST            "CheckCast"
//! PASS_AFTER      "ObjectTypeCheckElimination"
//! INST_NOT        "CheckCast"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "IsInstance"

//! CHECKER         Check checkcast elimination and complex instanceof non-elimination for LoadStatic result (AOT)
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*upcast_instanceof.*'"
//! METHOD          "isinstance_elim_load_static.ETSGLOBAL::upcast_instanceof"
//! PASS_BEFORE     "ObjectTypeCheckElimination"
//! INST            "CheckCast"
//! PASS_AFTER      "ObjectTypeCheckElimination"
//! INST_NOT        "CheckCast"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "IsInstance"
//! RUN             options: "", entry: "isinstance_elim_load_static.ETSGLOBAL::upcast_instanceof"

class Base {}
class Child extends Base {}

const asChild: Child = new Child;

function upcast_instanceof(): int {
    let asBase = asChild as Base;
    if (asBase instanceof Unrelated) {
        return 1
    }
    return 0
}

//! CHECKER         Check complex checkcast and instanceof non-elimination for LoadStatic result
//! RUN             force_jit: true, options: "--compiler-regex='.*downcast_nonfinal_instanceof.*'", entry: "isinstance_elim_load_static.ETSGLOBAL::downcast_nonfinal_instanceof"
//! METHOD          "isinstance_elim_load_static.ETSGLOBAL::downcast_nonfinal_instanceof"
//! PASS_BEFORE     "ObjectTypeCheckElimination"
//! INST            "CheckCast"
//! PASS_AFTER      "ObjectTypeCheckElimination"
//! INST            "CheckCast"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "IsInstance"

//! CHECKER         Check complex checkcast and instanceof non-elimination for LoadStatic result (AOT)
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*downcast_nonfinal_instanceof.*'"
//! METHOD          "isinstance_elim_load_static.ETSGLOBAL::downcast_nonfinal_instanceof"
//! PASS_BEFORE     "ObjectTypeCheckElimination"
//! INST            "CheckCast"
//! PASS_AFTER      "ObjectTypeCheckElimination"
//! INST            "CheckCast"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "IsInstance"
//! RUN             options: "", entry: "isinstance_elim_load_static.ETSGLOBAL::downcast_nonfinal_instanceof"

const asBase: Base = new Child;

function downcast_nonfinal_instanceof(): int {
    let asChild = asBase as Child;
    if (asChild instanceof Unrelated) {
        return 1
    }
    return 0
}
