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

/* White space charcodes:
    0x0020 -- space
    0x0009 -- horizontal tab
    0x000A -- line feed or new line
    0x000B -- vertical tab
    0x000C -- formfeed
    0x000D -- horizontal tab
    0x00A0 -- no-break space
    0x1680 -- Ogham space mark
    0x2000 -- en quad
    0x2001 -- em quad
    0x2002 -- en space
    0x2003 -- em space
    0x2004 -- three-per-em space
    0x2005 -- four-per-em space
    0x2006 -- six-per-em space
    0x2007 -- figure space
    0x2008 -- punctuation space
    0x2009 -- thin space
    0x200A -- hair space
    0x2028 -- line separator
    0x2029 -- paragraph separator
    0x202F -- narrow no-break space
    0x205F -- medium mathematical space
    0xFEFF -- byte order mark
    0x3000 -- ideographic space
*/
const Wss: String =
'\u0020\u0009\u000A\u000B\u000C\u000D\u00A0\u1680\
\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\
\u2008\u2009\u200A\u2028\u2029\u202F\u205F\uFEFF\u3000';

const Empty: String = '';
const Ws: String = ' ';
const X: String = 'x';
const XWs: String = X + Ws;
const WsX: String = Ws + X;
const WsXWs: String = Ws + X + Ws;
const Xy: String = 'Xy';
const WsXy: String = Ws + 'Xy';
const XyWs: String = 'Xy' + Ws;
const WsXyWs: String = Ws + 'Xy' + Ws;
const Z: String = '\u01B5'; // 'Ƶ'
const ZWs: String = Z + Ws;
const WsZ: String = Ws + Z;
const WsZWs: String = Ws + Z + Ws;
const Abc: String = 'abc';
const AwsBwsC: String = 'a b c';
const WsAbc: String = ' ' + Abc;
const AbcWs: String = Abc + ' ';
const WsAbcWs: String = ' ' + Abc + ' ';
const WssAbc: String = Wss + Abc;
const AbcWss: String = Abc + Wss;
const WssAbcWss: String = Wss + Abc + Wss;
const AbcWssAbc: String = Abc + Wss + Abc;
const Esc: String = ' \t\n\v';
const RusChars: String = 'Русские буквы';

const treestring: String = StringBuilder.concatStrings('12345678901234', 'abcdefghijklmn');
const WstreestringWs: String = Ws + treestring + Ws;
const treestringWs: String = treestring + Ws;
const Wstreestring: String = Ws + treestring;

const slicedstring: String = treestring.substring(1, 15);
const WsslicedstringWs: String = Ws + slicedstring + Ws;
const slicedstringWs: String = slicedstring + Ws;
const Wsslicedstring: String = Ws + slicedstring;

function testStringTrim() {
    let wssCount: int = 0;
    for (let i = 0; i <= 0xFFFF; ++i) {
        let c = i.toChar();
        if (Char.isWhiteSpace(c)) {
            ++wssCount;
            arktest.assertNE(Wss.indexOf(c), -1, 'Bad char code is encountered: ' + i);
        }
    }
    arktest.assertEQ(wssCount, Wss.length, 'Wrong number of u16 whitespaces: ' + wssCount);

    // trim
    arktest.assertEQ(Empty.trim(), '');
    arktest.assertEQ(Ws.trim(), Empty);
    arktest.assertEQ(Wss.trim(), Empty);
    arktest.assertEQ(Xy.trim(), Xy);
    arktest.assertEQ(WsXy.trim(), Xy);
    arktest.assertEQ(XyWs.trim(), Xy);
    arktest.assertEQ(WsXyWs.trim(), Xy);
    arktest.assertEQ(Abc.trim(), Abc);
    arktest.assertEQ(WsAbc.trim(), Abc);
    arktest.assertEQ(AbcWs.trim(), Abc);
    arktest.assertEQ(WsAbcWs.trim(), Abc);
    arktest.assertEQ(AwsBwsC.trim(), AwsBwsC);
    arktest.assertEQ(WssAbc.trim(), Abc);
    arktest.assertEQ(AbcWss.trim(), Abc);
    arktest.assertEQ(WssAbcWss.trim(), Abc);
    arktest.assertEQ(AbcWssAbc.trim(), AbcWssAbc);
    arktest.assertEQ(X.trim(), X);
    arktest.assertEQ(WsX.trim(), X);
    arktest.assertEQ(XWs.trim(), X);
    arktest.assertEQ(Z.trim(), Z);
    arktest.assertEQ(WsZ.trim(), Z);
    arktest.assertEQ(ZWs.trim(), Z);
    arktest.assertEQ(Esc.trim(), Empty);
    arktest.assertEQ(RusChars.trim(), RusChars);
    arktest.assertEQ(WstreestringWs.trim(), treestring);
    arktest.assertEQ(WsslicedstringWs.trim(), slicedstring);
    // trimLeft
    arktest.assertEQ(Ws.trimLeft(), Empty);
    arktest.assertEQ(Wss.trimLeft(), Empty);
    arktest.assertEQ(WsAbcWs.trimLeft(), AbcWs);
    arktest.assertEQ(AwsBwsC.trimLeft(), AwsBwsC);
    arktest.assertEQ(WssAbcWss.trimLeft(), AbcWss);
    arktest.assertEQ(X.trimLeft(), X);
    arktest.assertEQ(WsX.trimLeft(), X);
    arktest.assertEQ(XWs.trimLeft(), XWs);
    arktest.assertEQ(Z.trimLeft(), Z);
    arktest.assertEQ(WsZ.trimLeft(), Z);
    arktest.assertEQ(ZWs.trimLeft(), ZWs);
    arktest.assertEQ(Esc.trimLeft(), Empty);
    arktest.assertEQ(RusChars.trimLeft(), RusChars);
    arktest.assertEQ(WstreestringWs.trimLeft(), treestringWs);
    arktest.assertEQ(WsslicedstringWs.trimLeft(), slicedstringWs);
    // trimRight
    arktest.assertEQ(Ws.trimRight(), Empty);
    arktest.assertEQ(Wss.trimRight(), Empty);
    arktest.assertEQ(WsAbcWs.trimRight(), WsAbc);
    arktest.assertEQ(AwsBwsC.trimRight(), AwsBwsC);
    arktest.assertEQ(WssAbcWss.trimRight(), WssAbc);
    arktest.assertEQ(X.trimRight(), X);
    arktest.assertEQ(WsX.trimRight(), WsX);
    arktest.assertEQ(XWs.trimRight(), X);
    arktest.assertEQ(Z.trimRight(), Z);
    arktest.assertEQ(WsZ.trimRight(), WsZ);
    arktest.assertEQ(ZWs.trimRight(), Z);
    arktest.assertEQ(Esc.trimRight(), Empty);
    arktest.assertEQ(RusChars.trimRight(), RusChars);
    arktest.assertEQ(WstreestringWs.trimRight(), Wstreestring);
    arktest.assertEQ(WsslicedstringWs.trimRight(), Wsslicedstring);
}

function main(): int {
    let suite = new arktest.ArkTestsuite('StringTrim');
    suite.addTest('testStringTrim', testStringTrim);
    return suite.run();
}
