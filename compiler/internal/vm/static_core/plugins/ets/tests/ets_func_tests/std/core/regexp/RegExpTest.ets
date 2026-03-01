/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let str1: string = "";
let resultTest: string = "";
class CharNumGroups {
    charname: string = 'name';
    num: string = '0';
}

function createHaystack(): string {
    let s = "abCdefgz";
    for (let i = 0; i < 3; i++) {
        s += s;
    }
    return s;
}

function replaceTest(): void {
    const str10: string = 'A1B2';
    const regex10 = new RegExp('(?<charname>[A-Z])(?<num>\\d)', 'g');
    const result10: string = str10.replaceAll(
        regex10,
        (
            match: string,
            args: Object[],
        ): string => {
            const groups = args[4] as Record<string, string>
            const myGroups: CharNumGroups = {
                charname: groups.get('charname')!,
                num: groups.get('num')!
            }
            return `${myGroups.num}${myGroups.charname}`;
        }
    ); 
    arktest.assertEQ(result10, '1A2B');

    const str11: string = 'A1';
    const regex11 = new RegExp('([A-Z])(\\d)', 'g');
    const result11: string = str11.replaceAll(
        regex11,
        (
            _: string,
            args: Object[]
        ): String => {
        const charGroup = args[0] as String;
        const numGroup = args[1] as String;
        const dataToConvert: CharNumGroups = { num: numGroup, charname: charGroup  };
        return JSON.stringify(dataToConvert as CharNumGroups ) ;
    });
    let typ = Class.of(new CharNumGroups())
    const res: CharNumGroups = JSON.parse<CharNumGroups>(result11, typ) as CharNumGroups
    arktest.assertEQ(res.num, "1");
    arktest.assertEQ(res.charname, "A");
}


function String1Replace(re: RegExp): void {
    resultTest = str1.replace(re, ".");
}

function String2Replace(re: RegExp): void {
    resultTest = str1.replace(re, "xyz");
}

function String3Replace(re: RegExp): void {
    resultTest = str1.replace(re, "x$1yz");
}

function Function1Replace(re: RegExp): void {
    resultTest = str1.replace(re, (match: string) => match);
}

function verifyRegExp(text: string): string[] {
    text = text.replace(new RegExp("[\\s]", "g"), ' ');
    let emRegExp = new RegExp("<em>.*?</em>", "ig");
    let results = text.split(emRegExp);
    text.match(emRegExp);
    return results;
}

function GreekAlphabetTestFunction06(): void {
    let quotedEmailUserUtf8 = new RegExp("^([\\s\\x01-\\x08\\x0b\\x0c\\x0e-\\x1f\\x7f\\x21\\x23-\\x5b\\x5d-\\x7e\\u00A0-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFEF]|(\\\\[\\x01-\\x09\\x0b\\x0c\\x0d-\\x7f\\u00A0-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFEF]))*$", "i");
    arktest.assertEQ(quotedEmailUserUtf8.test(" foo m端ller "), true)
}

function GreekAlphabetTestFunction07(): void {
    let reg = new RegExp("^[Α-ώ]+$", "iu");
    arktest.assertEQ(reg.test('άέήίΰϊϋόύώ'), true)
    arktest.assertEQ(reg.test('ΆΈΉΊΪΫΎΏ'), true)
    arktest.assertEQ(reg.test('αβγδεζηθικλμνξοπρςστυφχψω'), true)
    arktest.assertEQ(reg.test('ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ'), true)
}

function GreekAlphabetTestFunction08(): void {
    let reg = new RegExp("^[A-ZÃÁÀÂÄÇÉÊËÍÏÕÓÔÖÚÜ]+$", "iu");
    arktest.assertEQ(reg.test('palíndromo'), true)
    arktest.assertEQ(reg.test('órgão'), true)
    arktest.assertEQ(reg.test('qwértyúão'), true)
    arktest.assertEQ(reg.test('àäãcëüïÄÏÜ'), true)
}

function GreekAlphabetTestFunction09(): void {
    let reg = new RegExp("^[A-ZÀÉÈÌÎÓÒÙ]+$", "iu");
    arktest.assertEQ(reg.test('àéèìîóòù'), true)
    arktest.assertEQ(reg.test('metró'), true)
    arktest.assertEQ(reg.test('pèsca'), true)
    arktest.assertEQ(reg.test('genî'), true)
}

function GreekAlphabetTestFunction10(): void {
    let reg = new RegExp("^[A-ZÀÁẠẢÃÂẦẤẬẨẪĂẰẮẶẲẴĐÈÉẸẺẼÊỀẾỆỂỄÌÍỊỈĨÒÓỌỎÕÔỒỐỘỔỖƠỜỚỢỞỠÙÚỤỦŨƯỪỨỰỬỮỲÝỴỶỸ]+$", "iu");
    arktest.assertEQ(reg.test('thiến'), true)
    arktest.assertEQ(reg.test('nghiêng'), true)
    arktest.assertEQ(reg.test('chào'), true)
    arktest.assertEQ(reg.test('thế'), true)
    arktest.assertEQ(reg.test('giới'), true)
}

function GreekAlphabetTestFunction11(): void {
    let reg = new RegExp("^[A-ZÅÄÖ]+$", "iu");
    arktest.assertEQ(reg.test('äiti'), true)
}

function GreekAlphabetTestFunction12(): void {
    let reg = new RegExp("^[A-ZÆØÅ]+$", "iu");
    arktest.assertEQ(reg.test('aøå'), true)
}

