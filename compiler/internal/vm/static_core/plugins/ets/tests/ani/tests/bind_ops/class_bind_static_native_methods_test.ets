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

class SmallBox {
    public static native foo(): int
    public static native foo1(): int
    public native foo(): int
    public native bar(): int
    public native bar2(): int

    public static native overload_fn_int(v: int): void
    public static native overload_fn_long(v: long): void
    static overload overload_fn{overload_fn_int, overload_fn_long}

    public static not_native(v: int): int {
        return v
    }
}

class BigBox extends SmallBox {
    public static native bar(): int
}

class CheckSignature {
    public static native method(a: String): void
}
