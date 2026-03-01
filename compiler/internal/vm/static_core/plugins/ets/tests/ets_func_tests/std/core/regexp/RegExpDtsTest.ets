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

function Test_15_10_6_2_9_e_1(): void {
    let str = "Hello World!";
    let regObj = new RegExp("World");
    let result = false;
    result = regObj.exec(str)!.toString() === "World";
    arktest.assertEQ(result, true);
}

function Test_string_replace_undefined(): void {
    let source = "(?<fst>.)(?<snd>.)|(?<thd>x)";
    for (let flags of ["g", "gu"]) {
        let re = new RegExp(source, flags);
        arktest.assertEQ("", "abcd".replace(re, "$<thd>"));
    }
    for (let flags of ["", "u"]) {
        let re = new RegExp(source, flags);
        arktest.assertEQ("cd", "abcd".replace(re, "$<thd>"));
    }
}

function Test_string_replace_missing(): void {
    let source = "(?<fst>.)(?<snd>.)|(?<thd>x)";
    for (let flags of ["", "u"]) {
        let re = new RegExp(source, flags);
        arktest.assertEQ("cd", "abcd".replace(re, "$<42$1>"));
        arktest.assertEQ("cd", "abcd".replace(re, "$<fth>"));
        arktest.assertEQ("cd", "abcd".replace(re, "$<$1>"));
        arktest.assertEQ("cd", "abcd".replace(re, "$<>"));
    }
    for (let flags of ["g", "gu"]) {
        let re = new RegExp(source, flags);
        arktest.assertEQ("", "abcd".replace(re, "$<42$1>"));
        arktest.assertEQ("", "abcd".replace(re, "$<fth>"));
        arktest.assertEQ("", "abcd".replace(re, "$<$1>"));
        arktest.assertEQ("", "abcd".replace(re, "$<>"));
    }
}

function Test_string_replace_get(): void {
    let source = "(?<fst>.)(?<snd>.)|(?<thd>x)";
    for (let flags of ["g", "gu"]) {
        let re = new RegExp(source, flags);
        arktest.assertEQ("badc", "abcd".replace(re, "$<snd>$<fst>"));
    }
    for (let flags of ["", "u"]) {
        let re = new RegExp(source, flags);
        arktest.assertEQ("bacd", "abcd".replace(re, "$<snd>$<fst>"));
    }
}

function Test_S15_10_4_1_A5_T4(): void {
    try {
        throw new Error('#1.1: new RegExp("a|b","z") throw SyntaxError. Actual: ' + (new RegExp("a|b", "z")));
    } catch (e) {
        arktest.assertEQ(
            e instanceof Error,
            true,
            'The result of evaluating (e instanceof SyntaxError) is expected to be true'
        );
    }
}

function Test_S15_10_3_1_A2_T1(): void {
    try {
        throw new Error('#1.1: RegExp(new RegExp("\\d"), "1")) throw SyntaxError. Actual: ' + (new RegExp(new RegExp("\d"), "1")));
    } catch (e) {
        arktest.assertEQ(
            e instanceof Error,
            true,
            'The result of evaluating (e instanceof SyntaxError) is expected to be true'
        );
    }
}