function GreekAlphabetTestFunction13(): void {
    let reg = new RegExp("^[A-ZĄĆĘŚŁŃÓŻŹ]+$", "iu");
    arktest.assertEQ(reg.test('kreską'), true)
    arktest.assertEQ(reg.test('zamknięte'), true)
    arktest.assertEQ(reg.test('zwykłe'), true)
    arktest.assertEQ(reg.test('kropką'), true)
    arktest.assertEQ(reg.test('przyjęły'), true)
    arktest.assertEQ(reg.test('święty'), true)
    arktest.assertEQ(reg.test('Pozwól'), true)
}

function GreekAlphabetTestFunction14(): void {
    let reg = new RegExp("^[А-ЯЂЈЉЊЋЏ]+$", "iu");
    arktest.assertEQ(reg.test('ШћжЂљЕ'), true)
}

function GreekAlphabetTestFunction15(): void {
    let reg = new RegExp("^[A-ZČĆŽŠĐ]+$", "iu");
    arktest.assertEQ(reg.test('ŠAabčšđćž'), true)
    arktest.assertEQ(reg.test('ŠATROĆčđš'), true)
}

function GreekAlphabetTestFunction16(): void {
    let reg = new RegExp("^[A-ZÁÉÍÑÓÚÜ]+$", "iu");
    arktest.assertEQ(reg.test('ábcó'), true)
    arktest.assertEQ(reg.test('dormís'), true)
    arktest.assertEQ(reg.test('volvés'), true)
    arktest.assertEQ(reg.test('español'), true)
}

function GreekAlphabetTestFunction17(): void {
    let reg = new RegExp("^[A-ZÅÄÖ]+$", "iu");
    arktest.assertEQ(reg.test('religiös'), true)
    arktest.assertEQ(reg.test('stjäla'), true)
    arktest.assertEQ(reg.test('västgöte'), true)
}

function GreekAlphabetTestFunction18(): void {
    let reg = new RegExp("^[A-ZÇĞİıÖŞÜ]+$", "iu");
    arktest.assertEQ(reg.test('AİıÖöÇçŞşĞğÜüZ'), true)
}

function GreekAlphabetTestFunction19(): void {
    let reg = new RegExp("^[Α-ώ]+$", "iu");
    arktest.assertEQ(reg.test('άέήίΰϊϋόύώ'), true)
    arktest.assertEQ(reg.test('ΆΈΉΊΪΫΎΏ'), true)
}

function GreekAlphabetTestFunction20(): void {
    let reg = new RegExp("^[0-9A-VXYZÇƏĞİıÖŞÜ]+$", "iu");
    arktest.assertEQ(reg.test('Azərbaycan'), true)
    arktest.assertEQ(reg.test('abcç2'), true)
    arktest.assertEQ(reg.test('3kərə4kərə'), true)
}

function GreekAlphabetTestFunction21(): void {
    let reg = new RegExp("^[0-9А-Я]+$", "iu");
    arktest.assertEQ(reg.test('абв1'), true)
    arktest.assertEQ(reg.test('жаба'), true)
    arktest.assertEQ(reg.test('яГоДа2'), true)
    arktest.assertEQ(reg.test('йЮя'), true)
}

function GreekAlphabetTestFunction22(): void {
    let reg = new RegExp("^[0-9A-ZÁČĎÉĚÍŇÓŘŠŤÚŮÝŽ]+$", "iu");
    arktest.assertEQ(reg.test('řiť123'), true)
}

function GreekAlphabetTestFunction23(): void {
    let reg = new RegExp("^[0-9A-ZÁČĎÉÍŇÓŠŤÚÝŽĹŔĽÄÔ]+$", "iu");
    arktest.assertEQ(reg.test('1môj'), true)
    arktest.assertEQ(reg.test('2ľúbím'), true)
    arktest.assertEQ(reg.test('3mäkčeň'), true)
    arktest.assertEQ(reg.test('5vŕba'), true)
    arktest.assertEQ(reg.test('6ňorimberk'), true)
    arktest.assertEQ(reg.test('7ťava'), true)
    arktest.assertEQ(reg.test('8žanéta'), true)
    arktest.assertEQ(reg.test('9Ďábelské'), true)
    arktest.assertEQ(reg.test('10ódy'), true)
}

function GreekAlphabetTestFunction24(): void {
    let reg = new RegExp("^[0-9A-ZÁÉËÏÓÖÜÚ]+$", "iu");
    arktest.assertEQ(reg.test('Kán123'), true)
    arktest.assertEQ(reg.test('één354'), true)
}

function GreekAlphabetTestFunction25(): void {
    let reg = new RegExp("^[0-9A-ZÅÄÖ]+$", "iu");
    arktest.assertEQ(reg.test('äiti124'), true)
    arktest.assertEQ(reg.test('451åå23'), true)
}

function GreekAlphabetTestFunction26(): void {
    let reg = new RegExp("^[0-9A-ZÄÖÜß]+$", "iu");
    arktest.assertEQ(reg.test('äbc123'), true)
}

