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

function main(): int {
    let failures: int = 0;
    failures += test(good(1, 0), "Log10 from int 1");
    failures += test(good(10, 1), "Log10 from int 10");
    failures += test(good(100, 2), "Log10 from int 100");
    failures += test(good(1000, 3), "Log10 from int 1000");
    failures += test(good(10000, 4), "Log10 from int 10000");
    failures += test(good(100000, 5), "Log10 from int 100000");
    failures += test(good(1000000, 6), "Log10 from int 1000000");
    failures += test(good(10000000, 7), "Log10 from int 10000000");
    failures += test(good(100000000, 8), "Log10 from int 100000000");
    failures += test(good(1000000000, 9), "Log10 from int 1000000000");
    return failures
}

function test(result: int, message: String ): int {
    if(result == 0) {
      return 0;      
    }
    console.println("FAILED: " + message);
    return 1;
}

function bad():int {
    try {
        log10(-1)
    } catch(e) {
        return 0;   
    }
    return 1;
}

function good(v: int, expected: int):int {
    try {
        if(log10(v) == expected) return 0;
        return 1;
    } catch(e) {
        return 1;
    }
}