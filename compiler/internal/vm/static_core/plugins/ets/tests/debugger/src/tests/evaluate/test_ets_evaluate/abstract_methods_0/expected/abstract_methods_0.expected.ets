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

package evalPatchFunction;

import {A, C} from "../abstract_methods_0.base"

class evalPatchFunction {
    private static evalPatchFunction() {
        let variable: A = DebuggerAPI.getLocalObject(0) as A;

        let state = variable.return_state();

        variable = new C(30);
        let state2 = variable.return_state();

        const evalGeneratedResult = (3 * state == state2);
        DebuggerAPI.setLocalObject(0, variable);
        return evalGeneratedResult;
    }
}