function GreekAlphabetTestFunction27(): void {
    let reg = new RegExp("^[0-9A-ZÁÉÍÓÖŐÚÜŰ]+$", "iu");
    arktest.assertEQ(reg.test('0árvíztűrőtükörfúrógép123'), true)

}
function GreekAlphabetTestFunction28(): void {
    let reg = new RegExp("^[0-9A-ZÃÁÀÂÄÇÉÊËÍÏÕÓÔÖÚÜ]+$", "iu");
    arktest.assertEQ(reg.test('palíndromo'), true)
    arktest.assertEQ(reg.test('2órgão'), true)
    arktest.assertEQ(reg.test('qwértyúão9'), true)
    arktest.assertEQ(reg.test('àäãcë4üïÄÏÜ'), true)
}

function GreekAlphabetTestFunction29(): void {
    let reg = new RegExp("^[0-9A-ZÀÉÈÌÎÓÒÙ]+$", "iu");
    arktest.assertEQ(reg.test('123àéèìîóòù'), true)
    arktest.assertEQ(reg.test('met23ró'), true)
    arktest.assertEQ(reg.test('pès56ca'), true)
    arktest.assertEQ(reg.test('gen45î'), true)
}

function GreekAlphabetTestFunction30(): void {
    let reg = new RegExp("^[0-9A-ZÁÉÍÑÓÚÜ]+$", "iu");
    arktest.assertEQ(reg.test('ábcó123'), true)
}

function GreekAlphabetTestFunction31(): void {
    let reg = new RegExp("^[0-9A-ZÀÁẠẢÃÂẦẤẬẨẪĂẰẮẶẲẴĐÈÉẸẺẼÊỀẾỆỂỄÌÍỊỈĨÒÓỌỎÕÔỒỐỘỔỖƠỜỚỢỞỠÙÚỤỦŨƯỪỨỰỬỮỲÝỴỶỸ]+$", "iu");
    arktest.assertEQ(reg.test('Thầy3'), true)
    arktest.assertEQ(reg.test('3Gà'), true)
}

function GreekAlphabetTestFunction32(): void {
    let reg = new RegExp("^[0-9A-ZĄĆĘŚŁŃÓŻŹ]+$", "iu");
    arktest.assertEQ(reg.test('kre123ską'), true)
    arktest.assertEQ(reg.test('zam21knięte'), true)
    arktest.assertEQ(reg.test('zw23ykłe'), true)
    arktest.assertEQ(reg.test('prz23yjęły'), true)
    arktest.assertEQ(reg.test('świ23ęty'), true)
    arktest.assertEQ(reg.test('Poz1322wól'), true)
}

function GreekAlphabetTestFunction33(): void {
    let reg = new RegExp("^[0-9А-ЯЂЈЉЊЋЏ]+$", "iu");
    arktest.assertEQ(reg.test('ШћжЂљЕ123'), true)
}

function GreekAlphabetTestFunction34(): void {
    let reg = new RegExp("^[0-9A-ZČĆŽŠĐ]+$", "iu");
    arktest.assertEQ(reg.test('ŠAabčšđćž123'), true)
    arktest.assertEQ(reg.test('ŠATRO11Ćčđš'), true)
}

function GreekAlphabetTestFunction35(): void {
    let reg = new RegExp("^[0-9A-ZÅÄÖ]+$", "iu");
    arktest.assertEQ(reg.test('religiös13'), true)
    arktest.assertEQ(reg.test('st23jäla'), true)
    arktest.assertEQ(reg.test('västgöte123'), true)
}

function GreekAlphabetTestFunction36(): void {
    let reg = new RegExp("^[0-9A-ZÇĞİıÖŞÜ]+$", "iu");
    arktest.assertEQ(reg.test('AİıÖöÇçŞşĞğÜüZ123'), true)
}

function GreekAlphabetTestFunction37(): void {
    let reg = new RegExp("^[-+]?([0-9]+)?(\\٫[0-9]{1,})?$");
    arktest.assertEQ(reg.test('0٫0000000000001'), true)
}

function GreekAlphabetTestFunction38(): void {
    let reg = new RegExp("^(?:[-+])?(?:[0-9]+)?(?:\\٫[0-9]*)?(?:[eE][\\+\\-]?(?:[0-9]+))?$");
    arktest.assertEQ(reg.test('123٫'), true)
    arktest.assertEQ(reg.test('123٫123'), true)
    arktest.assertEQ(reg.test('-123٫123'), true)
}

function GreekAlphabetTestFunction39(): void {
    let reg = new RegExp("^[A-Z0-9_\\-]*$", "iu");
    arktest.assertEQ(reg.test(''), true)
}

function GreekAlphabetTestFunction40(): void {
    let reg = new RegExp("^(?!-? )(?=.*\\d)(\\¥)?-?(0|[1-9]\\d|[1-9]\\d{0,2}(\\,\\d{3})*)?(\\.(\\d{2}))?$");
    arktest.assertEQ(reg.test('¥6,954,231'), true)
    arktest.assertEQ(reg.test('¥-6,954,231'), true)
}

function GreekAlphabetTestFunction41(): void {
    let reg = new RegExp("^[A-VXYZÇƏĞİıÖŞÜ]+$", "iu");
    arktest.assertEQ(reg.test('Azərbaycan'), true)
    arktest.assertEQ(reg.test('üöğıəçş'), true)
    arktest.assertEQ(reg.test('sizAzərbaycanlaşdırılmışlardansınızmı'), true)
    arktest.assertEQ(reg.test('dahaBirDüzgünString'), true)
    arktest.assertEQ(reg.test('abcçdeəfgğhxıijkqlmnoöprsştuüvyz'), true)
}

