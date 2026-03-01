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

namespace testing {

const MAGIC_NUMBER: int = 5

export function throwError(a: int): int {
    if (a == MAGIC_NUMBER) {
        throw new Error("some error message from throwError")
    }
    return 1
}

export function throwErrorNested(a: int): int {
    return baz(a)
}

function baz(a: int): int {
    return bar(a)
}

function bar(a: int): int {
    return throwError(foo(a))
}

function foo(a: int): int {
    return MAGIC_NUMBER;
}

export function throwErrorThroughNative(a: int): int {
    return callThroughNative(a)
}

/// Calls `throwToNativeCaller`
native function callThroughNative(a: int): int

function throwToNativeCaller(a: int): int {
    if (a > 0) {
        throw new Error();
    }
    return a;
}

}
