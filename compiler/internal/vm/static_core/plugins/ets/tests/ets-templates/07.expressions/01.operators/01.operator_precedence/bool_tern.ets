/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

/*---
desc: ternary operator on boolean values
---*/

let res = "";

function ff(): boolean {
    res += "F";
    return false;
}

function ft(): boolean {
    res += "T";
    return true;
}

function main() {

    let v = false, vt = true, vf = false
    res = "";

    v = ft() ? ff() ? vt: ft() ? ff() ? vt : ff() ? vt : vf : vf : vf;
    arktest.assertFalse(v || res != "TFTFF");

    
}