function GreekAlphabetTestFunction42(): void {
    let reg = new RegExp("^[А-Я]+$", "iu");
    arktest.assertEQ(reg.test('абв'), true)
    arktest.assertEQ(reg.test('жаба'), true)
    arktest.assertEQ(reg.test('яГоДа'), true)
}

function GreekAlphabetTestFunction43(): void {
    let reg = new RegExp("^[A-ZÁČĎÉĚÍŇÓŘŠŤÚŮÝŽ]+$", "iu");
    arktest.assertEQ(reg.test('žluťoučký'), true)
    arktest.assertEQ(reg.test('Pěl'), true)
    arktest.assertEQ(reg.test('Ďábelské'), true)
    arktest.assertEQ(reg.test('ódy'), true)
}

function GreekAlphabetTestFunction44(): void {
    let reg = new RegExp("^[A-ZÁČĎÉÍŇÓŠŤÚÝŽĹŔĽÄÔ]+$", "iu");
    arktest.assertEQ(reg.test('môj'), true)
    arktest.assertEQ(reg.test('ľúbím'), true)
    arktest.assertEQ(reg.test('mäkčeň'), true)
    arktest.assertEQ(reg.test('vŕba'), true)
    arktest.assertEQ(reg.test('ňorimberk'), true)
}

function GreekAlphabetTestFunction45(): void {
    let reg = new RegExp("^[A-ZÆØÅ]+$", "iu");
    arktest.assertEQ(reg.test('aøå'), true)
}

function GreekAlphabetTestFunction46(): void {
    let reg = new RegExp("^[A-ZÁÉËÏÓÖÜÚ]+$", "iu");
    arktest.assertEQ(reg.test('Kán'), true)
    arktest.assertEQ(reg.test('één'), true)
    arktest.assertEQ(reg.test('vóór'), true)
    arktest.assertEQ(reg.test('nú'), true)
    arktest.assertEQ(reg.test('héél'), true)
}

function GreekAlphabetTestFunction47(): void {
    let reg = new RegExp("^[A-ZÄÖÜß]+$", "iu");
    arktest.assertEQ(reg.test('äbc'), true)
    arktest.assertEQ(reg.test('FöÖbär'), true)
}

function GreekAlphabetTestFunction48(): void {
    let reg = new RegExp("^[A-ZÁÉÍÓÖŐÚÜŰ]+$", "iu");
    arktest.assertEQ(reg.test('árvíztűrőtükörfúrógép'), true)
}

function GreekAlphabetTest(): void {
    GreekAlphabetTestFunction06();
    GreekAlphabetTestFunction07();
    GreekAlphabetTestFunction08();
    GreekAlphabetTestFunction09();
    GreekAlphabetTestFunction10();
    GreekAlphabetTestFunction11();
    GreekAlphabetTestFunction12();
    GreekAlphabetTestFunction13();
    GreekAlphabetTestFunction14();
    GreekAlphabetTestFunction15();
    GreekAlphabetTestFunction16();
    GreekAlphabetTestFunction17();
    GreekAlphabetTestFunction18();
    GreekAlphabetTestFunction19();
    GreekAlphabetTestFunction20();
    GreekAlphabetTestFunction21();
    GreekAlphabetTestFunction22();
    GreekAlphabetTestFunction23();
    GreekAlphabetTestFunction24();
    GreekAlphabetTestFunction25();
    GreekAlphabetTestFunction26();
    GreekAlphabetTestFunction27();
    GreekAlphabetTestFunction28();
    GreekAlphabetTestFunction29();
    GreekAlphabetTestFunction30();
    GreekAlphabetTestFunction31();
    GreekAlphabetTestFunction32();
    GreekAlphabetTestFunction33();
    GreekAlphabetTestFunction34();
    GreekAlphabetTestFunction35();
    GreekAlphabetTestFunction36();
    GreekAlphabetTestFunction37();
    GreekAlphabetTestFunction38();
    GreekAlphabetTestFunction39();
    GreekAlphabetTestFunction40();
    GreekAlphabetTestFunction41();
    GreekAlphabetTestFunction42();
    GreekAlphabetTestFunction43();
    GreekAlphabetTestFunction44();
    GreekAlphabetTestFunction45();
    GreekAlphabetTestFunction46();
    GreekAlphabetTestFunction47();
    GreekAlphabetTestFunction48();
}

function asNonUndefined(r: RegExpExecArray | null): Array<String> {
  return r!.map((x: String | undefined): String => x!);
}

function testRegexDotAllUnicodeFlags() {
    const rep1 = new RegExp('^.$', 'su');
    const rep2 = new RegExp('^.$', 'sum');
    const arr: Array<RegExp> = new Array<RegExp>(rep1, rep2);

    for (let re of arr) {
        arktest.assertEQ(re.test('a'), true);
        arktest.assertEQ(re.test('3'), true);
        arktest.assertEQ(re.test('π'), true);
        arktest.assertEQ(re.test('\u2027'), true);
        arktest.assertEQ(re.test('\u0085'), true);
        arktest.assertEQ(re.test('\v'), true);
        arktest.assertEQ(re.test('\f'), true);
        arktest.assertEQ(re.test('\u180E'), true);
        arktest.assertEQ(re.test('\u{10300}'), true);
        arktest.assertEQ(re.test('\n'), true);
        arktest.assertEQ(re.test('\r'), true);
        arktest.assertEQ(re.test('\u2028'), true);
        arktest.assertEQ(re.test('\u2029'), true);
        arktest.assertEQ(re.test('\uD800'), true);
        arktest.assertEQ(re.test('\uDFFF'), true);
    }
}

