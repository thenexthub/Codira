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

/*---
desc: Rest function spread with generic tuple
---*/


function genericRestFunction<A, B, C>(...tuple: [A, B, C]) {
}

class TH<A,B,C> {
    genericRestMethod(...tuple: [A, B, C]) {
        genericRestFunction<A, B, C>(...tuple)
    }

    static restMethod<A, B, C>(...v: [A, B, C]) {
    }
}

function main() {
    genericRestFunction<number, string, boolean>(1, "2", false);

    let mc = new TH<number, string, boolean>
    mc.genericRestMethod(1, "2", false);

    TH.restMethod<number, string, boolean>(1, "2", false);
}
