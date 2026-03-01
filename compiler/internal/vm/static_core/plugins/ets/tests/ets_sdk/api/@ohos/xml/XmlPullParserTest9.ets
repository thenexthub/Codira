/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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
import {xml} from "@ohos.xml"
import { BusinessError } from "@ohos.base";
import { util } from "@ohos.util";

function main(): int {
    const suite = new arktest.ArkTestsuite("XmlPullParser API tests, part9")

    suite.addTest("Testing the XML PullParser's constructor with element names starting with a number.", testXmlPullParserConstructor0006)
    suite.addTest("Testing the XML PullParser's constructor with comments.", testXmlPullParserConstructor0005)
    suite.addTest("Testing the XML PullParser's constructor with incorrect nested tags.", testXmlPullParserConstructor0004)
    suite.addTest("Testing the XML PullParser's constructor with a double tag.", testXmlPullParserConstructor0003)
    suite.addTest("Testing the XML PullParser's constructor with a single tag.", testXmlPullParserConstructor0002)
    suite.addTest("Testing the XML PullParser's constructor with the attribute value is not within ''.", testXmlPullParserConstructor0001)
    suite.addTest("Testing the XML PullParser constructor with an empty string.", testXmlPullParserConstructor0010)
    suite.addTest("Testing the XML PullParser's constructor with namespaces.Verifies that it handles XML namespaces correctly.", testXmlPullParserConstructor0012)
    suite.addTest("Testing the XML PullParser's constructor with long character input.Verifies that it handles large inputs properly.", testXmlPullParserConstructor0011)
    suite.addTest("Testing the XML PullParser's CDATA parsing. testXml017", testXml017)
    return suite.run()
}

function rawStringToBuffer( str: string ) : ArrayBuffer {
    let idx = 0
    let len = str.length
    let arr = new Uint8Array( len )
    for ( idx = 0 ; idx < len ; ++idx ) {
        arr[ idx ] = str.charCodeAt(idx) & 0xFF;
    }

    return arr.buffer as ArrayBuffer
}

function testXmlPullParserConstructor0001() {
    const TAG = 'testXmlPullParserConstructor0001';
    try {
    let testXml =
        '<?xml version="1.0" encoding="utf-8"?>' +
        '<book price=245>' +
        '  <ti-tle>XML for Dummies</ti-tle>' +
        '  <author>John Doe</author>' +
        '  <price>19.99</price>' +
        '  <description>This book is about XML.</description>' +
        '</book>'

    let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
    let options: xml.ParseOptions = {
        supportDoctype: true, ignoreNameSpace: true,
        tagValueCallbackFunction: (name: string, value: string) => {
        return true;
        }
    }
    parser.parseXml(options);
    arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        try {
            arktest.assertEQ((err as BusinessError).code, 401);
        } catch (e) {}
    }
}

function testXmlPullParserConstructor0002() {
    const TAG = 'testXmlPullParserConstructor0002';
    try {
        let testXml =
            '<?xml version="1.0" encoding="utf-8" ?>' +
            '<book/>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        try {
            arktest.assertEQ((err as BusinessError).code, 401);
        } catch (e) {}
    }
}

function testXmlPullParserConstructor0003() {
    const TAG = 'testXmlPullParserConstructor0003';
    try {
        let testXml =
            '<?xml version="1.0" encoding="utf-8" ?>' +
            '<book>' +
            '  <title>XML for Dummies</title>' +
            '  <author>John Doe</author>' +
            '  <price>19.99</price>' +
            '</book>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserConstructor0004() {
    const TAG = 'testXmlPullParserConstructor0004';
    try {
        let testXml =
            '<?xml version="1.0" encoding="utf-8" ?>' +
            '<root>' +
            '    <b><i>This text is bold and italic</b></i>' +
            '</root>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        try {
            arktest.assertEQ((err as BusinessError).code, 401);
        } catch (e) {}
    }
}

function testXmlPullParserConstructor0005() {
    const TAG = 'testXmlPullParserConstructor0005';
    try {
        let testXml =
            '<?xml version="1.0" encoding="utf-8" ?>' +
            '<!-- This is a comment -->' +
            '<root>' +
            '    <child>Content here</child>' +
            '</root>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserConstructor0006() {
    const TAG = 'testXmlPullParserConstructor0006';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<123book>' +
            '  <title>XML for Dummies</title>' +
            '  <author>John Doe</author>' +
            '  <price>19.99</price>' +
            '</123book>'

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        try {
            arktest.assertEQ((err as BusinessError).code, 401);
        } catch (e) {}
    }
}

function testXmlPullParserConstructor0010() {
    const TAG = 'testXmlPullParserConstructor0010';
    try {
        let testXml = '';
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
        arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
    }
}

function testXmlPullParserConstructor0011() {
    const TAG = 'testXmlPullParserConstructor0011';
    try {
        const MY_MAX = 65536;
        let longText = new Array<char>(MY_MAX).join('a');
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<longText>' +
            longText +
            '</longText>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: true,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlPullParserConstructor0012() {
    const TAG = 'testXmlPullParserConstructor0012';
    try {
        let testXml =
            '<?xml version="1.0" encoding="UTF-8" ?>' +
            '<catalog xmlns:h="http://www.w3.org/TR/html4/" xmlns:f="http://www.example.com/furniture">' +
            '    <h:page>' +
            '        <h:title>Product Catalog</h:title>' +
            '        <h:body>' +
            '            <f:chair>' +
            '                <f:name>Office Chair</f:name>' +
            '                <f:price>59.99</f:price>' +
            '            </f:chair>' +
            '        </h:body>' +
            '    </h:page>' +
            '</catalog>';

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let options: xml.ParseOptions = {
            supportDoctype: true, ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true
        };
        parser.parseXml(options);
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}


let str = ""
function testXml017() {
    let strXml =
    '<?xml version="1.0" encoding="UTF-8"?>' +
        '<note importance="high" logged="true">' +
        '<company><![CDATA[SYSTEM</company>' +
        '<todo>work</todo>' +
        '</note>';
    let textEncoder = new util.TextEncoder();
    let uint8 = textEncoder.encodeInto(strXml);
    let pullParse = new xml.XmlPullParser(uint8.buffer as object as ArrayBuffer);
    str = '';
    let options: xml.ParseOptions = { supportDoctype: true, ignoreNameSpace: true, tagValueCallbackFunction: func17 }
    try {
    pullParse.parseXml(options);
    arktest.assertTrue(false)
    } catch (e) {
        e = e as BusinessError;
        arktest.assertEQ(e.toString(), "BusinessError: Cannot find the ']]>' in xml string.")
        arktest.assertEQ(e.code ,401)
    }

}

function func17(name: string, value: string): boolean {
  str += name + value;
  return true;
}
