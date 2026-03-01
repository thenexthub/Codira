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
name: type-call-expression-with-param_4.ets


---*/

let name: String = "0";
let counter: int = 0;
let pi: number = 0;

class CallMeTrinity {
    static $_invoke(val: int) { counter = val; }
    static $_invoke(val: String) { name = val; }
    static $_invoke(val: number) { pi = val; }
}

function main(): int {
    name = "0";
    counter = 0;
    pi = 0;
    CallMeTrinity.$_invoke("Trinity");
    CallMeTrinity.$_invoke(42);
    CallMeTrinity.$_invoke(3.14159265358)
    return name == "Trinity" && counter == 42 && pi == 3.14159265358 ? 0 : 1;
}
