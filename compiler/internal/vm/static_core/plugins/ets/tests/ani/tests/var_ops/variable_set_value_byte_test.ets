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
    export let aByte: byte = 2;
    export let aDouble: double = 3.0;
    export let sum: byte;
    class A {
        static val1: byte = 5;
        static val2: byte = 1;
        public static add(): byte {
            sum = (A.val1 + A.val2).toByte();
            return sum;
        }
    }

    namespace second {
        export let aValue: byte = 20;
    }
}

namespace anyns {
    export let testValue: byte = 30;
}