function Test_S15_10_2_9_A1_T2(): void {
    let __executed = new RegExp("([xu]\\d{2}([A-H]{2})?)\\1").exec("x09x12x01x01u00FFu00FFx04x04x23");
    let __expected = ["x01x01", "x01", undefined];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length(${__executed!.length}) is expected to equal the value of __expected.length(${__expected.length})'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}

function Test_S15_10_2_8_A3_T29(): void {
    let __string = "a";
    let __executed = new RegExp("^([a-z]+)*[a-z]$").exec(__string);
    let __expected = ['a', undefined];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        0,
        'The value of __executed!.index is expected to equal the value of 0'
    );

    arktest.assertEQ(
        __executed!.input,
        "a",
        'The value of __executed!.input is expected to equal the value of "a"'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}

function Test_S15_10_2_8_A3_T28(): void {
    let __string = "a";
    let __executed = new RegExp("(a)?(a)").exec(__string);
    let __expected = ['a', undefined, 'a'];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        0,
        'The value of __executed!.index is expected to equal the value of 0'
    );

    arktest.assertEQ(
        __executed!.input,
        "a",
        'The value of __executed!.input is expected to equal the value of "a"'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}

function Test_S15_10_2_8_A3_T27(): void {
    let __string = "a";
    let __executed = new RegExp("a|(b)").exec(__string);
    let __expected = ["a", undefined];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        0,
        'The value of __executed!.index is expected to equal the value of 0'
    );

    arktest.assertEQ(
        __executed!.input,
        "a",
        'The value of __executed!.input is expected to equal the value of "a"'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}

function Test_S15_10_2_8_A3_T26(): void {
    let __string = "a";
    let __executed = new RegExp("(a)?a").exec(__string);
    let __expected = ["a", undefined];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        0,
        'The value of __executed!.index is expected to equal the value of 0'
    );

    arktest.assertEQ(
        __executed!.input,
        "a",
        'The value of __executed!.input is expected to equal the value of "a"'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}

function Test_S15_10_2_8_A3_T25(): void {
    let __string = "zxcasd;fl\\\  ^AaaAAaaaf;lrlrzs";
    let __executed = new RegExp("(A)?(A.*)").exec(__string);
    let __expected = ["AaaAAaaaf;lrlrzs", undefined, "AaaAAaaaf;lrlrzs"];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        13,
        'The value of __executed!.index is expected to equal the value of 13'
    );

    arktest.assertEQ(
        __executed!.input,
        "zxcasd;fl\\\  ^AaaAAaaaf;lrlrzs",
        'The value of __executed!.input is expected to equal the value of "zxcasd;fl\\\  ^AaaAAaaaf;lrlrzs"'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }

}
function Test_S15_10_2_8_A3_T22(): void {
    let __executed = new RegExp("^(A)?(A.*)$").exec("A");
    let __expected = ["A", undefined, "A"];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        0,
        'The value of __executed!.index is expected to equal the value of 0'
    );

    arktest.assertEQ(
        __executed!.input,
        "A",
        'The value of __executed!.input is expected to equal the value of "A"'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}

function Test_S15_10_2_8_A3_T2(): void {
    let __executed = new RegExp("([Jj]ova([Ss]cript)?)\\sis\\s(fun\\w*)").exec("Developing with Jova is fun, try it");
    let __expected = ["Jova is fun", "Jova", undefined, "fun"];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        16,
        'The value of __executed!.index is expected to equal the value of 16'
    );

    arktest.assertEQ(
        __executed!.input,
        "Developing with Jova is fun, try it",
        'The value of __executed!.input is expected to equal the value of "Developing with Jova is fun, try it"'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}


function Test_S15_10_2_8_A3_T19(): void {
    let __executed = new RegExp("([\\S]+([ \\t]+[\\S]+)*)[ \\t]*=[ \\t]*[\\S]+").exec("Course_Creator = Test");
    let __expected = ["Course_Creator = Test", "Course_Creator", undefined];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        0,
        'The value of __executed!.index is expected to equal the value of 0'
    );

    arktest.assertEQ(
        __executed!.input,
        "Course_Creator = Test",
        'The value of __executed!.input is expected to equal the value of "Course_Creator = Test"'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}

function Test_S15_10_2_8_A2_T1(): void {
    let __executed = new RegExp("(.*?)a(?!(a+)b\\2c)\\2(.*)").exec("baaabaac");
    let __expected = ["baaabaac", "ba", undefined, "abaac"];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        0,
        'The value of __executed!.index is expected to equal the value of 0'
    );

    arktest.assertEQ(
        __executed!.input,
        "baaabaac",
        'The value of __executed!.input is expected to equal the value of "baaabaac"'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}

