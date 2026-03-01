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

package std.core

/*
  DISCLAIMER

  This API is _unsafe_ and should not be used under normal circumstances.
  The memory management in the native context (and possible clashes with
  the GC in the managed one) is the user's sole reponsibility.
*/

export namespace unsafeMemory {
    export native function writeInt8(address: long, value: byte) : void;
    export native function writeBoolean(address: long, value: boolean) : void;
    export native function writeInt16(address: long, value: short) : void;
    export native function writeInt32(address: long, value: int) : void;
    export native function writeFloat32(address: long, value: float) : void;
    export native function writeInt64(address: long, value: long) : void;
    export native function writeFloat64(address: long, value: double) : void;
    export native function writeNumber(address: long, value: number) : void;
    export native function writeString(address: long, value: String) : int;

    export native function readInt8(address: long) : byte;
    export native function readBoolean(address: long) : boolean;
    export native function readInt16(address: long) : short;
    export native function readInt32(address: long) : int;
    export native function readInt64(address: long) : long;
    export native function readFloat32(address: long) : float;
    export native function readFloat64(address: long) : double;
    export native function readNumber(address: long) : number;
    export native function readString(address: long, size: int) : String;

    export native function getStringSizeInBytes(value: string): int;
}
