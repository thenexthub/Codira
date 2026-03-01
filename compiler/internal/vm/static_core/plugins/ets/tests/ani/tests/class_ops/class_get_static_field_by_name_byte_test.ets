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

class GetbyteStatic {
    static name: string = "Bob";
    static age: byte = 0;
    static specia2: string = "";
    static specia3: null = null;
    static specia4: undefined  = undefined ;
    static specia5: string = '\n';
    static specia6: string = '\r';
    static specia7: string = '\t';
    static specia8: string = '\b';
    static specia9: string = '\a';
    static specia10: string = '\f';
    static specia11: string = '\v';
    static specia12: string = '\0';
    static byteMin: byte = -128;
    static byteMax: byte = 127;
}

class ByteStaticA extends GetbyteStatic {
    static byte_value: byte = 0;
}

final class ByteStaticFinal extends GetbyteStatic {
    static byte_value: byte = 0;
}