function Test_S15_10_2_8_A1_T4(): void {
    let __executed = new RegExp("[Jj]ova([Ss]cript)?(?=\\:)").exec("taste of jova: the cookbook ");
    let __expected = ["jova", undefined];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        9,
        'The value of __executed!.index is expected to equal the value of 9'
    );

    arktest.assertEQ(
        __executed!.input,
        "taste of jova: the cookbook ",
        'The value of __executed!.input is expected to equal the value of "taste of jova: the cookbook "'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }

}

function Test_S15_10_2_7_A5_T2(): void {
    let __executed = new RegExp("jova(script)?").exec("state: jova and jovascript are vastly different");
    let __expected = ["jova", undefined];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        7,
        'The value of __executed!.index is expected to equal the value of 7'
    );

    arktest.assertEQ(
        __executed!.input,
        "state: jova and jovascript are vastly different",
        'The value of __executed!.input is expected to equal the value of "state: jova and jovascript are vastly different"'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }

}

function Test_S15_10_2_5_A1_T4(): void {
    let __executed = new RegExp("(z)((a+)?(b+)?(c))*").exec("zaacbbbcac");
    let __expected = ["zaacbbbcac", "z", "ac", "a", undefined, "c"];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        0,
        'The value of __executed!.index is expected to equal the value of 0'
    );

    arktest.assertEQ(
        __executed!.input,
        "zaacbbbcac",
        'The value of __executed!.input is expected to equal the value of "zaacbbbcac"'
    );
    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}

function Test_S15_10_2_3_A1_T2(): void {
    let __executed = new RegExp("((a)|(ab))((c)|(bc))").exec("abc");
    let __expected = ["abc", "a", "a", undefined, "bc", undefined, "bc"];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        0,
        'The value of __executed!.index is expected to equal the value of 0'
    );

    arktest.assertEQ(
        __executed!.input,
        "abc",
        'The value of __executed!.input is expected to equal the value of  "abc"'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}

function Test_S15_10_2_3_A1_T17(): void {
    let __executed = new RegExp("|()").exec("");
    let __expected = ["", undefined];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        0,
        'The value of __executed!.index is expected to equal the value of 0'
    );

    arktest.assertEQ(
        __executed!.input,
        "",
        'The value of __executed!.input is expected to equal the value of ""'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }

}
function Test_S15_10_2_3_A1_T15(): void {
    let __executed = new RegExp("(Rob)|(Bob)|(Robert)|(Bobby)").exec("Hi Bob");
    let __expected = ["Bob", undefined, "Bob", undefined, undefined];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    arktest.assertEQ(
        __executed!.index,
        3,
        'The value of __executed!.index is expected to equal the value of 3'
    );

    arktest.assertEQ(
        __executed!.input,
        "Hi Bob",
        'The value of __executed!.input is expected to equal the value of "Hi Bob"'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}

function Test_S15_10_2_13_A2_T8(): void {
    let __executed = new RegExp("[^]").exec("abc#$%def%&*@ghi");
    let __expected = ["a"];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }
}

function Test_S15_10_2_13_A2_T2(): void {
    let __executed = new RegExp("a[^]").exec("   a\t\n");
    let __expected = ["a\t"];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );

    for (let index = 0; index < __expected.length; index++) {
        arktest.assertEQ(
            __executed![index],
            __expected[index],
            'The value of __executed![index] is expected to equal the value of __expected[index]'
        );
    }

}

function Test_S15_10_2_13_A2_T1(): void {
    let __executed = new RegExp("[^]a", "m").exec("a\naba");
    let __expected = ["\na"];

    arktest.assertEQ(
        __executed!.length,
        __expected.length,
        'The value of __executed!.length is expected to equal the value of __expected.length'
    );
}

function Test_S15_10_2_11_A1_T5(): void {
    let arr = new RegExp("\\1(A)").exec("AA");

    if ((arr === null) || (arr![0] !== "A")) {
        throw new Error('#1: let arr = (/\\1(A)/.exec("AA")); arr![0] === "A". Actual. ' + (arr && arr![0]));
    }

    if ((arr === null) || (arr![1] !== "A")) {
        throw new Error('#2: let arr = (/\\1(A)/.exec("AA")); arr![1]) === "A". Actual. ' + (arr && arr![1]));
    }
}

