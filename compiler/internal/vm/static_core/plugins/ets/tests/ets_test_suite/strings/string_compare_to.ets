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

import * as math from "std/math";
function main(): int {
    /* the cases to compare against:
     * empty,
     * {1, 4-1, 4, 4+1, 8-1, 8, 8+1, long} x Latin
     * {1, 2, 4-1, 4, 4+1, long} x utf-16
     * */
    let checks: [ String, String, int][] = [
        ["", "", 0],        // both empty -> EQ
        // 1 char string comparison
        ["", "A", -1],      // first empty -> LT
        ["A", "", 1],       // second empty -> GT
        ["А", "Ф", -1],     // latin vs utf-16 -> LT
        ["Ф", "A", 1 ],     // utf-16 vs latin -> GT
        [ "A", "A", 0 ],    // same str latin -> EQ
        // latin strings
        // under 8 bytes string comparison
        [ "A", "AAA", -1 ],         // same prefix -> LT
        [ "AAA", "A", 1 ],          // same prefix ->GT
        [ "AAA", "AAA", 0 ],        // same 3c str -> EQ
        [ "AAAA", "AAAA", 0 ],      // same 4c str -> EQ
        [ "AAAX", "AAAA", 1 ],      // different 4c str -> GT
        [ "AAAA", "AAAX", -1],      // different 4c str -> LT
        [ "AAAA", "AAAAA", -1 ],    // same prefix 4c vs 5c -> LT
        [ "AAAAA", "AAAA", 1 ],     // same prefix 5c vs 4c -> GT
        [ "AAAAX", "AAAAX", 0 ],    // same 5c str -> EQ
        [ "AAAAA", "AAAAX", -1],    // same 5c str -> LT
        [ "AAAAX", "AAAAA", 1 ],    // same 5c str -> EQ
        [ "AAAAX", "AAAAAAA", 1 ],  // different str -> GT
        [ "AAAAAAA", "AAAAX", -1 ],  // different str -> LT
        // under 16 bytes string comparison
        [ "AAAAAAAA", "AAAAAAAA", 0 ], // same 8c str -> EQ
        [ "AAAAAAAX", "AAAAAAAA", 1 ], // different 8c str -> GT
        [ "AAAAAAAA", "AAAAAAAX", -1], // different 8c str -> LT
        [ "AAAAAAAA", "AAAAAAAAAAAA", -1 ], // same prefix different length -> LT
        [ "AAAAAAAAAAA", "AAAAAAAAA", 1 ], // same prefix different length -> GT
        [ "AAAAAAAAXXX", "AAAAAAAAXXX", 0 ], // same str -> EQ
        [ "XXXAAAAAXXX", "AAAAAAAAXXX", 1 ], // different str same length -> GT
        [ "AAAAAAAAXXX", "XXXAAAAAXXX", -1], // different str same length -> LT
        [ "XXXAAAAAX", "AAAAAAAAXXX", 1 ], // different str different length -> GT
        [ "AAAAAAAAXXX", "XAAAAAXXX", -1], // different str different length -> LT
        // over 16 bytes non-multiple of 8 string comparison
        [ "AAAAAAAAAAAAAAAAA", "AAAAAAAAAAAAAAAAA", 0], // same str same length -> EQ
        [ "AAAAAAAAAAAAAAAAA", "XAAAAAAAAAAAAAAAA", -1], // different str same length -> LT
        [ "XAAAAAAAAAAAAAAAA", "AAAAAAAAAAAAAAAAA", 1], // different str same length -> GT
        [ "XAAAAAAAAAAAAAAAA", "AAAAA", 1], // different str different length -> GT
        [ "AAAAAAAA", "XAAAAAAAAAAAAAAAA", -1], // different str different length -> LT
        // over 16 bytes multiple of 8 string comparison
        [ "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0 ], // same strings -> EQ
        [ "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAX", -1], // same prefix different length -> LT
        [ "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAX",
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 1], // same prefix different length -> GT
        [ "XAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
          "AAAAAAAAAAAAAAAAAAAAAAAX", 1], // different str different length -> GT
        [ "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAX",
          "XAAAAAAAAAAAAAAA", -1], // different str different length -> LT
        [ "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAX", -1], // different str same length -> LT
        [ "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAX",
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 1], // different str same length -> GT
        // utf-16 strings
        // under 8 bytes string comparison
        [ "Г", "ГГГ", -1 ],         // same prefix -> LT
        [ "ГГГ", "Г", 1 ],          // same prefix ->GT
        [ "ГГГ", "ГГГ", 0 ],        // same 3c str -> EQ
        [ "ГГГГ", "ГГГГ", 0 ],      // same 4c str -> EQ
        [ "ГГГФ", "ГГГГ", 1 ],      // different 4c str -> GT
        [ "ГГГГ", "ГГГФ", -1],      // different 4c str -> LT
        [ "ГГГГ", "ГГГГГ", -1 ],    // same prefix 4c vs 5c -> LT
        [ "ГГГГГ", "ГГГГ", 1 ],     // same prefix 5c vs 4c -> GT
        [ "ГГГГФ", "ГГГГФ", 0 ],    // same 5c str -> EQ
        [ "ГГГГГ", "ГГГГФ", -1],    // same 5c str -> LT
        [ "ГГГГФ", "ГГГГГ", 1 ],    // same 5c str -> EQ
        [ "ГГГГФ", "ГГГГГГГ", 1 ],  // different str -> GT
        [ "ГГГГГГГ", "ГГГГФ", -1 ],  // different str -> LT
        // under 16 bytes string comparison
        [ "ГГГГГГГГ", "ГГГГГГГГ", 0 ], // same 8c str -> EQ
        [ "ГГГГГГГФ", "ГГГГГГГГ", 1 ], // different 8c str -> GT
        [ "ГГГГГГГГ", "ГГГГГГГФ", -1], // different 8c str -> LT
        [ "ГГГГГГГГ", "ГГГГГГГГГГГГ", -1 ], // same prefix different length -> LT
        [ "ГГГГГГГГГГГ", "ГГГГГГГГГ", 1 ], // same prefix different length -> GT
        [ "ГГГГГГГГФФФ", "ГГГГГГГГФФФ", 0 ], // same str -> EQ
        [ "ФФФГГГГГФФФ", "ГГГГГГГГФФФ", 1 ], // different str same length -> GT
        [ "ГГГГГГГГФФФ", "ФФФГГГГГФФФ", -1], // different str same length -> LT
        [ "ФФФГГГГГФ", "ГГГГГГГГФФФ", 1 ], // different str different length -> GT
        [ "ГГГГГГГГФФФ", "ФГГГГГФФФ", -1], // different str different length -> LT
        // over 16 bytes non-multiple of 8 string comparison
        [ "ГГГГГГГГГГГГГГГГГ", "ГГГГГГГГГГГГГГГГГ", 0], // same str same length -> EQ
        [ "ГГГГГГГГГГГГГГГГГ", "ФГГГГГГГГГГГГГГГГ", -1], // different str same length -> LT
        [ "ФГГГГГГГГГГГГГГГГ", "ГГГГГГГГГГГГГГГГГ", 1], // different str same length -> GT
        [ "ФГГГГГГГГГГГГГГГГ", "ГГГГГ", 1], // different str different length -> GT
        [ "ГГГГГГГГ", "ФГГГГГГГГГГГГГГГГ", -1], // different str different length -> LT
        // over 16 bytes multiple of 8 string comparison
        [ "ГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГ",
          "ГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГ", 0 ], // same strings -> EQ
        [ "ГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГ",
          "ГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГФ", -1], // same prefix different length -> LT
        [ "ГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГФ",
          "ГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГ", 1], // same prefix different length -> GT
        [ "ФГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГ",
          "ГГГГГГГГГГГГГГГГГГГГГГГФ", 1], // different str different length -> GT
        [ "ГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГФ",
          "ФГГГГГГГГГГГГГГГ", -1], // different str different length -> LT
        [ "ГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГ",
          "ГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГФ", -1], // different str same length -> LT
        [ "ГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГФ",
          "ГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГГ", 1], // different str same length -> GT
        // mixed (utf vs latin) strings comparison, never equals
        ["A", "Ф", -1],
        ["Ф", "A", 1],
        ["Ф", "AA", 1],
        ["AA", "Ф", -1],
        ["AX", "AФ", -1], // same prefix LT
        ["AФ", "AXX", 1], // same prefix LT
        ["AX", "AAAФ", 1], // same prefix LT
        ["AAX", "AAAФ", 1], // same prefix LT
        ["AAAX", "AAAAФ", 1], // same prefix LT
        ["AAAX", "AФ", -1], // same prefix LT
        ["AAAФ", "AX", -1], // same prefix GT
        ["AФ", "AAAX", 1], // same prefix GT
        ["AAAX", "AAAФ", -1], // same prefix LT
        ["AAAФ", "AAAX", 1],  // same prefix GT
        ["AAAA", "ФAAA", -1], // same suffix LT
        ["ФAAA", "AAAA", 1],  // same suffix GT
        ["XXXX", "AXXФ", 1],  // same prefix, GT
        ["AXXФ", "XXXX", -1],  // different prefix, LT
        ["AAAAAAAФ", "AAAAAAXX", -1], // same prefix, LT
        ["AAAAAAAФ", "AAAA", 1], // is a substring, GT
        ["AAAA", "AAAAAAAФ", -1],  // is a substring, LT
        ["AAAAAAAФ", "AAAAAAAX", 1],  // same prefix, GT
        ["AAAAAAAX", "AAAAAAAФ", -1], // same prefix, LT
        ["AAAAAAXX", "AAAAAAAФ", 1], // same prefix, GT
        ["ABCDEFGHJKLMNOXX", "ABCDEFGHJKLMNOAФ", 1], // same prefix, GT
        ["ABCDEFGHJKLMNOAФ", "ABCDEFGHJKLMNOXX", -1], // same prefix, LT
        ["AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAX", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAФ", -1], // same prefix, LT
        ["AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAФ", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAX", 1], // same prefix, GT
        ["AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAXX", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAФ", 1], // same prefix, LT
        ["AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAФ", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAXX", -1] // same prefix, GT
    ]

    for (let i: int = 0; i < checks.length; ++i) {
        let str1: String = checks[i][0]
        let str2: String = checks[i][1]
        let expected: int = checks[i][2]
        let actual: int = math.sign(str1.compareTo(str2).toDouble());

        if (expected != actual) {
            console.println("'" + str1 + "' compared to '" + str2 + "' expected: " +
                            expected + ", got: " + str1.compareTo(str2));
            return 1;
        }
    }

    return 0;
}
