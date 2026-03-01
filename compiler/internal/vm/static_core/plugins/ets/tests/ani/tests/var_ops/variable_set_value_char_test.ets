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

namespace anyns {
    export let aChar: char = c'A';
    export let aDouble: double = 3.0;
    export let sum: char;
    class A {
        static val1: char = c'C';
        static val2: char = Int.toChar(2);
        public static add(): char {
            sum = (A.val1.toInt() + A.val2.toInt()).toChar();
            return sum;
        }
    }

    namespace second {
        export let aValue: char = c'W';
    }
}

namespace anyns {
    export let testValue: char = c'M';
}