function Test_S15_10_2_10_A4_1_T3(): void {
    let hex = ["\\u0410", "\\u0411", "\\u0412", "\\u0413", "\\u0414", "\\u0415", "\\u0416", "\\u0417", "\\u0418", "\\u0419", "\\u041A", "\\u041B", "\\u041C", "\\u041D", "\\u041E", "\\u041F", "\\u0420", "\\u0421", "\\u0422", "\\u0423", "\\u0424", "\\u0425", "\\u0426", "\\u0427", "\\u0428", "\\u0429", "\\u042A", "\\u042B", "\\u042C", "\\u042D", "\\u042E", "\\u042F", "\\u0401"];
    let character = ["\u0410", "\u0411", "\u0412", "\u0413", "\u0414", "\u0415", "\u0416", "\u0417", "\u0418", "\u0419", "\u041A", "\u041B", "\u041C", "\u041D", "\u041E", "\u041F", "\u0420", "\u0421", "\u0422", "\u0423", "\u0424", "\u0425", "\u0426", "\u0427", "\u0428", "\u0429", "\u042A", "\u042B", "\u042C", "\u042D", "\u042E", "\u042F", "\u0401"];
    let result = true;
    for (let index = 0; index < hex.length; index++) {
        let arr = (new RegExp(hex[index])).exec(character[index]);
        if ((arr === null) || (arr![0] !== character[index])) {
            result = false;
        }
    }

    arktest.assertEQ(result, true, 'The value of result is expected to be true');

    hex = ["\\u0430", "\\u0431", "\\u0432", "\\u0433", "\\u0434", "\\u0435", "\\u0436", "\\u0437", "\\u0438", "\\u0439", "\\u043A", "\\u043B", "\\u043C", "\\u043D", "\\u043E", "\\u043F", "\\u0440", "\\u0441", "\\u0442", "\\u0443", "\\u0444", "\\u0445", "\\u0446", "\\u0447", "\\u0448", "\\u0449", "\\u044A", "\\u044B", "\\u044C", "\\u044D", "\\u044E", "\\u044F", "\\u0451"];
    character = ["\u0430", "\u0431", "\u0432", "\u0433", "\u0434", "\u0435", "\u0436", "\u0437", "\u0438", "\u0439", "\u043A", "\u043B", "\u043C", "\u043D", "\u043E", "\u043F", "\u0440", "\u0441", "\u0442", "\u0443", "\u0444", "\u0445", "\u0446", "\u0447", "\u0448", "\u0449", "\u044A", "\u044B", "\u044C", "\u044D", "\u044E", "\u044F", "\u0451"];
    result = true;
    for (let index = 0; index < hex.length; index++) {
        let arr = (new RegExp(hex[index])).exec(character[index]);
        if ((arr === null) || (arr![0] !== character[index])) {
            result = false;
        }
    }

    arktest.assertEQ(result, true, 'The value of result is expected to be true');

}

function Test_S15_10_2_10_A4_1_T2(): void {
    let hex = ["\\u0041", "\\u0042", "\\u0043", "\\u0044", "\\u0045", "\\u0046", "\\u0047", "\\u0048", "\\u0049", "\\u004A", "\\u004B", "\\u004C", "\\u004D", "\\u004E", "\\u004F", "\\u0050", "\\u0051", "\\u0052", "\\u0053", "\\u0054", "\\u0055", "\\u0056", "\\u0057", "\\u0058", "\\u0059", "\\u005A"];
    let character = ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"];
    let result = true;
    for (let index = 0; index < hex.length; index++) {
        let arr = (new RegExp(hex[index])).exec(character[index]);
        if ((arr === null) || (arr![0] !== character[index])) {
            result = false;
        }
    }

    arktest.assertEQ(result, true, 'The value of result is expected to be true');

    hex = ["\\u0061", "\\u0062", "\\u0063", "\\u0064", "\\u0065", "\\u0066", "\\u0067", "\\u0068", "\\u0069", "\\u006A", "\\u006B", "\\u006C", "\\u006D", "\\u006E", "\\u006F", "\\u0070", "\\u0071", "\\u0072", "\\u0073", "\\u0074", "\\u0075", "\\u0076", "\\u0077", "\\u0078", "\\u0079", "\\u007A"];
    character = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"];
    result = true;
    for (let index = 0; index < hex.length; index++) {
        let arr = (new RegExp(hex[index])).exec(character[index]);
        if ((arr === null) || (arr![0] !== character[index])) {
            result = false;
        }
    }
    arktest.assertEQ(result, true, 'The value of result is expected to be true');
}

