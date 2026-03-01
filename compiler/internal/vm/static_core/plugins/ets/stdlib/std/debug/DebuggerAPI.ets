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

package std.debug;

export final class DebuggerAPI {
    public static native getLocalBoolean(regNumber: long): boolean

    public static native getLocalByte(regNumber: long): byte

    public static native getLocalShort(regNumber: long): short

    public static native getLocalChar(regNumber: long): char

    public static native getLocalInt(regNumber: long): int

    public static native getLocalFloat(regNumber: long): float

    public static native getLocalDouble(regNumber: long): double

    public static native getLocalLong(regNumber: long): long

    public static native getLocalObject(regNumber: long): Object

    public static native setLocalBoolean(regNumber: long, value: boolean): void

    public static native setLocalByte(regNumber: long, value: byte): void

    public static native setLocalShort(regNumber: long, value: short): void

    public static native setLocalChar(regNumber: long, value: char): void

    public static native setLocalInt(regNumber: long, value: int): void

    public static native setLocalFloat(regNumber: long, value: float): void

    public static native setLocalDouble(regNumber: long, value: double): void

    public static native setLocalLong(regNumber: long, value: long): void

    public static native setLocalObject(regNumber: long, value: Object): void
}
