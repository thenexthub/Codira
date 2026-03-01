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
    const suite = new arktest.ArkTestsuite("XmlPullParser API tests, part8");
    
    suite.addTest("Testing XML ParseInfo getLineNumber with WHITESPACE event.", testXmlParseInfoGetLineNumber0011)
    suite.addTest("Testing XML ParseInfo getLineNumber with ENTITY_REFERENCE event.", testXmlParseInfoGetLineNumber0010)
    suite.addTest("Testing XML ParseInfo getLineNumber with INSTRUCTION event.", testXmlParseInfoGetLineNumber0009)
    suite.addTest("Testing XML ParseInfo getLineNumber with DOCDECL event.", testXmlParseInfoGetLineNumber0008)
    suite.addTest("Testing XML ParseInfo getLineNumber with COMMENT event.", testXmlParseInfoGetLineNumber0007)
    suite.addTest("Testing XML ParseInfo getLineNumber with CDSECT event.", testXmlParseInfoGetLineNumber0006)
    suite.addTest("Testing XML ParseInfo getLineNumber with end tag event.", testXmlParseInfoGetLineNumber0004)
    suite.addTest("Testing XML ParseInfo getLineNumber with start tag event.", testXmlParseInfoGetLineNumber0003)
    suite.addTest("Testing XML ParseInfo getLineNumber with TEXT event.", testXmlParseInfoGetLineNumber0005)
    suite.addTest("Testing XML ParseInfo getLineNumber with END_DOCUMENT event.", testXmlParseInfoGetLineNumber0002)
    suite.addTest("Testing XML ParseInfo getLineNumber with START_DOCUMENT event.", testXmlParseInfoGetLineNumber0001)
    suite.addTest("Test XmlPullParser getLineNumber", testGetLineNumber)

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
    let idx = 0
    let len = str.length
    let arr = new Uint8Array( len )
    for ( idx = 0 ; idx < len ; ++idx ) {
        arr[ idx ] = str.charCodeAt(idx) & 0xFF;
    }

    return arr.buffer as ArrayBuffer
}

function testXmlParseInfoGetLineNumber0003() {
    const TAG = 'testXmlParseInfoGetLineNumber0003';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getLineNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:2 value:1"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetLineNumber0001() {
    const TAG = 'testXmlParseInfoGetLineNumber0001';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getLineNumber() + ' ';
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

function testXmlParseInfoGetLineNumber0002() {
    const TAG = 'testXmlParseInfoGetLineNumber0002';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getLineNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:1 value:1"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetLineNumber0004() {
    const TAG = 'testXmlParseInfoGetLineNumber0004';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getLineNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:3 value:1"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetLineNumber0005() {
    const TAG = 'testXmlParseInfoGetLineNumber0005';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getLineNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:4 value:1"))
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetLineNumber0006() {
    const TAG = 'testXmlParseInfoGetLineNumber0006';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getLineNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:5 value:1"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetLineNumber0007() {
    const TAG = 'testXmlParseInfoGetLineNumber0007';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getLineNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:6 value:1"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetLineNumber0008() {
    const TAG = 'testXmlParseInfoGetLineNumber0008';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getLineNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:7 value:1"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetLineNumber0009() {
    const TAG = 'testXmlParseInfoGetLineNumber0009';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getLineNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:8 value:1"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetLineNumber0010() {
    const TAG = 'testXmlParseInfoGetLineNumber0010';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getLineNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:9 value:1"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

function testXmlParseInfoGetLineNumber0011() {
    const TAG = 'testXmlParseInfoGetLineNumber0011';
    try {
        let parser = new xml.XmlPullParser(rawStringToBuffer(testXml), "utf-8");
        let str = "";
        const func = (key: xml.EventType, value: xml.ParseInfo) => {
            str += 'key:' + key + ' value:' + value.getLineNumber() + ' ';
            return true;
        };
        let options: xml.ParseOptions = {
            supportDoctype: true,
            ignoreNameSpace: false,
            tagValueCallbackFunction: (name: string, value: string) => true,
            tokenValueCallbackFunction: func,
        };
        parser.parseXml(options);
        arktest.assertTrue(str.includes("key:10 value:1"));
    } catch (err) {
        console.error(`${TAG} failed, error: ${err}`);
        arktest.assertTrue(false)
    }
}

// testParseXml005
function testGetLineNumber() {
    const str : ReadonlyArray<string> = Array.of<string>(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
    "<note importance='high'>",
    "    <title>Happy</title>",
    "    <todo>Work</todo>",
    "    <todo>Play</todo>",
    "</note>")

    let textStr = ''
    let oldStr = ''
    let tokenCb = (eventType: xml.EventType, value: xml.ParseInfo) => { 
        textStr += "{ " + value.getName() + " : " + value.getLineNumber() + " }; "
        oldStr += "key:" + eventType + " value:" + value.getLineNumber() + " "
        return true
    }

    let strXml = str.join("")
    let data = rawStringToBuffer(strXml)
    let parser = new xml.XmlPullParser(data, "utf-8")
    let options: xml.ParseOptions = {supportDoctype: true, ignoreNameSpace: true, tokenValueCallbackFunction: tokenCb}
    parser.parseXml(options)

    let expectedTextStr = '{  : 1 }; { note : 1 }; {  : 1 }; { title : 1 }; {  : 1 }; { title : 1 }; \
{  : 1 }; { todo : 1 }; {  : 1 }; { todo : 1 }; {  : 1 }; { todo : 1 }; {  : 1 }; { todo : 1 }; { note : 1 }; {  : 1 }; '

    arktest.assertEQ(textStr, expectedTextStr)

    let expectedOld = 'key:0 value:1 key:2 value:1 key:10 value:1 key:2 value:1 key:4 value:1 ' +
    'key:3 value:1 key:10 value:1 key:2 value:1 key:4 value:1 key:3 value:1 ' +
    'key:10 value:1 key:2 value:1 key:4 value:1 key:3 value:1 key:3 value:1 key:1 value:1 '
    arktest.assertEQ(oldStr, expectedOld)
}