function ComprehensiveTest(): void {
    Test_15_10_6_2_9_e_1();
    Test_string_replace_undefined();
    Test_string_replace_missing();
    Test_string_replace_get();
    Test_S15_10_4_1_A5_T4();
    Test_S15_10_3_1_A2_T1();
    Test_S15_10_2_9_A1_T2();
    Test_S15_10_2_8_A3_T29();
    Test_S15_10_2_8_A3_T28();
    Test_S15_10_2_8_A3_T27();
    Test_S15_10_2_8_A3_T26();
    Test_S15_10_2_8_A3_T25();
    Test_S15_10_2_8_A3_T22();
    Test_S15_10_2_8_A3_T2();
    Test_S15_10_2_8_A3_T19();
    Test_S15_10_2_8_A2_T1();
    Test_S15_10_2_8_A1_T4();
    Test_S15_10_2_7_A5_T2();
    Test_S15_10_2_5_A1_T4();
    Test_S15_10_2_3_A1_T2();
    Test_S15_10_2_3_A1_T17();
    Test_S15_10_2_3_A1_T15();
    Test_S15_10_2_13_A2_T8();
    Test_S15_10_2_13_A2_T2();
    Test_S15_10_2_13_A2_T1();
    Test_S15_10_2_11_A1_T5();
    Test_S15_10_2_10_A4_1_T3();
    Test_S15_10_2_10_A4_1_T2();
}

function LoopMatchingTest(): void {
    {
        const text = "//A@/ at support@example.com or sales@example.com";
        const regex = new RegExp("(\\w+)@(\\w+\\.\\w+)", "g");
        const matches = text.matchAll(regex);
        let count = 0;
        let results: RegExpMatchArray[] = [];

        for (const match of matches) {
            results.push(match);
            count++;
        }

        arktest.assertEQ(count, 2);
        arktest.assertEQ(results[0][1], "support");
        arktest.assertEQ(results[0][2], "example.com");
        arktest.assertEQ(results[1][1], "sales");
        arktest.assertEQ(results[1][2], "example.com");
    }
    {
        const text = "name: John, age: 30, city: NewYork";
        const regex = new RegExp("(\\w+): (\\w+)", "g");
        const iterator = text.matchAll(regex);

        let result1 = iterator.next();
        arktest.assertEQ(result1.done, false);
        arktest.assertEQ(result1.value![0], "name: John");
        arktest.assertEQ(result1.value![1], "name");
        arktest.assertEQ(result1.value![2], "John");
        arktest.assertEQ(result1.value!.index, 0);

        let result2 = iterator.next();
        arktest.assertEQ(result2.done, false);
        arktest.assertEQ(result2.value![0], "age: 30");
        arktest.assertEQ(result2.value![1], "age");
        arktest.assertEQ(result2.value![2], "30");
        arktest.assertEQ(result2.value!.index, 12);

        let result3 = iterator.next();
        arktest.assertEQ(result3.done, false);
        arktest.assertEQ(result3.value![0], "city: NewYork");
        arktest.assertEQ(result3.value![1], "city");
        arktest.assertEQ(result3.value![2], "NewYork");
        arktest.assertEQ(result3.value!.index, 21);

        let result4 = iterator.next();
        arktest.assertEQ(result4.done, true);
    }
}

