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

class Cls {
    static func_s(i: int): int {
        return i + i;
    }

    final func_f(i: int): int {
        return i + i;
    }

    func_i(i: int): int {
        return i + i;
    }
}

let show = (str: string): string => {
    return "Hi," + str;
}

function foo(x: string) : string {
    let b: string = "Function foo was called"
    return b + x;
}

function bar(y: number) : string {
    let b: string = "Function bar was called"
    return b + y;
}