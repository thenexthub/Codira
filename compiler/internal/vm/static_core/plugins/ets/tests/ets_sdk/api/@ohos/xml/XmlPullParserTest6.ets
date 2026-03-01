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
import { util } from "@ohos.util";

function main(): int {
    const suite = new arktest.ArkTestsuite("XmlPullParser API tests, part6");

    suite.addTest("Testing XML ParseInfo getColumnNumber with WHITESPACE event.", testXmlParseInfoGetColumnNumber0011)
    suite.addTest("Testing XML ParseInfo getColumnNumber with ENTITY_REFERENCE event.", testXmlParseInfoGetColumnNumber0010)
    suite.addTest("Testing XML ParseInfo getColumnNumber with INSTRUCTION event.", testXmlParseInfoGetColumnNumber0009)
    suite.addTest("Testing XML ParseInfo getColumnNumber with DOCDECL event.", testXmlParseInfoGetColumnNumber0008)
    suite.addTest("Testing XML ParseInfo getColumnNumber with COMMENT event.", testXmlParseInfoGetColumnNumber0007)
    suite.addTest("Testing XML ParseInfo getColumnNumber with CDSECT event.", testXmlParseInfoGetColumnNumber0006)
    suite.addTest("Testing XML ParseInfo getColumnNumber with TEXT event.", testXmlParseInfoGetColumnNumber0005)
    suite.addTest("Testing XML ParseInfo getColumnNumber with end tag event.", testXmlParseInfoGetColumnNumber0004)
    suite.addTest("Testing XML ParseInfo getColumnNumber with start tag event.", testXmlParseInfoGetColumnNumber0003)
    suite.addTest("Testing XML ParseInfo getColumnNumber with END_DOCUMENT event.", testXmlParseInfoGetColumnNumber0002)
    suite.addTest("Testing XML ParseInfo getColumnNumber with START_DOCUMENT event.", testXmlParseInfoGetColumnNumber0001)
    suite.addTest("Test XmlPullParser getColumnNumber", testGetColumnNuber)

    return suite.run()
}

const testXml =
  "<?xml version='1.0' encoding=\"utf-8\"?><!DOCTYPE example [<!ENTITY exampleEntity 'some entity'>]>\
  <!-- XMLjghk --><example><!-- fghj -->\
  <startDocument><startTag><element><!-- yesdryug -->\
  <tagName>tfyugyi</tagName><!-- xtgh --><content>ydtyfugihj</content>" +
    "<!-- CDATAguhi -->" +
    "<cdata><![CDATA[fyghjk]]></cdata>" +
    "<!-- ugh -->" +
    "<?processingInstruction example='cfgvhj'?>" +
    "<!-- gvhblkm -->" +
    "<entityReference>&exampleEntity;</entityReference>" +
    "<!-- fcguhjk -->" +
    "<whitespace>     </whitespace>" +
    "</element>" +
    "<!-- ufyui -->" +
    "</startTag>" +
    "</startDocument>" +
    "<!-- ufygo -->" +
    "<endDocument/>" +
    "</example>";

function rawStringToBuffer( str: string ) : ArrayBuffer {
    return new util.TextEncoder().encodeInto(str).buffer;
}

function testXmlParseInfoGetColumnNumber0001() {
    const TAG = 'testXmlParseInfoGetColumnNumber0001';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getColumnNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:0 value:1"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetColumnNumber0002() {
    const TAG = 'testXmlParseInfoGetColumnNumber0002';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getColumnNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:1 value:566"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetColumnNumber0003() {
    const TAG = 'testXmlParseInfoGetColumnNumber0003';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getColumnNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:2 value:154"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetColumnNumber0004() {
    const TAG = 'testXmlParseInfoGetColumnNumber0004';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getColumnNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options)
        arktest.assertTrue(str.includes("key:3 value:218"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetColumnNumber0005() {
    const TAG = 'testXmlParseInfoGetColumnNumber0005';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getColumnNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:4 value:208"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetColumnNumber0006() {
    const TAG = 'testXmlParseInfoGetColumnNumber0006';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getColumnNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:5 value:303"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetColumnNumber0007() {
    const TAG = 'testXmlParseInfoGetColumnNumber0007';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getColumnNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:6 value:190"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetColumnNumber0008() {
    const TAG = 'testXmlParseInfoGetColumnNumber0008';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getColumnNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:7 value:97"))
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetColumnNumber0009() {
    const TAG = 'testXmlParseInfoGetColumnNumber0009';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getColumnNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:8 value:365"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetColumnNumber0010() {
    const TAG = 'testXmlParseInfoGetColumnNumber0010';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getColumnNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:9 value:413"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetColumnNumber0011() {
    const TAG = 'testXmlParseInfoGetColumnNumber0011';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getColumnNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:10 value:464"))
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

// testParseXml003
function testGetColumnNuber() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance='high'>",
    "    <title>Happy</title>",
    "    <todo>Work</todo>",
    "</note>")

    let textStr = ''
    let result = ''
    let tokenCb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        textStr += "{ " + value.getName() + " : " + value.getColumnNumber() + " }; "
        result += "key:" + eventType + " value:" + value.getColumnNumber() + " "
        return true
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: tokenCb}
    parser.parseXml(options)

    let expectedTextStr = "{  : 1 }; { note : 63 }; {  : 67 }; { title : 74 }; {  : 79 }; { title : 87 }; \
{  : 91 }; { todo : 97 }; {  : 101 }; { todo : 108 }; { note : 115 }; {  : 115 }; "

    arktest.assertEQ(textStr, expectedTextStr)

    let expectedRes = "key:0 value:1 key:2 value:63 key:10 value:67 key:2 value:74 key:4 value:79 key:3 value:87 \
key:10 value:91 key:2 value:97 key:4 value:101 key:3 value:108 key:3 value:115 key:1 value:115 "
    arktest.assertEQ(result, expectedRes)
}

