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

class GetCharStatic {
    static name: char = c'b';
    static age: int = 20;
    static specia2: string = "";
    static specia3: null = null;
    static specia4: undefined  = undefined ;
    static specia5: char = c'\n';
    static specia6: char = c'\r';
    static specia7: char = c'\t';
    static specia8: char = c'\b';
    static specia9: char = c'\a';
    static specia10: char = c'\f';
    static specia11: char = c'\v';
    static specia12: char = c'\0';
    static charMin: char = Int.toChar(0);
    static charMax: char = Int.toChar(65535);
}

class CharStaticA extends GetCharStatic {
    static char_value: char = c'a';
}

final class CharStaticFinal extends GetCharStatic {
    static char_value: char = c'a';
}
