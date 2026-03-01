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

//! CHECKER         No virtual call to Box.set,get
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*main.*'"
//! METHOD          "lambda_final_box.ETSGLOBAL::main"
//! PASS_AFTER      "IrBuilder"
//! INST_NOT        /CallVirtual.*std.core.Box/
//! INST_NOT        /CallVirtual.*std.core.BooleanBox/
//! INST_NOT        /CallVirtual.*std.core.ByteBox/
//! INST_NOT        /CallVirtual.*std.core.ShortBox/
//! INST_NOT        /CallVirtual.*std.core.CharBox/
//! INST_NOT        /CallVirtual.*std.core.IntBox/
//! INST_NOT        /CallVirtual.*std.core.LongBox/
//! INST_NOT        /CallVirtual.*std.core.FloatBox/
//! INST_NOT        /CallVirtual.*std.core.DoubleBox/
//! INST            /CallStatic.*std.core.Box/
//! INST            /CallStatic.*std.core.BooleanBox/
//! INST            /CallStatic.*std.core.ByteBox/
//! INST            /CallStatic.*std.core.ShortBox/
//! INST            /CallStatic.*std.core.CharBox/
//! INST            /CallStatic.*std.core.IntBox/
//! INST            /CallStatic.*std.core.LongBox/
//! INST            /CallStatic.*std.core.FloatBox/
//! INST            /CallStatic.*std.core.DoubleBox/

function main(): int {
    let c_boolean: boolean = true;
    let c_byte: byte = 1 as byte;
    let c_char: char = c'a';
    let c_short: short = 123;
    let c_int: int = 123000;
    let c_long: long = 123000000000;
    let c_float: float = 1.0f;
    let c_double: double = 1.0;
    let c_object = "abc";
    c_boolean = false;
    c_byte = 2;
    c_char = c'b';
    c_short++;
    c_int++;
    c_long++;
    c_float++;
    c_double++;
    c_object = "def";
    (() => {
        c_boolean = false;
        c_byte = 2;
        c_char = c'b';
        c_short++;
        c_int++;
        c_long++;
        c_float++;
        c_double++;
        c_object = "def";
    })();
    return 0;
}
