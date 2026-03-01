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

function main(): int {
    let failures = 0;

    failures += testEscapeId("azAZ123@*_+-./_")

    failures += testUnescapeId("abc123")
    failures += testUnescapeId("")
    failures += testUnescapeId("%1")
    failures += testUnescapeId("%u6fg123%6pf")
    failures += testUnescapeId('%6A', 'j')
    failures += testUnescapeId('%6a', 'j')
    failures += testUnescapeId('%5C', '\\')

    failures += testInverse("^")
    failures += testInverse("\\")
    failures += testInverse("!@#$%^&*()-=_+1234567890%%%")
    failures += testInverse("äöü")
    failures += test(unescape("%E4%F6%FC") == "äöü", "äöü failed, got " + unescape("%E4%F6%FC"))

    failures += testInverse("🌈🌬️👩🏻‍🏭💅🏿")

    failures += testInverse("^")
    failures += testInverse("♍⊕⢷⧏▏⠁∰⅛┅⍏➈③⿕⎢⍭≠⒚ⅴ⦾Ɑ⾤Ⓞ⹫⳰⻦⊮₢┄Ⲹ⩓⥬ⷱ⺽⎳≧⎘↾ⱓ✈ⳓ⇯Ⓑ⟋☱⨴⾼⾿⭉⺓⋇‷⾏Ⰾ✤⾤⤺⾐⿢⻶☓⳷℁▧◴⎀⛟♙⛨↹⼍⫤⎷⡚⪛⎾⾡⊼⎘⎈⌇⒱⽨⒯╖⏦⫻⇲⨩╅⨞")

    if (failures != 0) {
        console.println("Failed " + failures + " tests")
    }
    return failures
}

function testEscapeId(s1: string): int {
    return testEscapeId(s1, s1)
}

function testEscapeId(s1: string, s2: string): int {
    return test(escape(s1) == s2, 'escape("' + s1 + '") == "' + s2 + '" failed; got "' + escape(s1) + '"')
}

function testUnescapeId(s1: string): int {
    return testUnescapeId(s1, s1)
}

function testUnescapeId(s1: string, s2: string): int {
    return test(unescape(s1) == s2, 'unescape("' + s1 + '") == "' + s2 + '" failed; got "' + unescape(s1) + '"')
}

function testInverse(s1: string): int {
    return test(unescape(escape(s1)) == s1, 'unescape(escape("' + s1 + '")) == "' + s1 + '" failed, got "' + unescape(escape(s1)) + '", escaped "' + escape(s1) + '"')
}

function test(result: boolean, msg: string): int {
    if (!result) {
        console.println(msg)
        return 1
    }
    return 0
}
