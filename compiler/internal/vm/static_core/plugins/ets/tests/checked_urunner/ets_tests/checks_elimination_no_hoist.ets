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

let I:int = 1;
let C:int = 0;
let C1:int = 2;

//! CHECKER      Verify check's users NO_HOIST if check's input is is hoistable.
//! RUN          force_jit: true, options: "--compiler-regex=.*HoistableInput --compiler-enable-events --compiler-events-path=./events.csv", entry: "checks_elimination_no_hoist.ETSGLOBAL::HoistableInput"
//! METHOD       "checks_elimination_no_hoist.ETSGLOBAL::HoistableInput"
//! PASS_AFTER   "IrBuilder"
//! INST         /Div/; div1Id = /([0-9]+)\./.match(@ir_scope.lines[@ir_scope.current_index - 1])[1]
//! INST_NEXT    /Div/; div2Id = /([0-9]+)\./.match(@ir_scope.lines[@ir_scope.current_index - 1])[1]
//! EVENT        /checks_elimination_no_hoist.ETSGLOBAL::HoistableInput,licm,inst_id:#{div1Id}/
//! EVENT_NOT    /checks_elimination_no_hoist.ETSGLOBAL::HoistableInput,licm,inst_id:#{div2Id}/
function HoistableInput(): int {
    let u:int = 0;
    let v:int = 2;

    let c1:int = C1;
    let c:int = 0;
    // [prehead 1]
    for (let i:int = 0; i < I; i++) {
        c = c1 / 3;
        if (c > 0) {
            // 'c' != 0 here, 'v / c' can be hosisted to [prehead 2] but still
            // shouldn't be hoisted into [prehead 1]. Since 'c1 / 2' will be hoisted,
            // it will pull 'v / c' together to [prehead 1]. To prevent this, need to set
            // 'NO_HOIST' in 'v / c'.
            // [prehead 2]
            for (let k:int = 0; k < I; k++) {
                u = v / c;
            }
        }
    }

    return u;
}

//! CHECKER      Verify check's users hoisted if there is a dominating check outside.
//! RUN          force_jit: true, options: "--compiler-enable-events --compiler-events-path=./events.csv", entry: "checks_elimination_no_hoist.ETSGLOBAL::DomCheckOutside"
//! METHOD       "checks_elimination_no_hoist.ETSGLOBAL::DomCheckOutside"
//! PASS_AFTER   "IrBuilder"
//! INST         /Div/; div1Id = /([0-9]+)\./.match(@ir_scope.lines[@ir_scope.current_index - 1])[1]
//! INST_NEXT    /Div/; div2Id = /([0-9]+)\./.match(@ir_scope.lines[@ir_scope.current_index - 1])[1]
//! EVENT        /checks_elimination_no_hoist.ETSGLOBAL::DomCheckOutside,licm,inst_id:#{div2Id}/
//! EVENT_NEXT   /checks_elimination_no_hoist.ETSGLOBAL::DomCheckOutside,licm,inst_id:#{div2Id}/
function DomCheckOutside(): int {
    let u:int = 0;
    let v:int = 1;

    let c:int = C1;
    u = c / c;
    // [prehead 1]
    for (let i:int = 0; i < I; i++) {
        // [prehead 2]
        for (let j:int = 0; j < I; j++) {
            if (c > 0) {
                // 'c' != 0 since there was a check in the root loop,
                // 'v / c' can be hosisted to [prehead 1].
                // [prehead 3]
                for (let k:int = 0; k < I; k++) {
                    u = v / c;
                }
            }
        }
    }

    return u;
}

