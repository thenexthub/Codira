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

let Empty: String = '';
let EscChars: String = '\f\n\r\t\v';
let RusChars: String = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя'; // 16-bit
let EngChars: String = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz';               //  8-bit
let DupChars: String = 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA';               //  8-bit
const Digits: String = '0123456789';
const Symbols: String = '`~!@#$%^&*()_-+={}[]:;\"\'\\|/<>,.';
const CharNotFound: int = -1;

function main(): int {

    /**
     * indexOf() without an explicitly specified position to start from (i.e. position is 0)
     */

    if (Empty.indexOf(c'x') != CharNotFound) {
        return 1;
    }
    for (let i = 0; i < EscChars.getLength(); ++i) {
        if (EscChars.indexOf(EscChars.charAt(i) as char) != i) {
            return 2
        }
    }
    for (let i = 0; i < RusChars.getLength(); ++i) {
        if (RusChars.indexOf(RusChars.charAt(i) as char) != i) {
            return 3;
        }
    }
    if (RusChars.indexOf(c'Ѫ') != CharNotFound) {
        return 4;
    }
    for (let i = 0; i < EngChars.getLength(); ++i) {
        if (EngChars.indexOf(EngChars.charAt(i) as char) != i) {
            return 5;
        }
    }
    if (EngChars.indexOf(c'æ') != CharNotFound) {
        return 6;
    }
    for (let i = 0; i < DupChars.getLength(); ++i) {
        if (DupChars.indexOf(DupChars.charAt(i) as char) != 0) {
            return 7;
        }
    }
    for (let i = 0; i < DupChars.getLength(); ++i) {
        for (let j = 0; j < DupChars.getLength(); ++j) {
            let index: int = DupChars.indexOf(DupChars.charAt(i) as char, j);
            if (index != j) {
                return 8;
            }
        }
    }

    for (let i = 0; i < Digits.getLength(); ++i) {
        if (Digits.indexOf(Digits.charAt(i)).toChar() != i) {
            return 9;
        }
    }
    for (let i = 0; i < Symbols.getLength(); ++i) {
        if (Symbols.indexOf(Symbols.charAt(i) as char) != i) {
            return 10;
        }
    }

    /**
     * indexOf() with an explicitly specified position to start from
     */

    for (let i = 0; i < EngChars.getLength(); ++i) {
        for (let j = 0; j < EngChars.getLength(); ++j) {
            let index: int = EngChars.indexOf(EngChars.charAt(i) as char, j);
            if (i < j) {
                if (index != CharNotFound) {
                    return 20;
                }
            } else if (index != i) {
                    return 21;
            }
        }
    }
    for (let i = 0; i < RusChars.getLength(); ++i) {
        for (let j = 0; j < RusChars.getLength(); ++j) {
            let index: int = RusChars.indexOf(RusChars.charAt(i) as char, j);
            if (i < j) {
                if (index != CharNotFound) {
                    return 22;
                }
            } else if (index != i) {
                    return 23;
            }
        }
    }
    for (let i = 0; i < Symbols.getLength(); ++i) {
        for (let j = 0; j < Symbols.getLength(); ++j) {
            let index: int = Symbols.indexOf(Symbols.charAt(i) as char, j);
            if (i < j) {
                if (index != CharNotFound) {
                    return 24;
                }
            } else if (index != i) {
                    return 25;
            }
        }
    }

    /* Check the case when 'position' is negative */
    if (EngChars.indexOf(EngChars.charAt(0) as char, -1) != 0) {
        return 40;
    }
    if (RusChars.indexOf(RusChars.charAt(0) as char, -1) != 0) {
        return 41;
    }

    /* Check the case when 'position' is out of range */
    if (EngChars.indexOf(EngChars.charAt(0) as char, EngChars.getLength() * 2) != CharNotFound) {
        return 42;
    }
    if (RusChars.indexOf(RusChars.charAt(0) as char, RusChars.getLength() * 2) != CharNotFound) {
        return 43;
    }

    console.println('PASS');
    return 0;
}
