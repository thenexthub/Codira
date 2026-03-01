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

function scopeTest() {
    let a = 1;
    let b = 2;
    for (let c = 3; c < 4; c++) {
        let d = 5;
    }
    let e = 6;
    let f = 7;
    {
        let g = 8;
        for (let h = 9; h < 10; h++) {
            let i = 11;
        }
    }
    let j = 12;
    function innerTest() {
        let k = 13;
        let l = 14;
        {
            let m = 15;
        }
    }
    innerTest();
    let n = 16;
    {
        let o = 17;
    }
    let p = 18;
    let q = 19;
}
scopeTest();