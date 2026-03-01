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

//! CHECKER         Got lambda with capture int from es2panda without unneccary checkcast
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*main.*'"
//! METHOD          "lambda_with_capture_optimized.ETSGLOBAL::main"
//! PASS_AFTER      "IrBuilder"
//! INST_NOT        "std.core.Box"
//! INST            "std.core.BooleanBox"
//! INST            "std.core.ByteBox"
//! INST            "std.core.ShortBox"
//! INST            "std.core.CharBox"
//! INST            "std.core.IntBox"
//! INST            "std.core.LongBox"
//! INST            "std.core.FloatBox"
//! INST            "std.core.DoubleBox"

function main(): int {
    {
        let capture: boolean = true;
        () => { capture = false;}
    }
    {
        let capture: byte = 1;
        () => { capture = 0;}
    }
    {
        let capture: short = 1;
        () => { capture = 0;}
    }
    {
        let capture: char = c'1';
        () => { capture = c'0';}
    }
    {
        let capture: int = 1;
        () => { capture = 0;}
    }
    {
        let capture: long = 1;
        () => { capture = 0;}
    }
    {
        let capture: float = 1.0f;
        () => { capture = 0;}
    }

    {
        let capture: double = 1.0;
        () => { capture = 0.0;}
    }
    return 0;
}
