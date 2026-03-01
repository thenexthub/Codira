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

class BitopsBitwiseAnd {
    n: int = 600000;
    static readonly expected: long = 0;

    public main(): int {
        let bitwiseAndValue: long = 4294967296;
        for (let i: int = 0; i < this.n; i++) {
          bitwiseAndValue = bitwiseAndValue & i;
        }
        if (bitwiseAndValue != BitopsBitwiseAnd.expected) {
          console.print("ERROR: bad result: expected " + BitopsBitwiseAnd.expected + " but got " + bitwiseAndValue);
          console.println();
          return 1;
        }
        return 0;
    }
}
