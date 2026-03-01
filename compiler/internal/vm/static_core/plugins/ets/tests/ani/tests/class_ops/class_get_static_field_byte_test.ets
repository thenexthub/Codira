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
class TestByte {
    static byte_value: byte = 126;
    static string_value: String = "true";
    static age: byte = 0;
    static special1: string = "";
    static special2: null = null;
    static special3: undefined  = undefined ;
    static special4: string = "\n";
    static special5: string = "\r";
    static special6: string = "\t";
    static special7: string = "\b";
    static special8: string = "\a";
    static special9: string = "\f";
    static special10: string = "\v";
    static special11: string = "\0";
    static byteMin: byte = -128;
    static byteMax: byte = 127;
}

class TestByteA extends TestByte {
    static byte_value: byte = 126;
}

final class TestByteFinal extends TestByte {
    static byte_value: byte = 126;
}