//! CHECKER      Verify check's users NO_HOIST if check's input's bounds are invalid at the outermost preheader.
//! RUN          force_jit: true, options: "--compiler-regex=.*InvalidBoundsOutside --compiler-enable-events --compiler-events-path=./events.csv", entry: "checks_elimination_no_hoist.ETSGLOBAL::InvalidBoundsOutside"
//! METHOD       "checks_elimination_no_hoist.ETSGLOBAL::InvalidBoundsOutside"
//! PASS_AFTER   "IrBuilder"
//! INST         /Div/; div1Id = /([0-9]+)\./.match(@ir_scope.lines[@ir_scope.current_index - 1])[1]
//! EVENT_NOT    /checks_elimination_no_hoist.ETSGLOBAL::InvalidBoundsOutside,licm,inst_id:#{div1Id}/
function InvalidBoundsOutside(): int {
    let u:int = 0;
    let v:int = 1;
    let c:int = C;

    // [prehead 1]
    for (let i:int = 0; i < I; i++) {
        if (c > 0) {
            // 'c' != 0 here, 'v / c' can be hosisted to [prehead 2] but still
            // shouldn't be hoisted into [prehead 1]. To prevent this, need to set
            // 'NO_HOIST' in 'v / c'.
            // [prehead 2]
            for (let j:int = 0; j < I; j++) {
                u = v / c;
            }
        }
    }

    return u;
}

//! CHECKER      Verify check's users hoisted if check's input's bounds are valid at the outermost preheader.
//! RUN          force_jit: true, options: "--compiler-enable-events --compiler-events-path=./events.csv", entry: "checks_elimination_no_hoist.ETSGLOBAL::ValidBoundsOutside"
//! METHOD       "checks_elimination_no_hoist.ETSGLOBAL::ValidBoundsOutside"
//! PASS_AFTER   "IrBuilder"
//! INST         /Div/; div1Id = /([0-9]+)\./.match(@ir_scope.lines[@ir_scope.current_index - 1])[1]
//! EVENT        /checks_elimination_no_hoist.ETSGLOBAL::ValidBoundsOutside,licm,inst_id:#{div1Id}/
//! EVENT_NEXT   /checks_elimination_no_hoist.ETSGLOBAL::ValidBoundsOutside,licm,inst_id:#{div1Id}/
function ValidBoundsOutside(): int {
    let u:int = 0;
    let v:int = 1;
    let c:int = C;

    if (c > 0) {
        // [prehead 1]
        // 'c' != 0 here, 'v / c' allowed to be hosisted to [prehead 1].
        for (let i:int = 0; i < I; i++) {
            // [prehead 2]
            for (let j:int = 0; j < I; j++) {
                u = v / c;
            }
        }
    }

    return u;
}

//! CHECKER      Check NO_HOIST flag for NullCheck.
//! RUN          force_jit: true, entry: "checks_elimination_no_hoist.ETSGLOBAL::NullCheck"
//! METHOD       "checks_elimination_no_hoist.ETSGLOBAL::NullCheck"
//! PASS_AFTER   "Codegen"
//! INST         "Intrinsic.StdCoreStringIndexOf"
function __noinline__getnull(): string | null {
    return null;
}
function __noinline__updateIdx(): int {
    return 3;
}

let RES = -3;

function NullCheck(): int {
    let nulref = __noinline__getnull();
    let x : char = c'\0';
    for (let i = 0; i < 3; i++) {
        if (nulref != null) {
            RES += nulref.indexOf(x);
        } else {
            RES += 1;
        }
    }
    return RES;
}

// NOTE(compiler): Enable fast CharAt implementation #28221
// CHECKER      Check NO_HOIST flag for BoundsCheck.
// RUN_PAOC     options: "--compiler-cross-arch=arm64 --compiler-regex=.*BoundsCheck --compiler-enable-events --compiler-events-path=./events.csv"
// METHOD       "checks_elimination_no_hoist.ETSGLOBAL::BoundsCheck"
// PASS_AFTER   "IrBuilder"
// INST         "LoadCompressedStringChar"; lcscId = /([0-9]+)\./.match(@ir_scope.lines[@ir_scope.current_index - 1])[1]
// EVENT        /checks_elimination_no_hoist.ETSGLOBAL::BoundsCheck,licm,inst_id:#{lcscId}/
function __noinline__getstr(): string {
    return "x";
}

function BoundsCheck(): int {
    let str1char = __noinline__getstr();
    for (let i = 0; i < 3; i++) {
        if (str1char.length <= 100000) {
            RES += 1;
        } else {
            RES += Char.toInt(str1char.charAt(10000));
        }
    }
    return RES;
}
