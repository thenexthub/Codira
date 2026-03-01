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

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       AOT IR Builder, check StringBuilder internal buffer reserve
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*reserve[0-9]*' --compiler-inlining=true"
//!
//! METHOD        "ets_string_builder_reserve_uber.ETSGLOBAL::uber_reserve38"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=78)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=78)"

//! CHECKER       JIT IR Builder, check StringBuilder internal buffer reserve
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*reserve[0-9]*' --compiler-inlining=true", entry: "ets_string_builder_reserve_uber.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_reserve_uber.ETSGLOBAL::uber_reserve38"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=78)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=78)"

// CHECKER       JIT IR Builder, check StringBuilder internal buffer reallocation events occured (uber_only)
// SKIP_IF       @architecture == "arm32"
// RUN           force_jit: true, options: "--compiler-regex='.*reserve[0-9]*' --compiler-reserve-string-builder-buffer=false --compiler-inlining=true", entry: "ets_string_builder_reserve_uber.ETSGLOBAL::main"
// EVENT_NEXT    /SbBufferRealloc,\d+,0x[0-9a-fA-F]+,32.*/
// EVENT_NEXT    /SbBufferRealloc,\d+,0x[0-9a-fA-F]+,64.*/

//! CHECKER       JIT IR Builder, check StringBuilder internal buffer reallocation events not occured (uber_only)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*reserve[0-9]*' --compiler-reserve-string-builder-buffer=true --compiler-inlining=true", entry: "ets_string_builder_reserve_uber.ETSGLOBAL::main"
//! EVENT_NOT     "SbBufferRealloc"

function uber_reserve38(cond: boolean) : string
{
    let nullString: String = new String(null);
    let sb = new StringBuilder()
    sb.append(nullString)
    sb.append("-")
    for (let i = 0; i < 5; ++i) {
        switch (i) {
            case 0:
                sb.append(i)
                sb.append((i.toDouble()).toString())
                sb.append(i.toDouble().toString())
                sb.append(i.toLong().toString())
                sb.append("-")
            break
            case 1:
                sb.append(i)
                if (cond)
                    sb.append((i.toDouble()).toString())
                else
                    sb.append(i.toDouble().toString())
                sb.append(i.toLong().toString())
                sb.append("-")
            break
            case 2:
                sb.append(i)
                for (let j = 0; j < 5; ++j)
                    sb.append(5*i + j)
                sb.append(i)
                sb.append("-")
            break
            case 3:
                sb.append(i)
                for (let j = 0; j < 3; ++j) {
                    sb.append(j*4 + 1)
                    for (let k = 0; k < 2; ++k)
                        sb.append(j*4 + j + 2)
                    sb.append(j*4 + 4)
                }
                sb.append(i)
                sb.append("-")
            break
            case 4:
                sb.append(i)
                if (cond) {
                    sb.append(i + 1)
                    for (let j = 0; j < 3; ++j) {
                        sb.append(j*4 + 1)
                        if (cond)
                            sb.append(j*4 + 2)
                        else
                            sb.append(j*4 + 3)
                    }
                }
                else
                    sb.append(i + 5)
                sb.append(i)
                sb.append("-")
            break
            default:
                sb.append("Unreachable")
        }
    }
    sb.append(nullString)
    return sb.toString()
}

function main() : void {
    arktest.assertEQ(uber_reserve38(false), "null-0000-111-210111213142-31224577891212123-494-null", "Wrong result at uber_reserve38")
    arktest.assertEQ(uber_reserve38(true), "null-0000-111-210111213142-31224577891212123-4512569104-null", "Wrong result at uber_reserve38")
}
