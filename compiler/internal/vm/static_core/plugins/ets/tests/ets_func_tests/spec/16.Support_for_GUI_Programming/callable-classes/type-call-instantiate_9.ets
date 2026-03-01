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
desc: 16.6 Callable Types
name: type-call-instantiate_9.ets
tags: []


---*/


class CallMeTrinity {
    value: int = 0;

    static $_instantiate(factory: () => CallMeTrinity, getter: () => int): CallMeTrinity {
        let x = factory();
        x.value = getter();
        return x;
    }

    getValue(): int {
        return this.value;
    }

}

function main(): int {
    let tt: CallMeTrinity = CallMeTrinity((): int => {return 4});
    console.log("Value: " + tt.getValue())
    return tt.getValue() == 4 ? 0 : 1;
}