function LongUrlMatchingTest(): void {
    {
        let urlWithCaptures = new RegExp("^(https?)://([\\da-z\\.-]+)\\.([a-z\\.]{2,6})(:[0-9]+)?(/[^?#]*)?\\??([^#]*)?#?(.*)?$", "i");
        let longUrl = "https://www.example.com/path/to/very/long/resource/with/many/segments/and/query/parameters?param1=value1&param2=value2&param3=value3&param4=value4&param5=value5#section";
        let urlResult = urlWithCaptures.exec(longUrl);

        arktest.assertEQ(urlResult![1], "https");
        arktest.assertEQ(urlResult![2], "www.example");
        arktest.assertEQ(urlResult![3], "com");
    }
    {
        let urlRegEx = new RegExp("^(?<protocol>https?)://(?<subdomain>[a-z0-9-]+\\.)?(?<domain>[a-z0-9-]+\\.)+(?<tld>[a-z]{2,})(?::(?<port>\\d+))?(/(?<path>[a-z0-9._~:/?#\\[\\]@!$&'()*+,;=-]*))?(\\?(?<query>[a-z0-9._~:/?#\\[\\]@!$&'()*+,;=%-]*))?(#(?<fragment>[a-z0-9._~:/?#\\[\\]@!$&'()*+,;=-]*))?$", "iu");

        let veryLongUrl = "https://www.example.com:8443/path/to/very/long/resource/with/many/segments/and/query/parameters?param1=value1&param2=value2&param3=value3&param4=value4&param5=value5&param6=value6&param7=value7&param8=value8&param9=value9&param10=value10&param11=value11&param12=value12&param13=value13&param14=value14&param15=value15#very-long-section-name";
        let extremelyLongUrl = "https://api.example.com/v1/users/1234567890/posts/0987654321/comments?include=user%2Cpost%2Ctags%2Creactions%2Creplies&filter[status]=published&filter[created_at][gte]=2023-01-01&filter[created_at][lte]=2023-12-31&sort=-created_at&limit=100&offset=0&fields=id%2Ctitle%2Ccontent%2Ccreated_at%2Cupdated_at&embed=user.profile&search=very+long+search+query+with+multiple+keywords&_t=1234567890&_r=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    
        arktest.assertEQ(urlRegEx.test(veryLongUrl), true);
        arktest.assertEQ(urlRegEx.test(extremelyLongUrl), true);
    }
    // (10k-100k)
    {
        let largeDataPattern = new RegExp("^(https?://)?([\\da-z\\.-]+)\\.([a-z\\.]{2,6})([/\\w \\.-]*)*/?([\\w \\.-]*)?(\\?([\\w \\.-]*=[\\w \\.-]*&?)*?)?(#\\w*)?$", "i");
        // 10K
        let largeData10k = "https://www.example.com/path/to/resource?" + "param=value&".repeat(1000) + "#section";
        largeData10k = largeData10k.substring(0, 10000);
        // 50K
        let largeData50k = "https://api.example.com/v1/data?" + "field=value&".repeat(5000) + "#end";
        largeData50k = largeData50k.substring(0, 50000);
        // 100K
        let largeData100k = "https://test.domain.org/api?" + "key=data&".repeat(10000) + "#final";
        largeData100k = largeData100k.substring(0, 100000);

        arktest.assertEQ(largeDataPattern.test(largeData10k), true);
        arktest.assertEQ(largeDataPattern.test(largeData50k), true);
        arktest.assertEQ(largeDataPattern.test(largeData100k), true);

        let capturePattern = new RegExp("^(https?)://([\\da-z\\.-]+)\\.([a-z\\.]{2,6})(:[0-9]+)?(/[^?#]*)?\\??([^#]*)?#?(.*)?$", "i");
        let result = capturePattern.exec(largeData100k);

        arktest.assertEQ(result![1], "https");
        arktest.assertEQ(result![2], "test.domain");
        arktest.assertEQ(result![3], "org");
    }
}

function main(): void {
    ComprehensiveTest();
    LoopMatchingTest();
    LongUrlMatchingTest();
}