function testRegexUnicodeFlagOnly() {
    const rep1 = new RegExp('^.$', 'u');
    const rep2 = new RegExp('^.$', 'um');
    const arr: Array<RegExp> = new Array<RegExp>(rep1, rep2);

    for (let re of arr) {
        arktest.assertEQ(re.test('a'), true);
        arktest.assertEQ(re.test('3'), true);
        arktest.assertEQ(re.test('π'), true);
        arktest.assertEQ(re.test('\u2027'), true);
        arktest.assertEQ(re.test('\u0085'), true);
        arktest.assertEQ(re.test('\v'), true);
        arktest.assertEQ(re.test('\f'), true);
        arktest.assertEQ(re.test('\u180E'), true);
        arktest.assertEQ(re.test('\u{10300}'), true);
        arktest.assertEQ(!re.test('\n'), true);
        arktest.assertEQ(!re.test('\r'), true);
        arktest.assertEQ(!re.test('\u2028'), true);
        arktest.assertEQ(!re.test('\u2029'), true);
        arktest.assertEQ(re.test('\uD800'), true);
        arktest.assertEQ(re.test('\uDFFF'), true);
    }
}

function RegExpTest() {
    {
        let reg = new RegExp("ab|cd||")
        arktest.assertEQ(JSON.stringify(asNonUndefined(reg.exec("cd"))), '["cd"]')
    }
    {
        let reg = new RegExp("ab||")
        arktest.assertEQ(JSON.stringify(asNonUndefined(reg.exec("cd"))), '[""]')
    }
    {
        let reg = new RegExp("ab|cd|ef")
        arktest.assertEQ(JSON.stringify(asNonUndefined(reg.exec("cd"))), '["cd"]')
    }
    {
        let str = "😀";
        let regexp = new RegExp("[😀]");
        arktest.assertEQ(JSON.stringify(str.replace(regexp, "b")), JSON.stringify("b\ude00"));
    }
    {
        let str = "😀";
        let regexp = new RegExp("[😀]", "g");
        arktest.assertEQ(JSON.stringify(str.replace(regexp, "b")), JSON.stringify("bb"));
    }
    {
        let str = "😀";
        let regexp = new RegExp("[😀]", "u");
        arktest.assertEQ(JSON.stringify(str.replace(regexp, "b")), JSON.stringify("b"));
    }
    {
        let str = "😀";
        let regexp = new RegExp("[😀]", "s");
        arktest.assertEQ(JSON.stringify(str.replace(regexp, "b")), JSON.stringify("b\ude00"));
    }

    const re1 = new RegExp("[Cz]");
    const re2 = new RegExp("[Cz]", "g");
    const re3 = new RegExp("([Cz])");
    const re4 = new RegExp("([Cz])", "g");

    str1 = createHaystack();

    String1Replace(re1);
    arktest.assertEQ(resultTest, "ab.defgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgz");

    String1Replace(re2);
    arktest.assertEQ(resultTest, "ab.defg.ab.defg.ab.defg.ab.defg.ab.defg.ab.defg.ab.defg.ab.defg.");

    String2Replace(re2);
    arktest.assertEQ(resultTest, "abxyzdefgxyzabxyzdefgxyzabxyzdefgxyzabxyzdefgxyzabxyzdefgxyzabxyzdefgxyzabxyzdefgxyzabxyzdefgxyz");

    String2Replace(re1);
    arktest.assertEQ(resultTest, "abxyzdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgz");

    String3Replace(re3);
    arktest.assertEQ(resultTest, "abxCyzdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgz");

    String3Replace(re4);
    arktest.assertEQ(resultTest, "abxCyzdefgxzyzabxCyzdefgxzyzabxCyzdefgxzyzabxCyzdefgxzyzabxCyzdefgxzyzabxCyzdefgxzyzabxCyzdefgxzyzabxCyzdefgxzyz");

    Function1Replace(re3);
    arktest.assertEQ(resultTest, "abCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgz");

    Function1Replace(re4);
    arktest.assertEQ(resultTest, "abCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgzabCdefgz");

    let reg0 = new RegExp("[\\x5d-\\x7e]", "i");
    let result = reg0.test("a");
    arktest.assertEQ(result, true);

    let reg1 = new RegExp("^[-+]?([0-9]+)?(\\٫[0-9]{1,})?$");
    let result1 = reg1.test('0٫0000000000001');
    arktest.assertEQ(result1, true);

    let reg2 = new RegExp("^[A-Z0-9_\\-]*$", "i");
    arktest.assertEQ(reg2.test(''), true)

    let reg3 = new RegExp("^(?<urlStrIndex>.*?)(?<urlStr>https?[:：]//[^/]+/svn(?:/[a-z0-9.,;?'*:+&%$#=~_ \\u4E00-\\u9FA5-]*)*).*$", "i")
    arktest.assertEQ(reg3.test('a'), false);

    let reg4 = new RegExp("^(?<urlStrIndex>.*?)(?<urlStr>(?:(?:ht|f)tps?[:：]//)?(?:[a-z0-9-]+\\.)+" + "(?:com|edu|gov|mil|net|org|biz|info|name|museum|us|ca|uk|cn|cc|tw|de|au|sg|hk|ei|fr|me|im)(?![a-z])" + "(?:\\:[0-9][0-9]*)?(?:\\.?/[a-z0-9.,;?'\\|*:\\\\+&%$#=~_-]*)*).*$", "i")
    arktest.assertEQ(reg4.test('a'), false);

    let reg5 = new RegExp("^(?<urlStrIndex>.*?)(?<urlStr>(?:ht|f)tps?[:：]//(?:[a-z0-9-]+\\.)*[a-z0-9-]+(?:/[a-z0-9]+)*[/a-z0-9.,;?'\\|*:\\\\+&%$#=~_-]*).*$", "i")
    arktest.assertEQ(reg5.test('a'), false);

    let reg6 = new RegExp("^(?<urlStrIndex>.*?)(?<urlStr>(?:https?[:：]//)?(?:[a-z0-9-\\\\]+\\.)+" + "(?:com|edu|gov|mil|net|org|biz|info|name|museum|us|ca|uk|cn|cc|tw|de|au|sg|hk|ei|fr|me|im)(?![a-z])" + "(?:\\:\\d{4})?(?:/[a-z0-9.,;?'\\|*:\\\\+&%$#=~_-]*)*\\?(?:[a-z0-9]*=[a-z0-9.,;?'*:+%$#=~_\\u4E00-\\u9FA5-]*&?)*).*$", "i")
    arktest.assertEQ(reg6.test('a'), false);

    GreekAlphabetTest();

    {
        let arr1: boolean[] = [];
        let temp1 = false;
        let reg49 = new RegExp("[^A-Z0-9+/=]", "i");
        arr1.push(reg49.test("Zg=="));
        arr1.push(reg49.test("Zm8="));
        arr1.push(reg49.test("Zm9v"));
        arr1.push(reg49.test("Zm9vYg=="));
        arr1.push(reg49.test("Zm9vYmE="));
        arr1.push(reg49.test("Zm9vYmFy"));
        arr1.push(reg49.test(
            "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdC4="
        ));
        arr1.push(reg49.test("Vml2YW11cyBmZXJtZW50dW0gc2VtcGVyIHBvcnRhLg=="));
        arr1.push(reg49.test("U3VzcGVuZGlzc2UgbGVjdHVzIGxlbw=="));
        arr1.forEach((item) => {
            if (item) {
                temp1 = true;
            }
        });
        arktest.assertEQ(temp1, false);
    }

    {
        let str1 = 'SC52BAHL01031234567890123456USD';
        arktest.assertEQ(str1.replace(new RegExp("[^A-Z0-9]+", "gi"), ''), "SC52BAHL01031234567890123456USD");

        let regabc = new RegExp("abc", "g");
        let strabcd = "abcdabcdabcd";
        let exceptArr: boolean[] = [true, true, true, false, true, true, true, false, true, true];
        let resultArr: boolean[] = [];
        for (let i = 0; i < 10; i++) {
            // cache is used in this case
            resultArr.push(regabc.test(strabcd));
        }
        arktest.assertEQ(JSON.stringify(exceptArr), JSON.stringify(resultArr));

        let str2 = "aaaabbBbcccC";
        for (let i = 0; i < 2; i++) {
            arktest.assertEQ(str2, 'aaaabbBbcccC');
            let t1 = str2.replace(new RegExp("([A-Z])", "g"), (e: string) => {
                return "_" + e;
            });
            arktest.assertEQ(t1, 'aaaabb_Bbccc_C');
            let t2 = str2.replace(new RegExp("([A-Z])", "g"), "_$1");
            arktest.assertEQ(t2, 'aaaabb_Bbccc_C');
            arktest.assertEQ(t1.replace(new RegExp("([a-z]+)", "g"), "_xy"), '_xy_B_xy_C');
            arktest.assertEQ(t2.replace(new RegExp("_", "g"), ""), 'aaaabbBbcccC');
        }
    }

    {
        let regexp1 = new RegExp("a*", "gs");
        let regexp2 = new RegExp("a*", "g");
        regexp2.lastIndex = 8;
        arktest.assertEQ(regexp1.exec('aaa')?.toString(), 'aaa');
        arktest.assertEQ(regexp2.exec('aaa'), null);

        let regexp3 = new RegExp("a*", "g");
        let regexp4 = new RegExp("a*", "g");
        regexp4.lastIndex = 1;
        arktest.assertEQ(regexp3.exec('aaabab')?.toString(), 'aaa');
        arktest.assertEQ(regexp4.exec('aaabaa')?.toString(), 'aa');
    }

    // Testing regexp.prototype.replace after optimization
    {
        const re1 = new RegExp("[Cz]");
        const re2 = new RegExp("[Cz]", "g");
        const re3 = new RegExp("([Cz])");
        const re4 = new RegExp("([Cz])", "g");

        let replace_str = "abCdefgzabCdefgzabCdefgz";
        let replace_result = replace_str.replace(re1, "");
        arktest.assertEQ(re1.lastIndex, 0);
        let cached_reuslt = replace_str.replace(re1, "");
        arktest.assertEQ(replace_result === cached_reuslt, true);
        arktest.assertEQ(re1.lastIndex, 0);

        replace_result = replace_str.replace(re2, "xyz");
        arktest.assertEQ(re2.lastIndex, 0);
        cached_reuslt = replace_str.replace(re2, "xyz");
        arktest.assertEQ(replace_result === cached_reuslt, true);
        arktest.assertEQ(re2.lastIndex, 0);

        replace_result = replace_str.replace(re3, "x$1yz");
        arktest.assertEQ(re3.lastIndex, 0);
        cached_reuslt = replace_str.replace(re3, "x$1yz");
        arktest.assertEQ(replace_result === cached_reuslt, true);
        arktest.assertEQ(re3.lastIndex, 0);

        replace_result = replace_str.replace(re4, (match: string) => match);
        arktest.assertEQ(re4.lastIndex, 0);
        cached_reuslt = replace_str.replace(re4, (match: string) => match);
        arktest.assertEQ(replace_result === cached_reuslt, true);
        arktest.assertEQ(re4.lastIndex, 0);
    }

    {
        let inputString = "/vedio/av{avid}{cid}";
        let extractedContent = inputString.match(new RegExp("\\{([^{}]+)\\}", "g"));
        let replacedString = inputString.replace(new RegExp("\\{([^{}]+)\\}", "g"), '(uuu)').replace(new RegExp("\\/", "g"), "\\/");
        arktest.assertEQ(replacedString, '\\/vedio\\/av(uuu)(uuu)');

        let str = "beep boop   afff测试样本one1";
        arktest.assertEQ(str.split(new RegExp("([{}:;,]|\\s+)")).toString(), 'beep, ,boop,   ,afff测试样本one1');

        const srcTextA = '<em>a</em>bcdefghijklmnopqrstuvwxyz<em>a</em>bcdefghijklmnopqrstuvwxyz<em>a</em>bcdefghijklmnopqrstuvwxyz<em>a</em>bcdefghijklmnopqrstuvwxyz';
        const srcTextAbcd = '<em>abcd</em>efghijklmnopqrstuvwxyz<em>abcd</em>efghijklmnopqrstuvwxyz<em>abcd</em>efghijklmnopqrstuvwxyz<em>abcd</em>efghijklmnopqrstuvwxyz';

        let s0 = `verifyRegExp_1: ${verifyRegExp(srcTextA)}`;
        arktest.assertEQ(s0, 'verifyRegExp_1: ,bcdefghijklmnopqrstuvwxyz,bcdefghijklmnopqrstuvwxyz,bcdefghijklmnopqrstuvwxyz,bcdefghijklmnopqrstuvwxyz');
        let s1 = `verifyRegExp_2: ${verifyRegExp(srcTextAbcd)}`;
        arktest.assertEQ(s1, 'verifyRegExp_2: ,efghijklmnopqrstuvwxyz,efghijklmnopqrstuvwxyz,efghijklmnopqrstuvwxyz,efghijklmnopqrstuvwxyz');
        let s2 = `verifyRegExp_3: ${verifyRegExp(srcTextAbcd)}`;
        arktest.assertEQ(s2, 'verifyRegExp_3: ,efghijklmnopqrstuvwxyz,efghijklmnopqrstuvwxyz,efghijklmnopqrstuvwxyz,efghijklmnopqrstuvwxyz');
    }

    {
        try {
            const ans11 = new RegExp("/[\\p{}]/u");
            throw Error()
        } catch (error) {
            arktest.assertEQ(error instanceof SyntaxError, true);
        }
        const str3 = "a-b-c";
        const re = new RegExp("-", "y");
        arktest.assertEQ(str3.split(re).toString(), 'a,b,c');

        re.lastIndex = 1;
        arktest.assertEQ(str3.split(re).toString(), 'a,b,c');

        re.lastIndex = -1;
        arktest.assertEQ(str3.split(re).toString(), 'a,b,c');

        re.lastIndex = 3;
        arktest.assertEQ(str3.split(re).toString(), 'a,b,c');

        arktest.assertEQ(re.test(str3), true);

        arktest.assertEQ(str3.split(new RegExp("-", "g")).toString(), 'a,b,c');
    }

    {
        // search
        const str4 = "abc";
        let re1 = new RegExp("b");
        re1.lastIndex = 2;
        arktest.assertEQ(str4.search(re1), 1);
        arktest.assertEQ(str4.search(new RegExp("b", "y")), -1);
        arktest.assertEQ(str4.search(re1), 1);
        arktest.assertEQ(re1.lastIndex, 2);

        // check cache
        const str5 = "a-bc";
        let re2 = new RegExp("-");
        re2.lastIndex = 2;
        arktest.assertEQ(str5.split(re2).toString(), 'a,bc');
        arktest.assertEQ(re2.lastIndex, 2);
        arktest.assertEQ(str5.split(re2).toString(), 'a,bc');
        arktest.assertEQ(re2.lastIndex, 2);

        const str6 = "abcabc";
        let re3 = new RegExp("abc");
        re3.lastIndex = 2;
        arktest.assertEQ(str6.match(re3)!.toString(), 'abc');
        arktest.assertEQ(re3.lastIndex, 2);
        arktest.assertEQ(str6.match(re3)!.toString(), 'abc');
        arktest.assertEQ(re3.lastIndex, 2);

        let re4 = new RegExp("abc", "g");
        re4.lastIndex = 2;
        arktest.assertEQ(str6.match(re4)!.toString(), 'abc,abc');
        arktest.assertEQ(re4.lastIndex, 0);
        arktest.assertEQ(str6.match(re4)!.toString(), 'abc,abc');
        arktest.assertEQ(re4.lastIndex, 0);
    }

    {
        const regex = new RegExp("(?:)+");
        const str10 = "abcabcabc";
        const matches = regex.test(str10);
        arktest.assertEQ(matches, true);
        const matches1 = regex.exec(str10);
        arktest.assertEQ(matches1!.toString(), "");
    }

    // test getFlags
    {
        const res = new RegExp("abc", "giymsud");
        res.lastIndex = -1;
        arktest.assertEQ(res.flags, 'dgimsuy');
        const res1 = new RegExp("abc", "g");
        res1.lastIndex = -1;
        arktest.assertEQ(res1.flags, 'g');
        const res2 = new RegExp("abc", "i");;
        res2.lastIndex = -1;
        arktest.assertEQ(res2.flags, 'i');
        const res3 = new RegExp("abc", "y");;
        res3.lastIndex = -1;
        arktest.assertEQ(res3.flags, 'y');
        const res4 = new RegExp("abc", "m");;
        res4.lastIndex = -1;
        arktest.assertEQ(res4.flags, 'm');
        const res5 = new RegExp("abc", "s");;
        res5.lastIndex = -1;
        arktest.assertEQ(res5.flags, 's');
        const res6 = new RegExp("abc", "u");;
        res6.lastIndex = -1;
        arktest.assertEQ(res6.flags, 'u');
        const res7 = new RegExp("abc", "d");;
        res7.lastIndex = -1;
        arktest.assertEQ(res7.flags, 'd');
        const res8 = new RegExp("abc");;
        res8.lastIndex = -1;
        arktest.assertEQ(res8.flags, '');
    }

    {
        let str = "\\\\\\\\[aaa"
        try {
            let pattern = "[";
            let reg = new RegExp(pattern);
            throw Error()
        } catch (error) {
            arktest.assertEQ(error instanceof SyntaxError, true);
        }
        try {
            let pattern = "\[";
            let reg = new RegExp(pattern);
            throw Error()
        } catch (error) {
            arktest.assertEQ(error instanceof SyntaxError, true);
        }
        try {
            let pattern = "\\[";
            let reg = new RegExp(pattern);
            arktest.assertEQ(pattern + " " + JSON.stringify(asNonUndefined(reg.exec(str))), '\\[ ["["]')
        } catch (error) {
            arktest.assertEQ(true, false, "Expected SyntaxError but got none");
        }
        try {
            let pattern = "\\\[";
            let reg = new RegExp(pattern);
            arktest.assertEQ(pattern + " " + JSON.stringify(asNonUndefined(reg.exec(str))), '\\[ ["["]')
        } catch (error) {
            arktest.assertEQ(true, false, "Expected SyntaxError but got none");
        }
        try {
            let pattern = "\\\\[";
            let reg = new RegExp(pattern);
            throw Error()
        } catch (error) {
            arktest.assertEQ(error instanceof SyntaxError, true);
        }
        try {
            let pattern = "\\\\\[";
            let reg = new RegExp(pattern);
            throw Error()
        } catch (error) {
            arktest.assertEQ(error instanceof SyntaxError, true);
        }
        try {
            let pattern = "\\\\\\[";
            let reg = new RegExp(pattern);
            arktest.assertEQ(pattern + " " + JSON.stringify(asNonUndefined(reg.exec(str))), '\\\\\\[ ["\\\\["]')
        } catch (error) {
            arktest.assertEQ(true, false, "Expected SyntaxError but got none");
        }
        try {
            let pattern = "\\\\\\\[";
            let reg = new RegExp(pattern);
            arktest.assertEQ(pattern + " " + JSON.stringify(asNonUndefined(reg.exec(str))), '\\\\\\[ ["\\\\["]')
        } catch (error) {
            arktest.assertEQ(true, false, "Expected SyntaxError but got none");
        }
    }

    {
        // subStringIsUtf8 branch canbecompressed
        str1 = 'utf83c这要替换"!';
        let regexp = new RegExp("([0-9])([a-z])", "g");
        let newStr1 = str1.replace(regexp, "$1");
        arktest.assertEQ(newStr1, 'utf83这要替换"!');

        // subStringIsUtf8 branch length=0;
        str1 = '3c这要替换"!';
        regexp = new RegExp("([0-9])([a-z])", "g");
        newStr1 = str1.replace(regexp, "$1");
        arktest.assertEQ(newStr1, '3这要替换"!');

        // subStringIsUtf8 branch src isUtf8;
        str1 = 'abcdefg3chigk"!';
        regexp = new RegExp("([0-9])([a-z])", "g");
        newStr1 = str1.replace(regexp, "$1");
        arktest.assertEQ(newStr1, 'abcdefg3higk"!');
    }

    {
        let s1 = 'abc';
        let s2 = '\ud834\udf06';
        let reg = new RegExp(s1.repeat(10000));
        let str = s1.repeat(10000);
        let res = str.replace(reg, s2);
        arktest.assertEQ(res == s2, true);

        reg = new RegExp(s2.repeat(10000));
        str = s2.repeat(10000);
        res = str.replace(reg, s1);
        arktest.assertEQ(res == s1, true);
    }

    {
        let reg = new RegExp("a(?:|x)$");
        arktest.assertEQ(JSON.stringify(asNonUndefined(reg.exec("ax"))), '["ax"]')
    }

}

function main(): int {
    const suite = new arktest.ArkTestsuite("RegExp Test")

    suite.addTest('RegExp Test', RegExpTest)
    suite.addTest('RegExp Test dotall', testRegexDotAllUnicodeFlags)
    suite.addTest('RegExp Test Unicode flag only', testRegexUnicodeFlagOnly)
    suite.addTest('RegExp replaceTest', replaceTest)

    return suite.run()
}
