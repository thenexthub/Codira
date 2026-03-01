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

function main(): int {
    const suite = new arktest.ArkTestsuite("XmlPullParser API tests, part5");
    suite.addTest("Testing XML ParseInfo getName with ENTITY_REFERENCE event.", testXmlParseInfoGetName0010)
    suite.addTest("Testing XML ParseInfo getName with single testXml.", estXmlParseInfoGetName0013)
    suite.addTest("Testing XML ParseInfo getName with invalid testXml.", testXmlParseInfoGetName0012)
    suite.addTest("Testing XML ParseInfo getName with WHITESPACE event.", testXmlParseInfoGetName0011)
    suite.addTest("Testing XML ParseInfo getName with INSTRUCTION event.", testXmlParseInfoGetName0009)
    suite.addTest("Testing XML ParseInfo getName with DOCDECL event.", testXmlParseInfoGetName0008)
    suite.addTest("Testing XML ParseInfo getName with COMMENT event.", testXmlParseInfoGetName0007)
    suite.addTest("Testing XML ParseInfo getName with CDSECT event.", testXmlParseInfoGetName0006)
    suite.addTest("Testing XML ParseInfo getName with TEXT event.", testXmlParseInfoGetName0005)
    suite.addTest("Testing XML ParseInfo getName with end tag event.", testXmlParseInfoGetName0004)
    suite.addTest("Testing XML ParseInfo getName with start tag event.", testXmlParseInfoGetName0003)
    suite.addTest("Testing XML ParseInfo getName with END_DOCUMENT event.", testXmlParseInfoGetName0002)
    suite.addTest("Testing XML ParseInfo getName with START_DOCUMENT event.", testXmlParseInfoGetName0001)
    suite.addTest("Test XmlPullParser getName", testGetName)
    
    return suite.run()
}

function eventName(type: xml.EventType): string {
    switch (type) {
        case xml.EventType.START_DOCUMENT : return "START_DOCUMENT"
        case xml.EventType.END_DOCUMENT: return "END_DOCUMENT"
        case xml.EventType.START_TAG : return "START_TAG"
        case xml.EventType.END_TAG: return "END_TAG"
        case xml.EventType.TEXT: return "TEXT"
        case xml.EventType.CDSECT: return "CDSECT"
        case xml.EventType.COMMENT: return "COMMENT"
        case xml.EventType.DOCDECL: return "DOCDECL"
        case xml.EventType.INSTRUCTION: return "INSTRUCTION"
        case xml.EventType.ENTITY_REFERENCE: return "ENTITY_REFERENCE"
        case xml.EventType.WHITESPACE: return "WHITESPACE"
        default: return "default"
    }
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

function testXmlParseInfoGetName0001() {
    const TAG = 'testXmlParseInfoGetName0001';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:0 value:"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetName0002() {
    const TAG = 'testXmlParseInfoGetName0002';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:1 value:"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetName0003() {
    const TAG = 'testXmlParseInfoGetName0003';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:2 value:example"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetName0004() {
    const TAG = 'testXmlParseInfoGetName0004';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:3 value:endDocument"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetName0005() {
    const TAG = 'testXmlParseInfoGetName0005';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:4 value:"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetName0006() {
    const TAG = 'testXmlParseInfoGetName0006';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:5 value:"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetName0007() {
    const TAG = 'testXmlParseInfoGetName0007';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:6 value:"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetName0008() {
    const TAG = 'testXmlParseInfoGetName0008';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:7 value:"))
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetName0009() {
    const TAG = 'testXmlParseInfoGetName0009';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:8 value:"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetName0010() {
    const TAG = 'testXmlParseInfoGetName0010';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:9 value:exampleEntity"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetName0011() {
    const TAG = 'testXmlParseInfoGetName0011';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:10 value:"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetName0012() {
    const TAG = 'testXmlParseInfoGetName0012';
    try {
        let testXml = "<1example>" +
            "<example>" +
            "<invalid-name!></invalid-name!>" +
            "</example>" +
            "</1example>";

        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(false)
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertEQ((err as BusinessError).code, 401);
    }
}

function estXmlParseInfoGetName0013() {
    try {
        let testXml = "<item id=\"1\" />\n";
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getName() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:2 value:item"));
        arktest.assertTrue(str.includes("key:3 value:item"));
    } catch (err) {
        const TAG = 'testXmlParseInfoGetName0013';
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

// testParseXml006
function testGetName() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance='heigh' logged='true'>",
    "    <title>Happy</title>",
    "    <todo>Work</todo>",
    "</note>")

    let textStr = ''
    let tokenCb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        if ((eventType === xml.EventType.START_TAG) || (eventType === xml.EventType.END_TAG)) {
            textStr += "{ " + eventName(eventType) + " : " + value.getName() + " }; "
        }
        return true
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: tokenCb}
    parser.parseXml(options)

    let expectedTextStr = '{ START_TAG : note }; { START_TAG : title }; { END_TAG : title }; { START_TAG : todo }; { END_TAG : todo }; { END_TAG : note }; '

    arktest.assertEQ(textStr, expectedTextStr)
}

